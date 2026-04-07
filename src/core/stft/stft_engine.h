#pragma once

#include <complex>
#include <cstddef>
#include <functional>
#include <vector>

namespace sonarium::core::stft {

struct StftConfig {
    std::size_t fft_size = 1024;
    std::size_t hop_size = 256;
};

class StftEngine {
  public:
    using SpectralFrame = std::vector<std::complex<float>>;
    using FrameProcessor = std::function<void(SpectralFrame&)>;

    explicit StftEngine(StftConfig config);

    [[nodiscard]] const StftConfig& config() const noexcept { return config_; }

    [[nodiscard]] std::vector<float> process_mono(const std::vector<float>& input,
                                                  const FrameProcessor& processor) const;

  private:
    [[nodiscard]] std::vector<float> make_hann_window(std::size_t size) const;
    void forward_dft(const std::vector<float>& time, SpectralFrame& freq) const;
    void inverse_dft(const SpectralFrame& freq, std::vector<float>& time) const;

    StftConfig config_;
    std::vector<float> window_;
};

}  // namespace sonarium::core::stft
