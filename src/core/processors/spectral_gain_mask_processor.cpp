#include "core/processors/spectral_gain_mask_processor.h"

#include <algorithm>
#include <cmath>

namespace sonarium::core::processors {

SpectralGainMaskProcessor::SpectralGainMaskProcessor(std::size_t fft_size)
    : fft_size_(fft_size), bin_gains_(fft_size, 1.0f) {}

void SpectralGainMaskProcessor::update_state(const state::SpectralGainMaskParameters& state) {
    wet_mix_ = params::clamp_normalized(state.wet_mix_normalized);

    if (state.mask_nodes_normalized.empty()) {
        std::fill(bin_gains_.begin(), bin_gains_.end(), 1.0f);
        return;
    }

    const std::size_t node_count = state.mask_nodes_normalized.size();
    for (std::size_t bin = 0; bin < fft_size_; ++bin) {
        const float node_pos = (static_cast<float>(bin) / static_cast<float>(fft_size_ - 1)) *
                               static_cast<float>(node_count - 1);
        const std::size_t left = static_cast<std::size_t>(std::floor(node_pos));
        const std::size_t right = std::min(left + 1, node_count - 1);
        const float frac = node_pos - static_cast<float>(left);

        const float left_gain = params::normalized_to_linear_gain(state.mask_nodes_normalized[left]);
        const float right_gain = params::normalized_to_linear_gain(state.mask_nodes_normalized[right]);
        const float gain = left_gain + ((right_gain - left_gain) * frac);
        bin_gains_[bin] = std::clamp(gain, 0.01f, 8.0f);
    }
}

void SpectralGainMaskProcessor::process_frame(stft::StftEngine::SpectralFrame& frame) const {
    const std::size_t bins = std::min(frame.size(), bin_gains_.size());
    for (std::size_t i = 0; i < bins; ++i) {
        const float mixed_gain = (1.0f - wet_mix_) + (wet_mix_ * bin_gains_[i]);
        frame[i] *= mixed_gain;
    }
}

}  // namespace sonarium::core::processors
