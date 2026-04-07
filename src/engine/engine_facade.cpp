#include "engine/engine_facade.h"

namespace sonarium::engine {

EngineFacade::EngineFacade(core::stft::StftConfig stft_config)
    : stft_(stft_config), gain_mask_processor_(stft_config.fft_size) {}

void EngineFacade::update_state(const core::state::EngineState& state) {
    gain_mask_processor_.update_state(state.gain_mask);
}

std::vector<float> EngineFacade::process_mono(const std::vector<float>& input) const {
    return stft_.process_mono(input, [this](core::stft::StftEngine::SpectralFrame& frame) {
        gain_mask_processor_.process_frame(frame);
    });
}

}  // namespace sonarium::engine
