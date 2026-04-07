#include "engine/engine_facade.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <numbers>
#include <vector>

int main() {
    constexpr std::size_t sample_rate = 48000;
    constexpr float frequency_hz = 1000.0f;
    constexpr std::size_t frames = sample_rate / 4;

    std::vector<float> input(frames, 0.0f);
    for (std::size_t i = 0; i < frames; ++i) {
        const float phase = 2.0f * static_cast<float>(std::numbers::pi) * frequency_hz *
                            static_cast<float>(i) / static_cast<float>(sample_rate);
        input[i] = std::sin(phase);
    }

    sonarium::engine::EngineFacade engine({1024, 256});
    sonarium::core::state::EngineState state;

    for (std::size_t i = 0; i < state.gain_mask.mask_nodes_normalized.size(); ++i) {
        const float x = static_cast<float>(i) / static_cast<float>(state.gain_mask.mask_nodes_normalized.size() - 1);
        state.gain_mask.mask_nodes_normalized[i] = 0.5f - (0.35f * x);
    }

    engine.update_state(state);
    auto output = engine.process_mono(input);

    float peak = 0.0f;
    for (float sample : output) {
        peak = std::max(peak, std::abs(sample));
    }

    std::cout << "Processed " << output.size() << " samples. Peak amplitude: " << peak << '\n';
    return 0;
}
