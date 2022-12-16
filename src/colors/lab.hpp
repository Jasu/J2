#pragma once

#include "colors/xyz.hpp"
#include "hzd/math.hpp"

namespace j::colors {
  using labf = geometry::vec3<float>;

  namespace detail {
    template<typename Float>
    J_INLINE_GETTER constexpr Float xyz_d65_to_lab_helper(Float in) noexcept {
#     pragma clang fp contract(fast)
      constexpr Float inv_three_delta_squared = 7.78703703703703703758,
                      delta_cubed             = 0.00885645167903563082,
                      offset                  = 0.13793103448275862069;
      return in > delta_cubed ? ::j::cbrt(in) : inv_three_delta_squared * in + offset;
    }

    template<typename Float>
    J_INLINE_GETTER constexpr Float lab_to_xyz_d65_helper(Float in) noexcept {
#     pragma clang fp contract(fast)
      constexpr Float delta               = 0.20689655172413793103,
                      three_delta_squared = 0.12841854934601664684,
                      offset              = -0.13793103448275862069;
      return in > delta ? in * in * in : three_delta_squared * (in + offset);
    }
  }

  template<typename Float>
  constexpr geometry::vec3<Float> xyz_d65_to_lab(const geometry::vec3<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float x_normalization_d65 = 1.05209002944799992425,
                    z_normalization_d65 = 0.91840858160978656185,
                    l_multiplier        = 116.0,
                    l_offset            = -16.0,
                    a_multiplier        = 500.0,
                    b_multiplier        = 200.0;
    const Float x = detail::xyz_d65_to_lab_helper(x_normalization_d65 * in.x),
                y = detail::xyz_d65_to_lab_helper(in.y),
                z = detail::xyz_d65_to_lab_helper(z_normalization_d65 * in.z);
    return geometry::vec3<Float>{
      l_multiplier * y + l_offset,
      a_multiplier * (x - y),
      b_multiplier * (y - z)
    };
  }

  template<typename Float>
  constexpr geometry::vec3<Float> lab_to_xyz_d65(const geometry::vec3<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    constexpr Float x_normalization_d65 = 0.950489,
                    z_normalization_d65 = 1.088840,
                    l_multiplier        = 0.00862068965517241379,
                    l_offset            = 16.0,
                    a_multiplier        = 0.002,
                    b_multiplier        = 0.005;
    const Float l = (in.x + l_offset) * l_multiplier,
                a = in.y * a_multiplier,
                b = in.z * b_multiplier;
    return geometry::vec3<float>{
      x_normalization_d65 * detail::lab_to_xyz_d65_helper(l + a),
      detail::lab_to_xyz_d65_helper(l),
      z_normalization_d65 * detail::lab_to_xyz_d65_helper(l - b)
    };
  }

  template<typename Float>
  J_INLINE_GETTER constexpr geometry::vec3<Float> srgb_to_lab(const rgb<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return xyz_d65_to_lab(srgb_to_xyz_d65(in));
  }

  template<typename Float>
  J_INLINE_GETTER constexpr rgb<Float> lab_to_srgb(const geometry::vec3<Float> & in) noexcept {
#   pragma clang fp contract(fast)
    return xyz_d65_to_srgb(lab_to_xyz_d65(in));
  }
}
