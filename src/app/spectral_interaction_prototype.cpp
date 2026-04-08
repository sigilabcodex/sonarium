#include "core/params/parameter_model.h"
#include "engine/engine_facade.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;
};

std::vector<float> make_demo_signal(std::size_t sample_rate, std::size_t frames) {
    std::vector<float> input(frames, 0.0f);
    for (std::size_t i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        const float low = 0.55f * std::sin(2.0f * static_cast<float>(std::numbers::pi) * 220.0f * t);
        const float mid = 0.3f * std::sin(2.0f * static_cast<float>(std::numbers::pi) * 1200.0f * t);
        const float high = 0.2f * std::sin(2.0f * static_cast<float>(std::numbers::pi) * 5100.0f * t);
        input[i] = low + mid + high;
    }
    return input;
}

std::vector<float> magnitude_spectrum(const std::vector<float>& signal, std::size_t fft_size) {
    const std::size_t n = std::min(signal.size(), fft_size);
    std::vector<float> spectrum(fft_size / 2, 0.0f);

    for (std::size_t k = 0; k < spectrum.size(); ++k) {
        double real = 0.0;
        double imag = 0.0;
        for (std::size_t t = 0; t < n; ++t) {
            const double window = 0.5 - 0.5 * std::cos((2.0 * std::numbers::pi * static_cast<double>(t)) /
                                                       static_cast<double>(n));
            const double sample = static_cast<double>(signal[t]) * window;
            const double phase = -2.0 * std::numbers::pi * static_cast<double>(k * t) / static_cast<double>(fft_size);
            real += sample * std::cos(phase);
            imag += sample * std::sin(phase);
        }
        spectrum[k] = static_cast<float>(std::sqrt((real * real) + (imag * imag)));
    }

    return spectrum;
}

float to_db(float magnitude) {
    constexpr float min_mag = 1e-6f;
    return 20.0f * std::log10(std::max(magnitude, min_mag));
}

std::vector<float> to_db_spectrum(const std::vector<float>& magnitude) {
    std::vector<float> out(magnitude.size(), 0.0f);
    for (std::size_t i = 0; i < magnitude.size(); ++i) {
        out[i] = to_db(magnitude[i]);
    }
    return out;
}

std::vector<float> mask_to_db(const std::vector<float>& mask_nodes) {
    std::vector<float> out(mask_nodes.size(), 0.0f);
    for (std::size_t i = 0; i < mask_nodes.size(); ++i) {
        out[i] = to_db(sonarium::core::params::normalized_to_linear_gain(mask_nodes[i]));
    }
    return out;
}

bool contains(const Rect& rect, float px, float py) {
    return px >= rect.x && py >= rect.y && px < (rect.x + rect.w) && py < (rect.y + rect.h);
}

int map_x_to_index(const Rect& rect, float px, std::size_t size) {
    if (size < 2) {
        return 0;
    }

    const float u = std::clamp((px - rect.x) / std::max(1.0f, rect.w - 1.0f), 0.0f, 1.0f);
    return static_cast<int>(u * static_cast<float>(size - 1));
}

float map_y_to_normalized(const Rect& rect, float py) {
    const float v = std::clamp((py - rect.y) / std::max(1.0f, rect.h - 1.0f), 0.0f, 1.0f);
    return sonarium::core::params::clamp_normalized(1.0f - v);
}

float map_index_to_x(const Rect& rect, std::size_t index, std::size_t size) {
    if (size < 2) {
        return rect.x;
    }

    const float u = static_cast<float>(index) / static_cast<float>(size - 1);
    return rect.x + (u * (rect.w - 1.0f));
}

float map_normalized_to_y(const Rect& rect, float normalized) {
    const float clamped = sonarium::core::params::clamp_normalized(normalized);
    return rect.y + rect.h - 1.0f - (clamped * std::max(1.0f, rect.h - 1.0f));
}

float map_db_to_y(const Rect& rect, float db, float min_db, float max_db) {
    const float span = std::max(1e-6f, max_db - min_db);
    const float y_n = std::clamp((db - min_db) / span, 0.0f, 1.0f);
    return rect.y + rect.h - 1.0f - (y_n * std::max(1.0f, rect.h - 1.0f));
}

void draw_filled_rect(const Rect& rect) {
    glBegin(GL_QUADS);
    glVertex2f(rect.x, rect.y);
    glVertex2f(rect.x + rect.w, rect.y);
    glVertex2f(rect.x + rect.w, rect.y + rect.h);
    glVertex2f(rect.x, rect.y + rect.h);
    glEnd();
}

