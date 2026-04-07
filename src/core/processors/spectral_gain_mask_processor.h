#pragma once

#include "core/params/parameter_model.h"
#include "core/state/engine_state.h"
#include "core/stft/stft_engine.h"

#include <cstddef>
#include <vector>

namespace sonarium::core::processors {

class SpectralGainMaskProcessor {
  public:
    explicit SpectralGainMaskProcessor(std::size_t fft_size);

    void update_state(const state::SpectralGainMaskParameters& state);
    void process_frame(stft::StftEngine::SpectralFrame& frame) const;

    [[nodiscard]] const std::vector<float>& bin_gains() const noexcept { return bin_gains_; }

  private:
    std::size_t fft_size_;
    float wet_mix_ = 1.0f;
    std::vector<float> bin_gains_;
};

}  // namespace sonarium::core::processors
