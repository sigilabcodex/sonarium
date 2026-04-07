#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string_view>

namespace sonarium::core::params {

enum class ParameterId {
    WetMix,
    MaskNodeBase,
};

struct ParameterDescriptor {
    ParameterId id;
    std::string_view name;
    float normalized_default;
};

inline constexpr ParameterDescriptor kWetMixDescriptor{
    ParameterId::WetMix,
    "wet_mix",
    1.0f,
};

inline float clamp_normalized(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

inline float normalized_to_db(float normalized) {
    const float clamped = clamp_normalized(normalized);
    return -24.0f + (48.0f * clamped);
}

inline float db_to_linear(float db) {
    return std::pow(10.0f, db / 20.0f);
}

inline float normalized_to_linear_gain(float normalized) {
    return db_to_linear(normalized_to_db(normalized));
}

}  // namespace sonarium::core::params
