#pragma once

#include <cstddef>
#include <vector>

namespace sonarium::core::state {

struct SpectralGainMaskParameters {
    float wet_mix_normalized = 1.0f;
    std::vector<float> mask_nodes_normalized = std::vector<float>(64, 0.5f);
};

struct EngineState {
    SpectralGainMaskParameters gain_mask;
};

}  // namespace sonarium::core::state