void draw_rect_outline(const Rect& rect) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(rect.x, rect.y);
    glVertex2f(rect.x + rect.w, rect.y);
    glVertex2f(rect.x + rect.w, rect.y + rect.h);
    glVertex2f(rect.x, rect.y + rect.h);
    glEnd();
}

void draw_curve(const Rect& rect, const std::vector<float>& values, float min_value, float max_value) {
    if (values.size() < 2) {
        return;
    }

    const float span = std::max(1e-6f, max_value - min_value);
    glBegin(GL_LINE_STRIP);
    for (std::size_t i = 0; i < values.size(); ++i) {
        const float x = map_index_to_x(rect, i, values.size());
        const float y_n = std::clamp((values[i] - min_value) / span, 0.0f, 1.0f);
        const float y = rect.y + rect.h - 1.0f - (y_n * std::max(1.0f, rect.h - 1.0f));
        glVertex2f(x, y);
    }
    glEnd();
}

// Minimal 3x5 bitmap glyphs for compact in-window diagnostics/help text.
int glyph_bits(char c) {
    switch (c) {
        case 'A': return 0b010101111101101;
        case 'B': return 0b110101110101110;
        case 'C': return 0b011100100100011;
        case 'D': return 0b110101101101110;
        case 'E': return 0b111100110100111;
        case 'F': return 0b111100110100100;
        case 'G': return 0b011100101101011;
        case 'H': return 0b101101111101101;
        case 'I': return 0b111010010010111;
        case 'J': return 0b111001001101010;
        case 'K': return 0b101101110101101;
        case 'L': return 0b100100100100111;
        case 'M': return 0b101111111101101;
        case 'N': return 0b101111111111101;
        case 'O': return 0b010101101101010;
        case 'P': return 0b110101110100100;
        case 'Q': return 0b010101101111011;
        case 'R': return 0b110101110101101;
        case 'S': return 0b011100010001110;
        case 'T': return 0b111010010010010;
        case 'U': return 0b101101101101111;
        case 'V': return 0b101101101101010;
        case 'W': return 0b101101111111101;
        case 'X': return 0b101101010101101;
        case 'Y': return 0b101101010010010;
        case 'Z': return 0b111001010100111;
        case '0': return 0b111101101101111;
        case '1': return 0b010110010010111;
        case '2': return 0b110001111100111;
        case '3': return 0b111001011001111;
        case '4': return 0b101101111001001;
        case '5': return 0b111100111001111;
        case '6': return 0b111100111101111;
        case '7': return 0b111001001001001;
        case '8': return 0b111101111101111;
        case '9': return 0b111101111001111;
        case '-': return 0b000000111000000;
        case '.': return 0b000000000000010;
        case ':': return 0b000010000010000;
        case '>': return 0b100010001010100;
        case '+': return 0b000010111010000;
        case '/': return 0b001001010100100;
        case '=': return 0b000111000111000;
        case ' ': return 0;
        default: return 0b111101101101111;
    }
}

void draw_text(float x, float y, const std::string& text, float scale) {
    float cursor_x = x;
    for (char c : text) {
        const char uc = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        const int bits = glyph_bits(uc);

        for (int row = 0; row < 5; ++row) {
            for (int col = 0; col < 3; ++col) {
                const int bit_index = 14 - (row * 3 + col);
                if ((bits & (1 << bit_index)) == 0) {
                    continue;
                }

                const Rect pixel{cursor_x + (col * scale), y + (row * scale), scale, scale};
                draw_filled_rect(pixel);
            }
        }
        cursor_x += (4.0f * scale);
    }
}

class PrototypeWindow {
  public:
    PrototypeWindow()
        : input_(make_demo_signal(kSampleRate, kSampleRate / 2)),
          engine_({1024, 256}),
          mask_nodes_(engine_.state().gain_mask.mask_nodes_normalized),
          pre_db_(to_db_spectrum(magnitude_spectrum(input_, engine_.config().fft_size))),
          debug_events_(std::getenv("SONARIUM_PROTOTYPE_DEBUG") != nullptr) {
        refresh_analysis();
    }

