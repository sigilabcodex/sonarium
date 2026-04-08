#include "core/params/parameter_model.h"
#include "engine/engine_facade.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <string>
#include <vector>

namespace {

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
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

bool contains(const Rect& rect, int px, int py) {
    return px >= rect.x && py >= rect.y && px < (rect.x + rect.w) && py < (rect.y + rect.h);
}

int map_x_to_index(const Rect& rect, int px, std::size_t size) {
    if (size < 2) {
        return 0;
    }

    const float u = std::clamp(static_cast<float>(px - rect.x) / static_cast<float>(std::max(1, rect.w - 1)), 0.0f, 1.0f);
    return static_cast<int>(u * static_cast<float>(size - 1));
}

float map_y_to_normalized(const Rect& rect, int py) {
    const float v = std::clamp(static_cast<float>(py - rect.y) / static_cast<float>(std::max(1, rect.h - 1)), 0.0f, 1.0f);
    return sonarium::core::params::clamp_normalized(1.0f - v);
}

int map_index_to_x(const Rect& rect, std::size_t index, std::size_t size) {
    if (size < 2) {
        return rect.x;
    }

    const float u = static_cast<float>(index) / static_cast<float>(size - 1);
    return rect.x + static_cast<int>(u * static_cast<float>(rect.w - 1));
}

void draw_frame(Display* display, Drawable drawable, GC gc, const Rect& rect) {
    XDrawRectangle(display, drawable, gc, rect.x, rect.y, rect.w, rect.h);
}

void draw_label(Display* display, Drawable drawable, GC gc, int x, int y, const std::string& label) {
    XDrawString(display, drawable, gc, x, y, label.c_str(), static_cast<int>(label.size()));
}

void draw_curve(Display* display,
                Drawable drawable,
                GC gc,
                const Rect& rect,
                const std::vector<float>& values,
                float min_value,
                float max_value) {
    if (values.size() < 2) {
        return;
    }

    const float span = std::max(1e-6f, max_value - min_value);
    for (std::size_t i = 1; i < values.size(); ++i) {
        const int x0 = map_index_to_x(rect, i - 1, values.size());
        const int x1 = map_index_to_x(rect, i, values.size());

        const float y0_n = std::clamp((values[i - 1] - min_value) / span, 0.0f, 1.0f);
        const float y1_n = std::clamp((values[i] - min_value) / span, 0.0f, 1.0f);

        const int y0 = rect.y + rect.h - 1 - static_cast<int>(y0_n * static_cast<float>(rect.h - 1));
        const int y1 = rect.y + rect.h - 1 - static_cast<int>(y1_n * static_cast<float>(rect.h - 1));
        XDrawLine(display, drawable, gc, x0, y0, x1, y1);
    }
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

    void run() {
        Display* display = XOpenDisplay(nullptr);
        if (display == nullptr) {
            return;
        }

        const int screen = DefaultScreen(display);
        Window window = XCreateSimpleWindow(display,
                                            RootWindow(display, screen),
                                            120,
                                            120,
                                            kWindowWidth,
                                            kWindowHeight,
                                            1,
                                            BlackPixel(display, screen),
                                            WhitePixel(display, screen));

        XStoreName(display, window, "Sonarium Spectral Interaction Prototype");
        XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask |
                                          PointerMotionMask | StructureNotifyMask);

        Atom wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &wm_delete, 1);

        GC gc = XCreateGC(display, window, 0, nullptr);
        XMapWindow(display, window);

        bool running = true;
        while (running) {
            XEvent event;
            XNextEvent(display, &event);

            switch (event.type) {
                case Expose:
                    if (event.xexpose.count == 0) {
                        draw(display, window, gc, event.xexpose.width, event.xexpose.height);
                    }
                    break;
                case ConfigureNotify:
                    draw(display, window, gc, event.xconfigure.width, event.xconfigure.height);
                    break;
                case ButtonPress:
                    if (event.xbutton.button == Button1) {
                        begin_drag(event.xbutton.x, event.xbutton.y);
                        draw(display, window, gc, kWindowWidth, kWindowHeight);
                    }
                    break;
                case MotionNotify:
                    if (dragging_) {
                        continue_drag(event.xmotion.x, event.xmotion.y);
                        draw(display, window, gc, kWindowWidth, kWindowHeight);
                    }
                    break;
                case ButtonRelease:
                    if (event.xbutton.button == Button1) {
                        dragging_ = false;
                    }
                    break;
                case KeyPress: {
                    char buffer[8] = {};
                    KeySym key = 0;
                    XLookupString(&event.xkey, buffer, sizeof(buffer), &key, nullptr);
                    if (buffer[0] == 'q') {
                        running = false;
                    }
                    if (buffer[0] == 'r') {
                        engine_.update_state(sonarium::core::state::EngineState{});
                        mask_nodes_ = engine_.state().gain_mask.mask_nodes_normalized;
                        refresh_analysis();
                        draw(display, window, gc, kWindowWidth, kWindowHeight);
                    }
                    break;
                }
                case ClientMessage:
                    if (static_cast<Atom>(event.xclient.data.l[0]) == wm_delete) {
                        running = false;
                    }
                    break;
                default:
                    break;
            }
        }

        XFreeGC(display, gc);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
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

    void begin_drag(int mouse_x, int mouse_y) {
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

    void continue_drag(int mouse_x, int mouse_y) {
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

    void draw(Display* display, Window window, GC gc, int width, int height) {
        XClearWindow(display, window);

        const int margin = 24;
        const int plot_height = (height - (margin * 5)) / 4;

        pre_rect_ = Rect{margin, margin + 20, width - (margin * 2), plot_height};
        mask_rect_ = Rect{margin, pre_rect_.y + pre_rect_.h + margin + 10, width - (margin * 2), plot_height};
        post_rect_ = Rect{margin, mask_rect_.y + mask_rect_.h + margin + 10, width - (margin * 2), plot_height};

        draw_label(display, window, gc, margin, margin + 12, "Sonarium Spectral Interaction Prototype | q=quit, r=reset");

        draw_label(display, window, gc, pre_rect_.x, pre_rect_.y - 8, "Pre Spectrum (dB)");
        draw_frame(display, window, gc, pre_rect_);
        draw_curve(display, window, gc, pre_rect_, pre_db_, -120.0f, 12.0f);

        draw_label(display, window, gc, mask_rect_.x, mask_rect_.y - 8,
                   "Gain Mask Editor (click=set node, drag=draw ramp, range -24dB..+24dB)");
        draw_frame(display, window, gc, mask_rect_);
        draw_curve(display, window, gc, mask_rect_, mask_db_, -24.0f, 24.0f);

        for (std::size_t i = 0; i < mask_nodes_.size(); i += 8) {
            const int x = map_index_to_x(mask_rect_, i, mask_nodes_.size());
            XDrawLine(display, window, gc, x, mask_rect_.y + mask_rect_.h, x, mask_rect_.y + mask_rect_.h + 4);
        }

        draw_label(display, window, gc, post_rect_.x, post_rect_.y - 8, "Post Spectrum (dB)");
        draw_frame(display, window, gc, post_rect_);
        draw_curve(display, window, gc, post_rect_, post_db_, -120.0f, 12.0f);

        const std::string note =
            "Engine updates route through EngineFacade::set_gain_mask_nodes(); spectrum is recomputed after each edit.";
        draw_label(display, window, gc, margin, post_rect_.y + post_rect_.h + 24, note);

        XFlush(display);
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
    app.run();
    return 0;
}
