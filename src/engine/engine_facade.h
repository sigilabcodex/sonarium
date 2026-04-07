#pragma once

#include "core/processors/spectral_gain_mask_processor.h"
#include "core/state/engine_state.h"
#include "core/stft/stft_engine.h"

#include <cstddef>
#include <vector>

namespace sonarium::engine {

class EngineFacade {
  public:
    explicit EngineFacade(core::stft::StftConfig stft_config);

    void update_state(const core::state::EngineState& state);
    void set_gain_mask_node(std::size_t node_index, float normalized_value);
    void set_gain_mask_nodes(const std::vector<float>& normalized_nodes);

    [[nodiscard]] core::state::EngineState state() const;
    [[nodiscard]] std::vector<float> process_mono(const std::vector<float>& input) const;

    [[nodiscard]] const core::stft::StftConfig& config() const noexcept { return stft_.config(); }

  private:
    void sync_processors_from_state();

    core::stft::StftEngine stft_;
    core::processors::SpectralGainMaskProcessor gain_mask_processor_;
    core::state::EngineState state_;
};

}  // namespace sonarium::engine
