#pragma once

#include "hzd/math.hpp"
#include "colors/rgb.hpp"

namespace j::colors {
  template<typename Float>
  constexpr Float srgb_gamma(Float f) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float dark_multiplier   = 12.92,
                    dark_light_cutoff = 0.0031308,
                    light_offset      = 0.055,
                    light_multiplier  = 1.055,
                    exponent          = 0.4166666666666666;
    return (f < dark_light_cutoff)
      ? j::fmax(Float(0.0), f * dark_multiplier)
      : j::fmin(Float(1.0), j::pow(f, exponent) * light_multiplier - light_offset);
  }

  template<typename Float>
  J_INLINE_GETTER constexpr Float srgb_inverse_gamma(Float f) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float dark_multiplier   = 0.07739938080495356037,
                    dark_light_cutoff = 0.04045,
                    light_offset      = 0.055,
                    light_multiplier  = 0.94786729857819905213;
    return (f < dark_light_cutoff)
      ? f * dark_multiplier
      : j::pow((f + light_offset) * light_multiplier, Float(2.4));
  }

  template<typename Float>
  J_INLINE_GETTER constexpr rgb<Float> srgb_to_linear_rgb(const rgb<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return rgb<Float>{ srgb_inverse_gamma(in.r), srgb_inverse_gamma(in.g), srgb_inverse_gamma(in.b) };
  }

  template<typename Float>
 J_INLINE_GETTER constexpr rgb<Float> linear_rgb_to_srgb(const rgb<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return rgb<Float>{srgb_gamma(in.r), srgb_gamma(in.g), srgb_gamma(in.b)};
  }
}
