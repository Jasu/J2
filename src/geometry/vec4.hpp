#pragma once

#include "geometry/vec2.hpp"

namespace j::geometry {
  template<typename Number>
  struct vec4 {
    Number x, y, z, w;

    J_ALWAYS_INLINE_NO_DEBUG vec4() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG constexpr vec4(Number x, Number y, Number z, Number w) noexcept
      : x(x), y(y), z(z), w(w) { }

    J_ALWAYS_INLINE_NO_DEBUG constexpr vec4(vec2<Number> xy, Number z, Number w) noexcept
      : x(xy.x), y(xy.y), z(z), w(w) { }

    J_ALWAYS_INLINE_NO_DEBUG constexpr vec4(vec2<Number> xy, vec2<Number> zw) noexcept
      : x(xy.x), y(xy.y), z(zw.x), w(zw.y) { }


    template<typename N2>
    J_INLINE_GETTER constexpr operator vec4<N2>() const noexcept {
      return vec4<N2>(x, y, z, w);
    }


    J_INLINE_GETTER constexpr vec4 operator*(const vec4 & rhs) const noexcept {
      return vec4{ x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w };
    }

    J_INLINE_GETTER constexpr vec4 operator/(const vec4 & rhs) const {
      return vec4{ x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w };
    }

    J_INLINE_GETTER constexpr vec4 operator+(const vec4 & rhs) const noexcept {
      return vec4{ Number(x + rhs.x), Number(y + rhs.y), Number(z + rhs.z), Number(w + rhs.w) };
    }

    J_INLINE_GETTER constexpr vec4 operator-(const vec4 & rhs) const noexcept {
      return vec4{ x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w };
    }



    J_INLINE_GETTER constexpr vec4 operator*(Number rhs) const noexcept {
      return vec4{ x * rhs, y * rhs, z * rhs, w * rhs };
    }

    J_INLINE_GETTER constexpr vec4 operator/(Number rhs) const {
      return vec4{ x / rhs, y / rhs, z / rhs, w / rhs };
    }



    J_INLINE_GETTER constexpr vec4 operator-() const noexcept {
      return vec4{ -x, -y, -z, -w };
    }



    J_ALWAYS_INLINE constexpr vec4 & operator+=(const vec4 & rhs) noexcept {
      return x += rhs.x, y += rhs.y, z += rhs.z, w += rhs.w, *this;
    }

    J_ALWAYS_INLINE constexpr vec4 & operator-=(const vec4 & rhs) noexcept {
      return x -= rhs.x, y -= rhs.y, z -= rhs.z, w -= rhs.w, *this;
    }

    J_ALWAYS_INLINE constexpr vec4 & operator*=(const vec4 & rhs) noexcept {
      return x *= rhs.x, y *= rhs.y, z *= rhs.z, w *= rhs.w, *this;
    }

    J_ALWAYS_INLINE constexpr vec4 & operator/=(const vec4 & rhs) {
      return x /= rhs.x, y /= rhs.y, z /= rhs.z, w /= rhs.z, *this;
    }

    J_ALWAYS_INLINE constexpr vec4 & operator*=(Number rhs) noexcept {
      return x *= rhs, y *= rhs, z *=rhs, w *= rhs, *this;
    }

    J_ALWAYS_INLINE constexpr vec4 & operator/=(Number rhs) {
      return x /= rhs, y /= rhs, z /= rhs, w /= rhs, *this;
    }

    J_INLINE_GETTER constexpr bool operator==(const vec4 & rhs) const noexcept = default;
  };

  template<typename Number, Arithmetic Multiplier>
  J_INLINE_GETTER constexpr vec4<Number> operator*(Multiplier lhs, const vec4<Number> & rhs) noexcept {
    return vec4<Number>{ (Number)(rhs.x * lhs), (Number)(rhs.y * lhs), (Number)(rhs.z * lhs), (Number)(rhs.w * lhs) };
  }

  using vec4f   = vec4<float>;
  using vec4d   = vec4<double>;

  using vec4i8  = vec4<i8_t>;
  using vec4i16 = vec4<i16_t>;
  using vec4i32 = vec4<i32_t>;

  using vec4u8  = vec4<u8_t>;
  using vec4u16 = vec4<u16_t>;
  using vec4u32 = vec4<u32_t>;
  using vec4u64 = vec4<u64_t>;
}
