#include "core/state/engine_state.h"
#include "engine/engine_facade.h"
#include "test_framework.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

SONARIUM_TEST(gain_mask_reduces_high_frequency_energy) {
    constexpr std::size_t sample_rate = 48000;
    constexpr std::size_t frames = 8192;

    std::vector<float> input(frames, 0.0f);
    for (std::size_t i = 0; i < frames; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(sample_rate);
        const float low = 0.5f * std::sin(2.0f * static_cast<float>(std::numbers::pi) * 250.0f * t);
        const float high = 0.5f * std::sin(2.0f * static_cast<float>(std::numbers::pi) * 6000.0f * t);
        input[i] = low + high;
    }

    sonarium::engine::EngineFacade engine({1024, 256});
    sonarium::core::state::EngineState state;
    for (std::size_t i = 0; i < state.gain_mask.mask_nodes_normalized.size(); ++i) {
        const float x = static_cast<float>(i) / static_cast<float>(state.gain_mask.mask_nodes_normalized.size() - 1);
        state.gain_mask.mask_nodes_normalized[i] = 0.45f - (0.3f * x);
    }
    state.gain_mask.wet_mix_normalized = 1.0f;
    engine.update_state(state);

    const auto output = engine.process_mono(input);

    const auto rms = [](const std::vector<float>& buffer) {
        float sum = 0.0f;
        for (float x : buffer) {
            sum += x * x;
        }
        return std::sqrt(sum / static_cast<float>(buffer.size()));
    };

    const float input_rms = rms(input);
    const float output_rms = rms(output);

    REQUIRE_TRUE(std::isfinite(output_rms), "Output RMS should be finite");
    REQUIRE_TRUE(output_rms < input_rms, "Mask should attenuate overall energy in this scenario");
}

SONARIUM_TEST(gain_mask_output_stays_finite_under_extreme_nodes) {
    std::vector<float> input(4096, 0.01f);

    sonarium::engine::EngineFacade engine({256, 64});
    sonarium::core::state::EngineState state;
    for (std::size_t i = 0; i < state.gain_mask.mask_nodes_normalized.size(); ++i) {
        state.gain_mask.mask_nodes_normalized[i] = (i % 2 == 0) ? 0.0f : 1.0f;
    }
    state.gain_mask.wet_mix_normalized = 1.0f;
    engine.update_state(state);

    const auto output = engine.process_mono(input);
    const bool all_finite = std::all_of(output.begin(), output.end(), [](float s) { return std::isfinite(s); });
    REQUIRE_TRUE(all_finite, "Output contains NaN/Inf");
}


SONARIUM_TEST(engine_facade_single_node_updates_flow_through_state_boundary) {
    sonarium::engine::EngineFacade engine({512, 128});

    engine.set_gain_mask_node(3, 0.9f);
    auto state = engine.state();
    REQUIRE_TRUE(std::abs(state.gain_mask.mask_nodes_normalized[3] - 0.9f) < 1e-6f,
                 "Node update should be reflected in engine state snapshot");

    engine.set_gain_mask_node(3, -1.0f);
    state = engine.state();
    REQUIRE_TRUE(std::abs(state.gain_mask.mask_nodes_normalized[3] - 0.0f) < 1e-6f,
                 "Node update should be clamped to normalized range");
}

void register_processor_tests() {}