    bool run() {
        std::cout << "[prototype] Initializing GLFW..." << std::endl;
        if (glfwInit() == GLFW_FALSE) {
            std::cerr << "[prototype] Failed to initialize GLFW." << std::endl;
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        GLFWwindow* window =
            glfwCreateWindow(kWindowWidth, kWindowHeight, "Sonarium Spectral Interaction Prototype", nullptr, nullptr);
        if (window == nullptr) {
            std::cerr << "[prototype] Failed to create window." << std::endl;
            glfwTerminate();
            return false;
        }

        std::cout << "[prototype] Window created: " << kWindowWidth << "x" << kWindowHeight << std::endl;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
            auto* self = static_cast<PrototypeWindow*>(glfwGetWindowUserPointer(w));
            if (self != nullptr) {
                self->on_key(w, key, action);
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            auto* self = static_cast<PrototypeWindow*>(glfwGetWindowUserPointer(w));
            if (self != nullptr) {
                self->on_mouse_button(w, button, action, mods);
            }
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double x, double y) {
            auto* self = static_cast<PrototypeWindow*>(glfwGetWindowUserPointer(w));
            if (self != nullptr) {
                self->on_cursor_move(static_cast<float>(x), static_cast<float>(y));
            }
        });

        while (!glfwWindowShouldClose(window)) {
            int framebuffer_w = 0;
            int framebuffer_h = 0;
            glfwGetFramebufferSize(window, &framebuffer_w, &framebuffer_h);

            int window_w = 0;
            int window_h = 0;
            glfwGetWindowSize(window, &window_w, &window_h);

            draw(window_w, window_h, framebuffer_w, framebuffer_h);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        std::cout << "[prototype] Shutting down." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return true;
    }

  private:
    static constexpr std::size_t kSampleRate = 48000;
    static constexpr int kWindowWidth = 1280;
    static constexpr int kWindowHeight = 840;

    void refresh_analysis() {
        const auto output = engine_.process_mono(input_);
        post_db_ = to_db_spectrum(magnitude_spectrum(output, engine_.config().fft_size));
        mask_db_ = mask_to_db(mask_nodes_);
    }

    void apply_ramp(std::size_t start_index, std::size_t end_index, float start_value, float end_value) {
        if (mask_nodes_.empty()) {
            return;
        }

        if (start_index > end_index) {
            std::swap(start_index, end_index);
            std::swap(start_value, end_value);
        }

        start_index = std::min(start_index, mask_nodes_.size() - 1);
        end_index = std::min(end_index, mask_nodes_.size() - 1);
        const std::size_t span = std::max<std::size_t>(1, end_index - start_index);

        for (std::size_t i = start_index; i <= end_index; ++i) {
            const float t = static_cast<float>(i - start_index) / static_cast<float>(span);
            mask_nodes_[i] = sonarium::core::params::clamp_normalized(start_value + ((end_value - start_value) * t));
        }

        engine_.set_gain_mask_nodes(mask_nodes_);
        refresh_analysis();
    }

    void reset_nearest_node() {
        if (!has_hover_node_ || hover_node_ >= mask_nodes_.size()) {
            return;
        }

        mask_nodes_[hover_node_] = 0.5f;
        engine_.set_gain_mask_nodes(mask_nodes_);
        refresh_analysis();

        if (debug_events_) {
            std::cout << "[prototype] Right click reset node=" << hover_node_ << std::endl;
        }
    }

    void begin_drag(float mouse_x, float mouse_y) {
        if (!contains(mask_rect_, mouse_x, mouse_y)) {
            dragging_ = false;
            return;
        }

        const std::size_t node = static_cast<std::size_t>(map_x_to_index(mask_rect_, mouse_x, mask_nodes_.size()));
        const float value = map_y_to_normalized(mask_rect_, mouse_y);

        apply_ramp(node, node, value, value);
        dragging_ = true;
        last_drag_node_ = node;
        last_drag_value_ = value;
        active_node_ = node;

        if (debug_events_) {
            std::cout << "[prototype] Begin drag node=" << node << " value=" << value << std::endl;
        }
    }

    void continue_drag(float mouse_x, float mouse_y) {
        if (!contains(mask_rect_, mouse_x, mouse_y)) {
            return;
        }

        const std::size_t node = static_cast<std::size_t>(map_x_to_index(mask_rect_, mouse_x, mask_nodes_.size()));
        const float value = map_y_to_normalized(mask_rect_, mouse_y);

        if (node == last_drag_node_ && std::abs(value - last_drag_value_) < 0.001f) {
            return;
        }

        apply_ramp(last_drag_node_, node, last_drag_value_, value);
        last_drag_node_ = node;
        last_drag_value_ = value;
        active_node_ = node;

        if (debug_events_) {
            std::cout << "[prototype] Drag node=" << node << " value=" << value << std::endl;
        }
    }

    void on_key(GLFWwindow* window, int key, int action) {
        if (action != GLFW_PRESS) {
            return;
        }

        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return;
        }

        if (key == GLFW_KEY_R) {
            engine_.update_state(sonarium::core::state::EngineState{});
            mask_nodes_ = engine_.state().gain_mask.mask_nodes_normalized;
            refresh_analysis();
            active_node_ = 0;

            if (debug_events_) {
                std::cout << "[prototype] Full reset." << std::endl;
            }
        }
    }

