#pragma once

#include "geometry/vec3.hpp"
#include "colors/rgb.hpp"
#include "colors/srgb.hpp"

namespace j::colors {
  using xyzf = geometry::vec3f;

  template<typename Float>
  constexpr geometry::vec3<Float> linear_rgb_to_xyz_d65(const rgb<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float rx = 0.4124564,
                    gx = 0.3575761,
                    bx = 0.1804375,

                    ry = 0.2126729,
                    gy = 0.7151522,
                    by = 0.0721750,

                    rz = 0.0193339,
                    gz = 0.1191920,
                    bz = 0.9503041;

    return geometry::vec3<Float>{
      bx * in.b + rx * in.r + gx * in.g,
      by * in.b + ry * in.r + gy * in.g,
      rz * in.r + gz * in.g + bz * in.b,
    };
  }

  template<typename Float>
  constexpr rgb<Float> xyz_d65_to_linear_rgb(const geometry::vec3<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float rx =  3.2404542,
                    ry = -1.5371385,
                    rz = -0.4985314,

                    gx = -0.9692660,
                    gy =  1.8760108,
                    gz =  0.0415560,

                    bx =  0.0556434,
                    by = -0.2040259,
                    bz =  1.0572252;

    return rgb<Float>{
      rz * in.z + rx * in.x + ry * in.y,
      gz * in.z + gx * in.x + gy * in.y,
      bx * in.x + by * in.y + bz * in.z,
    };
  }

  template<typename Float>
  J_INLINE_GETTER constexpr geometry::vec3<Float> srgb_to_xyz_d65(const rgb<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return linear_rgb_to_xyz_d65<Float>(srgb_to_linear_rgb(in));
  }

  template<typename Float>
  J_INLINE_GETTER constexpr rgb<Float> xyz_d65_to_srgb(const geometry::vec3<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return linear_rgb_to_srgb<Float>(xyz_d65_to_linear_rgb(in));
  }
}
