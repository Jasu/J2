#pragma once

#include "hzd/utility.hpp"
#include "hzd/concepts.hpp"

namespace j::geometry {
  template<typename Number>
  struct vec3 {
    Number x, y, z;

    J_ALWAYS_INLINE_NO_DEBUG vec3() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG constexpr vec3(Number x, Number y, Number z) noexcept
      : x(x), y(y), z(z)
    { }


    template<typename N2>
    J_ALWAYS_INLINE explicit constexpr operator vec3<N2>() const noexcept
    { return vec3<N2>(x, y, z); }

    J_INLINE_GETTER constexpr vec3 operator*(vec3 rhs) const noexcept
    { return vec3(x * rhs.x, y * rhs.y, z * rhs.z); }

    J_INLINE_GETTER constexpr vec3 operator/(vec3 rhs) const
    { return vec3(x / rhs.x, y / rhs.y, z / rhs.z); }

    J_INLINE_GETTER constexpr vec3 operator+(vec3 rhs) const noexcept
    { return vec3(x + rhs.x, y + rhs.y, z + rhs.z); }

    J_INLINE_GETTER constexpr vec3 operator-(vec3 rhs) const noexcept
    { return vec3(x - rhs.x, y - rhs.y, z - rhs.z); }



    J_INLINE_GETTER constexpr vec3 operator*(Number rhs) const noexcept
    { return vec3(x * rhs, y * rhs, z * rhs); }

    J_INLINE_GETTER constexpr vec3 operator/(Number rhs) const
    { return vec3(x / rhs, y / rhs, z / rhs); }


    J_INLINE_GETTER constexpr vec3 operator-() const noexcept
    { return vec3( -x, -y, -z); }



    J_ALWAYS_INLINE constexpr vec3 & operator+=(vec3 rhs) noexcept {
      return x += rhs.x, y += rhs.y, z += rhs.z, *this;
    }

    J_ALWAYS_INLINE constexpr vec3 & operator-=(vec3 rhs) noexcept {
      return x -= rhs.x, y -= rhs.y, z -= rhs.z, *this;
    }

    J_ALWAYS_INLINE constexpr vec3 & operator*=(vec3 rhs) noexcept {
      return x *= rhs.x, y *= rhs.y, z *= rhs.z, *this;
    }

    J_ALWAYS_INLINE constexpr vec3 & operator/=(vec3 rhs) {
      return x /= rhs.x, y /= rhs.y, z /= rhs.z, *this;
    }

    J_ALWAYS_INLINE constexpr vec3 & operator*=(Number rhs) noexcept {
      return x *= rhs, y *= rhs, z *=rhs, *this;
    }

    J_ALWAYS_INLINE constexpr vec3 & operator/=(Number rhs)
    { return x /= rhs, y /= rhs, z /= rhs, *this; }

    inline constexpr bool operator==(const vec3 & rhs) const noexcept = default;
  };

  template<typename Number, Arithmetic Multiplier>
  J_INLINE_GETTER constexpr vec3<Number> operator*(Multiplier lhs, vec3<Number> rhs) noexcept
  { return vec3<Number>(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs); }

  using vec3f   = vec3<float>;
  using vec3d   = vec3<double>;

  using vec3i8  = vec3<i8_t>;
  using vec3i16 = vec3<i16_t>;
  using vec3i32 = vec3<i32_t>;

  using vec3u8  = vec3<u8_t>;
  using vec3u16 = vec3<u16_t>;
  using vec3u32 = vec3<u32_t>;
  using vec3u64 = vec3<u64_t>;
}
