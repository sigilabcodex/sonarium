#include "core/params/parameter_model.h"
#include "engine/engine_facade.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <sstream>
#include <string>
#include <vector>

namespace {

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

float to_db(float mag) {
    constexpr float min_mag = 1e-6f;
    return 20.0f * std::log10(std::max(mag, min_mag));
}

std::string sparkline(const std::vector<float>& values, std::size_t width) {
    static const char* levels = " .:-=+*#%@";
    constexpr std::size_t level_count = 10;

    if (values.empty() || width == 0) {
        return "";
    }

    const auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
    const float min_v = *min_it;
    const float max_v = *max_it;
    const float range = std::max(max_v - min_v, 1e-6f);

    std::string line;
    line.reserve(width);
    for (std::size_t i = 0; i < width; ++i) {
        const std::size_t src = (i * values.size()) / width;
        const float norm = (values[std::min(src, values.size() - 1)] - min_v) / range;
        const std::size_t idx = static_cast<std::size_t>(norm * static_cast<float>(level_count - 1));
        line.push_back(levels[idx]);
    }

    return line;
}

std::vector<float> mask_to_db(const std::vector<float>& mask_nodes) {
    std::vector<float> out(mask_nodes.size(), 0.0f);
    for (std::size_t i = 0; i < mask_nodes.size(); ++i) {
        out[i] = to_db(sonarium::core::params::normalized_to_linear_gain(mask_nodes[i]));
    }
    return out;
}

void render_state(sonarium::engine::EngineFacade& engine, const std::vector<float>& input, std::size_t sample_rate) {
    const auto output = engine.process_mono(input);
    const auto pre = magnitude_spectrum(input, engine.config().fft_size);
    const auto post = magnitude_spectrum(output, engine.config().fft_size);
    const auto state = engine.state();
    const auto mask_db = mask_to_db(state.gain_mask.mask_nodes_normalized);

    std::vector<float> pre_db(pre.size(), 0.0f);
    std::vector<float> post_db(post.size(), 0.0f);
    std::vector<float> delta(post.size(), 0.0f);
    for (std::size_t i = 0; i < delta.size(); ++i) {
        pre_db[i] = to_db(pre[i]);
        post_db[i] = to_db(post[i]);
        delta[i] = post_db[i] - pre_db[i];
    }

    std::cout << "\n=== Sonarium Spectral Interaction Prototype ===\n";
    std::cout << "Pre spectrum : " << sparkline(pre_db, 64) << "\n";
    std::cout << "Post spectrum: " << sparkline(post_db, 64) << "\n";
    std::cout << "Delta (dB)   : " << sparkline(delta, 64) << "\n";
    std::cout << "Mask (dB)    : " << sparkline(mask_db, 64) << "\n\n";

    std::cout << "Bin summary (Hz / pre dB / post dB / delta dB / mask dB)\n";
    for (std::size_t i = 0; i < 8; ++i) {
        const std::size_t bin = (i * pre.size()) / 8;
        const float hz = static_cast<float>(bin) * static_cast<float>(sample_rate) /
                         static_cast<float>(engine.config().fft_size);
        const std::size_t node = (bin * state.gain_mask.mask_nodes_normalized.size()) / pre.size();
        std::cout << std::fixed << std::setprecision(1) << std::setw(6) << hz << " Hz | "
                  << std::setw(7) << to_db(pre[bin]) << " | " << std::setw(7) << to_db(post[bin]) << " | "
                  << std::setw(7) << (to_db(post[bin]) - to_db(pre[bin])) << " | "
                  << std::setw(7) << mask_db[std::min(node, mask_db.size() - 1)] << "\n";
    }
    std::cout << '\n';
}

void apply_drag(sonarium::engine::EngineFacade& engine,
                std::size_t start,
                std::size_t end,
                float start_value,
                float end_value) {
    auto state = engine.state();
    auto& nodes = state.gain_mask.mask_nodes_normalized;
    if (nodes.empty()) {
        return;
    }

    if (start > end) {
        std::swap(start, end);
        std::swap(start_value, end_value);
    }

    start = std::min(start, nodes.size() - 1);
    end = std::min(end, nodes.size() - 1);
    const std::size_t span = std::max<std::size_t>(1, end - start);

    for (std::size_t i = start; i <= end; ++i) {
        const float t = static_cast<float>(i - start) / static_cast<float>(span);
        nodes[i] = sonarium::core::params::clamp_normalized(start_value + ((end_value - start_value) * t));
    }

    engine.update_state(state);
}

}  // namespace

int main() {
    constexpr std::size_t sample_rate = 48000;
    const auto input = make_demo_signal(sample_rate, sample_rate / 2);

    sonarium::engine::EngineFacade engine({1024, 256});

    std::cout << "Commands:\n"
              << "  show\n"
              << "  set <node_index> <normalized_0_to_1>\n"
              << "  drag <start_node> <end_node> <start_value> <end_value>\n"
              << "  reset\n"
              << "  quit\n";

    render_state(engine, input, sample_rate);

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "quit" || command == "exit") {
            break;
        }

        if (command == "show") {
            render_state(engine, input, sample_rate);
            continue;
        }

        if (command == "set") {
            std::size_t node = 0;
            float value = 0.5f;
            if (iss >> node >> value) {
                engine.set_gain_mask_node(node, value);
                render_state(engine, input, sample_rate);
            } else {
                std::cout << "Usage: set <node_index> <normalized_0_to_1>\n";
            }
            continue;
        }

        if (command == "drag") {
            std::size_t start = 0;
            std::size_t end = 0;
            float start_value = 0.5f;
            float end_value = 0.5f;
            if (iss >> start >> end >> start_value >> end_value) {
                apply_drag(engine, start, end, start_value, end_value);
                render_state(engine, input, sample_rate);
            } else {
                std::cout << "Usage: drag <start_node> <end_node> <start_value> <end_value>\n";
            }
            continue;
        }

        if (command == "reset") {
            engine.update_state(sonarium::core::state::EngineState{});
            render_state(engine, input, sample_rate);
            continue;
        }

        std::cout << "Unknown command. Try: show, set, drag, reset, quit\n";
    }

    return 0;
}
