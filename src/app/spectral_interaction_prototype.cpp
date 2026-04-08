#include "core/params/parameter_model.h"
#include "engine/engine_facade.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
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

class PrototypeWindow {
  public:
    PrototypeWindow()
        : input_(make_demo_signal(kSampleRate, kSampleRate / 2)),
          engine_({1024, 256}),
          mask_nodes_(engine_.state().gain_mask.mask_nodes_normalized),
          pre_db_(to_db_spectrum(magnitude_spectrum(input_, engine_.config().fft_size))) {
        refresh_analysis();
    }

    bool run() {
        if (glfwInit() == GLFW_FALSE) {
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

        GLFWwindow* window =
            glfwCreateWindow(kWindowWidth, kWindowHeight, "Sonarium Spectral Interaction Prototype", nullptr, nullptr);
        if (window == nullptr) {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwSetWindowUserPointer(window, this);

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int, int action, int) {
            auto* self = static_cast<PrototypeWindow*>(glfwGetWindowUserPointer(w));
            if (self != nullptr) {
                self->on_key(w, key, action);
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int) {
            auto* self = static_cast<PrototypeWindow*>(glfwGetWindowUserPointer(w));
            if (self != nullptr) {
                self->on_mouse_button(w, button, action);
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
        }
    }

    void on_mouse_button(GLFWwindow* window, int button, int action) {
        if (button != GLFW_MOUSE_BUTTON_LEFT) {
            return;
        }

        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);

        if (action == GLFW_PRESS) {
            begin_drag(static_cast<float>(x), static_cast<float>(y));
        } else if (action == GLFW_RELEASE) {
            dragging_ = false;
        }
    }

    void on_cursor_move(float x, float y) {
        if (dragging_) {
            continue_drag(x, y);
        }
    }

    void draw(int window_w, int window_h, int framebuffer_w, int framebuffer_h) {
        glViewport(0, 0, framebuffer_w, framebuffer_h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, static_cast<double>(window_w), static_cast<double>(window_h), 0.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        const float margin = 24.0f;
        const float plot_height = (static_cast<float>(window_h) - (margin * 5.0f)) / 4.0f;

        pre_rect_ = Rect{margin, margin + 20.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};
        mask_rect_ =
            Rect{margin, pre_rect_.y + pre_rect_.h + margin + 10.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};
        post_rect_ =
            Rect{margin, mask_rect_.y + mask_rect_.h + margin + 10.0f, static_cast<float>(window_w) - (margin * 2.0f), plot_height};

        glColor3f(0.15f, 0.15f, 0.15f);
        draw_rect_outline(pre_rect_);
        draw_rect_outline(mask_rect_);
        draw_rect_outline(post_rect_);

        glColor3f(0.10f, 0.24f, 0.74f);
        draw_curve(pre_rect_, pre_db_, -120.0f, 12.0f);

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

        glColor3f(0.00f, 0.45f, 0.20f);
        draw_curve(post_rect_, post_db_, -120.0f, 12.0f);

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
    std::size_t last_drag_node_ = 0;
    float last_drag_value_ = 0.5f;
};

}  // namespace

int main() {
    PrototypeWindow app;
    return app.run() ? 0 : 1;
}
