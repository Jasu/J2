#pragma once

#include "colors/xyz.hpp"
#include "hzd/math.hpp"

namespace j::colors {
  template<typename Float>
  constexpr geometry::vec3<Float> lms_to_xyz_d65_von_kries(const geometry::vec3<Float> & in) noexcept {
    return geometry::vec3<Float>{
      in.x * 1.8599364 + in.y * -1.1293816 + in.z * 0.2198974,
      in.x * 0.3611914 + in.y * 0.6388125 + in.z * -0.0000064,
      in.z * 1.0890636,
    };
  }

  template<typename Float>
  constexpr geometry::vec3<Float> xyz_d65_to_lms_von_kries(const geometry::vec3<Float> & in) noexcept {
    return geometry::vec3<Float>{
      in.x * 0.40024 + in.y * 0.70760 + in.z * -0.08081,
      in.x * -0.22630 + in.y * 1.16532 + in.z * 0.04570,
      in.z * 0.91822,
    };
  }

  template<typename Float>
  constexpr geometry::vec3<Float> lms_to_xyz_d65_bradford(const geometry::vec3<Float> & in) noexcept {
    return geometry::vec3<Float>{
      in.x * 0.9869929 + in.y * -0.1470543 + in.z * 0.1599627,
      in.x * 0.4323053 + in.y * 0.5183603 + in.z * 0.0492912,
      in.x * -0.0085287 + in.y * 0.0400428 + in.z * 0.9684867,
    };
  }

  template<typename Float>
  constexpr geometry::vec3<Float> xyz_d65_to_lms_bradford(const geometry::vec3<Float> & in) noexcept {
    return geometry::vec3<Float>{
      in.x * 0.8951+ in.y * 0.2664 + in.z * -0.1614,
      in.x * -0.7502 + in.y * 1.7135 + in.z * 0.0367,
      in.x * 0.0389 + in.y * 0.0685 + in.z * 1.0296,
    };
  }
}
