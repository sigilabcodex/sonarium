#include "core/stft/stft_engine.h"
#include "test_framework.h"

#include <cmath>
#include <numbers>
#include <vector>

SONARIUM_TEST(stft_roundtrip_sine_is_stable) {
    constexpr std::size_t sample_rate = 48000;
    constexpr float frequency_hz = 700.0f;
    constexpr std::size_t frames = 8192;

    std::vector<float> input(frames, 0.0f);
    for (std::size_t i = 0; i < frames; ++i) {
        const float phase = 2.0f * static_cast<float>(std::numbers::pi) * frequency_hz *
                            static_cast<float>(i) / static_cast<float>(sample_rate);
        input[i] = 0.5f * std::sin(phase);
    }

    sonarium::core::stft::StftEngine stft({512, 128});
    auto output = stft.process_mono(input, [](auto&) {});

    float mse = 0.0f;
    for (std::size_t i = 0; i < input.size(); ++i) {
        const float err = input[i] - output[i];
        mse += err * err;
    }
    mse /= static_cast<float>(input.size());

    REQUIRE_TRUE(std::isfinite(mse), "MSE must be finite");
    REQUIRE_TRUE(mse < 5e-4f, "STFT roundtrip MSE unexpectedly high");
}

void register_stft_tests() {}
