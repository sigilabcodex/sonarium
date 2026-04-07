#pragma once

#include "core/processors/spectral_gain_mask_processor.h"
#include "core/state/engine_state.h"
#include "core/stft/stft_engine.h"

#include <vector>

namespace sonarium::engine {

class EngineFacade {
  public:
    explicit EngineFacade(core::stft::StftConfig stft_config);

    void update_state(const core::state::EngineState& state);

    [[nodiscard]] std::vector<float> process_mono(const std::vector<float>& input) const;

    [[nodiscard]] const core::stft::StftConfig& config() const noexcept { return stft_.config(); }

  private:
    core::stft::StftEngine stft_;
    core::processors::SpectralGainMaskProcessor gain_mask_processor_;
};

}  // namespace sonarium::engine
