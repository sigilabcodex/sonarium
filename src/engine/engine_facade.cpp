#include "engine/engine_facade.h"

#include "core/params/parameter_model.h"

namespace sonarium::engine {

EngineFacade::EngineFacade(core::stft::StftConfig stft_config)
    : stft_(stft_config), gain_mask_processor_(stft_config.fft_size) {
    sync_processors_from_state();
}

void EngineFacade::update_state(const core::state::EngineState& state) {
    state_ = state;
    sync_processors_from_state();
}

void EngineFacade::set_gain_mask_node(std::size_t node_index, float normalized_value) {
    if (node_index >= state_.gain_mask.mask_nodes_normalized.size()) {
        return;
    }

    state_.gain_mask.mask_nodes_normalized[node_index] = core::params::clamp_normalized(normalized_value);
    sync_processors_from_state();
}

void EngineFacade::set_gain_mask_nodes(const std::vector<float>& normalized_nodes) {
    if (normalized_nodes.empty()) {
        return;
    }

    state_.gain_mask.mask_nodes_normalized = normalized_nodes;
    for (float& value : state_.gain_mask.mask_nodes_normalized) {
        value = core::params::clamp_normalized(value);
    }
    sync_processors_from_state();
}

core::state::EngineState EngineFacade::state() const {
    return state_;
}

std::vector<float> EngineFacade::process_mono(const std::vector<float>& input) const {
    return stft_.process_mono(input, [this](core::stft::StftEngine::SpectralFrame& frame) {
        gain_mask_processor_.process_frame(frame);
    });
}

void EngineFacade::sync_processors_from_state() {
    gain_mask_processor_.update_state(state_.gain_mask);
}

}  // namespace sonarium::engine