    void on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                begin_drag(static_cast<float>(x), static_cast<float>(y));
            } else if (action == GLFW_RELEASE) {
                dragging_ = false;
            }
            return;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && hovering_mask_) {
            if ((mods & GLFW_MOD_SHIFT) != 0) {
                engine_.update_state(sonarium::core::state::EngineState{});
                mask_nodes_ = engine_.state().gain_mask.mask_nodes_normalized;
                refresh_analysis();
                if (debug_events_) {
                    std::cout << "[prototype] Shift+right click full reset." << std::endl;
                }
            } else {
                reset_nearest_node();
            }
        }
    }

    void on_cursor_move(float x, float y) {
        cursor_x_ = x;
        cursor_y_ = y;
        hovering_mask_ = contains(mask_rect_, x, y);

        if (hovering_mask_) {
            hover_node_ = static_cast<std::size_t>(map_x_to_index(mask_rect_, x, mask_nodes_.size()));
            hover_value_ = map_y_to_normalized(mask_rect_, y);
            has_hover_node_ = true;
        } else {
            has_hover_node_ = false;
        }

        if (dragging_) {
            continue_drag(x, y);
        }
    }

    void draw_help_overlay() const {
        const Rect overlay{26.0f, 6.0f, 560.0f, 56.0f};
        glColor3f(0.95f, 0.95f, 0.98f);
        draw_filled_rect(overlay);
        glColor3f(0.25f, 0.25f, 0.30f);
        draw_rect_outline(overlay);

        glColor3f(0.12f, 0.12f, 0.15f);
        draw_text(34.0f, 14.0f, "MASK EDIT: LMB CLICK/DRAG | RMB RESET NODE", 2.0f);
        draw_text(34.0f, 30.0f, "SHIFT+RMB RESET ALL | R RESET ALL | Q OR ESC QUIT", 2.0f);
    }

    void draw_mask_feedback() const {
        if (!has_hover_node_ || hover_node_ >= mask_nodes_.size()) {
            return;
        }

        const float x = map_index_to_x(mask_rect_, hover_node_, mask_nodes_.size());
        const float hover_y = map_normalized_to_y(mask_rect_, hover_value_);
        const float node_y = map_normalized_to_y(mask_rect_, mask_nodes_[hover_node_]);

        glColor3f(0.15f, 0.15f, 0.18f);
        glBegin(GL_LINES);
        glVertex2f(x, mask_rect_.y);
        glVertex2f(x, mask_rect_.y + mask_rect_.h);
        glEnd();

        glColor3f(0.95f, 0.50f, 0.18f);
        const Rect hover_dot{x - 2.0f, hover_y - 2.0f, 4.0f, 4.0f};
        draw_filled_rect(hover_dot);

        glColor3f(0.72f, 0.10f, 0.10f);
        const Rect node_dot{x - 3.0f, node_y - 3.0f, 6.0f, 6.0f};
        draw_filled_rect(node_dot);

        std::ostringstream line;
        line.setf(std::ios::fixed);
        line.precision(2);
        line << "NODE " << hover_node_ << "  NORM " << hover_value_ << "  DB " << to_db(sonarium::core::params::normalized_to_linear_gain(hover_value_));

        const Rect tooltip{mask_rect_.x + 10.0f, mask_rect_.y + 10.0f, 400.0f, 20.0f};
        glColor3f(0.95f, 0.95f, 0.95f);
        draw_filled_rect(tooltip);
        glColor3f(0.10f, 0.10f, 0.10f);
        draw_rect_outline(tooltip);
        draw_text(tooltip.x + 6.0f, tooltip.y + 5.0f, line.str(), 1.8f);
    }

    void draw(int window_w, int window_h, int framebuffer_w, int framebuffer_h) {
        glViewport(0, 0, framebuffer_w, framebuffer_h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, static_cast<double>(window_w), static_cast<double>(window_h), 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(0.99f, 0.99f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const float margin = 24.0f;
        const float plot_height = (static_cast<float>(window_h) - (margin * 5.0f)) / 4.0f;

        pre_rect_ = Rect{margin, margin + 44.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};
        mask_rect_ =
            Rect{margin, pre_rect_.y + pre_rect_.h + margin + 10.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};
        post_rect_ =
            Rect{margin, mask_rect_.y + mask_rect_.h + margin + 10.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};

        glColor3f(0.96f, 0.98f, 1.0f);
        draw_filled_rect(pre_rect_);
        draw_filled_rect(post_rect_);

        glColor3f(hovering_mask_ ? 0.92f : 0.97f, hovering_mask_ ? 0.95f : 0.97f, hovering_mask_ ? 1.0f : 0.97f);
        draw_filled_rect(mask_rect_);

        glColor3f(0.20f, 0.20f, 0.24f);
        draw_rect_outline(pre_rect_);
        draw_rect_outline(mask_rect_);
        draw_rect_outline(post_rect_);

        glColor3f(0.2f, 0.2f, 0.2f);
        draw_text(pre_rect_.x + 6.0f, pre_rect_.y - 14.0f, "PRE SPECTRUM DB", 2.0f);
        draw_text(mask_rect_.x + 6.0f, mask_rect_.y - 14.0f, "GAIN MASK DB", 2.0f);
        draw_text(post_rect_.x + 6.0f, post_rect_.y - 14.0f, "POST SPECTRUM DB", 2.0f);

        glColor3f(0.10f, 0.24f, 0.74f);
        draw_curve(pre_rect_, pre_db_, -120.0f, 12.0f);

        // Zero-gain baseline in mask panel.
        glColor3f(0.5f, 0.5f, 0.5f);
        const float zero_db_y = map_db_to_y(mask_rect_, 0.0f, -24.0f, 24.0f);
        glBegin(GL_LINES);
        glVertex2f(mask_rect_.x, zero_db_y);
        glVertex2f(mask_rect_.x + mask_rect_.w, zero_db_y);
        glEnd();

        // Filled mask area for better visual feedback.
        glColor3f(0.96f, 0.86f, 0.84f);
        glBegin(GL_TRIANGLE_STRIP);
        for (std::size_t i = 0; i < mask_nodes_.size(); ++i) {
            const float x = map_index_to_x(mask_rect_, i, mask_nodes_.size());
            const float y = map_normalized_to_y(mask_rect_, mask_nodes_[i]);
            glVertex2f(x, zero_db_y);
            glVertex2f(x, y);
        }
        glEnd();

        glColor3f(0.85f, 0.20f, 0.12f);
        draw_curve(mask_rect_, mask_db_, -24.0f, 24.0f);

        glColor3f(0.40f, 0.40f, 0.40f);
        for (std::size_t i = 0; i < mask_nodes_.size(); i += 8) {
            const float x = map_index_to_x(mask_rect_, i, mask_nodes_.size());
            glBegin(GL_LINES);
            glVertex2f(x, mask_rect_.y + mask_rect_.h);
            glVertex2f(x, mask_rect_.y + mask_rect_.h + 4.0f);
            glEnd();
        }

        if (active_node_ < mask_nodes_.size()) {
            const float x = map_index_to_x(mask_rect_, active_node_, mask_nodes_.size());
            const float y = map_normalized_to_y(mask_rect_, mask_nodes_[active_node_]);
            glColor3f(0.05f, 0.05f, 0.05f);
            const Rect active_dot{x - 3.0f, y - 3.0f, 6.0f, 6.0f};
            draw_filled_rect(active_dot);
        }

        draw_mask_feedback();

        glColor3f(0.00f, 0.45f, 0.20f);
        draw_curve(post_rect_, post_db_, -120.0f, 12.0f);

        draw_help_overlay();
    }

    std::vector<float> input_;
    sonarium::engine::EngineFacade engine_;

    std::vector<float> mask_nodes_;
    std::vector<float> pre_db_;
    std::vector<float> post_db_;
    std::vector<float> mask_db_;

    Rect pre_rect_;
    Rect mask_rect_;
    Rect post_rect_;

    bool dragging_ = false;
    bool hovering_mask_ = false;
    bool has_hover_node_ = false;
    bool debug_events_ = false;
    std::size_t hover_node_ = 0;
    std::size_t active_node_ = 0;
    std::size_t last_drag_node_ = 0;
    float hover_value_ = 0.5f;
    float last_drag_value_ = 0.5f;
    float cursor_x_ = 0.0f;
    float cursor_y_ = 0.0f;
};

}  // namespace

int main() {
    PrototypeWindow app;
    return app.run() ? 0 : 1;
}
