#include "core/stft/stft_engine.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace sonarium::core::stft {

StftEngine::StftEngine(StftConfig config) : config_(config), window_(make_hann_window(config.fft_size)) {
    if (config_.hop_size == 0 || config_.hop_size > config_.fft_size) {
        throw std::invalid_argument("hop_size must be in range (0, fft_size]");
    }
}

std::vector<float> StftEngine::process_mono(const std::vector<float>& input,
                                            const FrameProcessor& processor) const {
    const std::size_t fft_size = config_.fft_size;
    const std::size_t hop = config_.hop_size;
    const std::size_t padded_input_size = input.size() + fft_size;
    std::vector<float> padded_input(padded_input_size, 0.0f);
    std::copy(input.begin(), input.end(), padded_input.begin());

    std::vector<float> output(padded_input_size, 0.0f);
    std::vector<float> normalization(padded_input_size, 0.0f);

    std::vector<float> frame(fft_size, 0.0f);
    std::vector<float> time_domain(fft_size, 0.0f);
    SpectralFrame freq(fft_size);

    for (std::size_t pos = 0; pos + fft_size <= padded_input_size; pos += hop) {
        for (std::size_t i = 0; i < fft_size; ++i) {
            frame[i] = padded_input[pos + i] * window_[i];
        }

        forward_dft(frame, freq);
        processor(freq);
        inverse_dft(freq, time_domain);

        for (std::size_t i = 0; i < fft_size; ++i) {
            const float weighted = time_domain[i] * window_[i];
            output[pos + i] += weighted;
            normalization[pos + i] += window_[i] * window_[i];
        }
    }

    for (std::size_t i = 0; i < output.size(); ++i) {
        if (normalization[i] > 1e-12f) {
            output[i] /= normalization[i];
        }
    }

    output.resize(input.size());
    return output;
}

std::vector<float> StftEngine::make_hann_window(std::size_t size) const {
    std::vector<float> window(size, 0.0f);
    const double two_pi = 2.0 * std::numbers::pi;
    for (std::size_t i = 0; i < size; ++i) {
        window[i] = static_cast<float>(0.5 - 0.5 * std::cos(two_pi * static_cast<double>(i) /
                                                            static_cast<double>(size)));
    }
    return window;
}

void StftEngine::forward_dft(const std::vector<float>& time, SpectralFrame& freq) const {
    const std::size_t n = time.size();
    const double two_pi = 2.0 * std::numbers::pi;
    for (std::size_t k = 0; k < n; ++k) {
        std::complex<double> acc{0.0, 0.0};
        for (std::size_t t = 0; t < n; ++t) {
            const double phase = -two_pi * static_cast<double>(k * t) / static_cast<double>(n);
            acc += static_cast<double>(time[t]) * std::complex<double>{std::cos(phase), std::sin(phase)};
        }
        freq[k] = {static_cast<float>(acc.real()), static_cast<float>(acc.imag())};
    }
}

void StftEngine::inverse_dft(const SpectralFrame& freq, std::vector<float>& time) const {
    const std::size_t n = freq.size();
    const double two_pi = 2.0 * std::numbers::pi;
    for (std::size_t t = 0; t < n; ++t) {
        std::complex<double> acc{0.0, 0.0};
        for (std::size_t k = 0; k < n; ++k) {
            const double phase = two_pi * static_cast<double>(k * t) / static_cast<double>(n);
            acc += std::complex<double>{static_cast<double>(freq[k].real()), static_cast<double>(freq[k].imag())} *
                   std::complex<double>{std::cos(phase), std::sin(phase)};
        }
        time[t] = static_cast<float>(acc.real() / static_cast<double>(n));
    }
}

}  // namespace sonarium::core::stft
