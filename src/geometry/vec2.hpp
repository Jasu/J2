#pragma once

#include "geometry/axis.hpp"
#include "hzd/concepts.hpp"

namespace j::geometry {
  template<typename Number>
  struct vec2 {
    Number x, y;

    J_ALWAYS_INLINE_NO_DEBUG vec2() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG constexpr vec2(Number x, Number y) noexcept : x(x), y(y) { }


    template<typename N2>
    J_ALWAYS_INLINE constexpr operator vec2<N2>() const noexcept {
      return vec2<N2>(x, y);
    }

    J_INLINE_GETTER constexpr Number & at(axis a) noexcept {
      return (a == axis::horizontal) ? x : y;
    }

    J_INLINE_GETTER constexpr Number at(axis a) const noexcept {
      return (a == axis::horizontal) ? x : y;
    }

    template<typename ONum>
    J_INLINE_GETTER constexpr vec2 operator*(vec2<ONum> rhs) const noexcept {
      return vec2{ (Number)(x * rhs.x), (Number)(y * rhs.y) };
    }

    template<typename ONum>
    J_INLINE_GETTER constexpr vec2 operator/(vec2<ONum> rhs) const {
      return vec2{ (Number)(x / rhs.x), (Number)(y / rhs.y) };
    }

    template<typename ONum>
    J_INLINE_GETTER constexpr vec2 operator+(vec2<ONum> rhs) const noexcept {
      return vec2{ (Number)(x + rhs.x), (Number)(y + rhs.y) };
    }

    template<typename ONum>
    J_INLINE_GETTER constexpr vec2 operator-(vec2<ONum> rhs) const noexcept {
      return vec2{ (Number)(x - rhs.x), (Number)(y - rhs.y) };
    }



    J_INLINE_GETTER constexpr vec2 operator*(Number rhs) const noexcept {
      return vec2{ x * rhs, y * rhs };
    }

    J_INLINE_GETTER vec2 operator/(Number rhs) const {
      return vec2{ x / rhs, y / rhs };
    }



    J_INLINE_GETTER constexpr vec2 operator-() const noexcept {
      return vec2{ -x, -y };
    }



    template<typename ONum>
    J_ALWAYS_INLINE vec2 & operator+=(vec2<ONum> rhs) noexcept {
      return x += rhs.x, y += rhs.y, *this;
    }

    template<typename ONum>
    J_ALWAYS_INLINE constexpr vec2 & operator-=(vec2<ONum> rhs) noexcept {
      return x -= rhs.x, y -= rhs.y, *this;
    }

    template<typename ONum>
    J_ALWAYS_INLINE constexpr vec2 & operator*=(vec2<ONum> rhs) noexcept {
      return x *= rhs.x, y *= rhs.y, *this;
    }

    template<typename ONum>
    J_ALWAYS_INLINE constexpr vec2 & operator/=(vec2<ONum> rhs) {
      return x /= rhs.x, y /= rhs.y, *this;
    }

    J_ALWAYS_INLINE constexpr vec2 & operator*=(Number rhs) noexcept {
      return x *= rhs, y *= rhs, *this;
    }

    J_ALWAYS_INLINE constexpr vec2 & operator/=(Number rhs) {
      return x /= rhs, y /= rhs, *this;
    }

    J_INLINE_GETTER constexpr bool operator==(const vec2 & rhs) const noexcept = default;
  };

  template<typename Number, Arithmetic Multiplier>
  J_INLINE_GETTER constexpr vec2<Number> operator*(Multiplier lhs, vec2<Number> rhs) noexcept {
    return vec2<Number>{ rhs.x * lhs, rhs.y * lhs };
  }

  template<typename Num>
  [[nodiscard]] constexpr vec2<Num> min(vec2<Num> a, vec2<Num> b) noexcept {
    return vec2<Num>(j::min(a.x, b.x), j::min(a.y, b.y));
  }

  template<typename Num>
  [[nodiscard]] constexpr vec2<Num> max(vec2<Num> a, vec2<Num> b) noexcept {
    return vec2<Num>(j::max(a.x, b.x), j::max(a.y, b.y));
  }

  using vec2f   = vec2<float>;

  using vec2i8  = vec2<i8_t>;
  using vec2i16 = vec2<i16_t>;
  using vec2i32 = vec2<i32_t>;

  using vec2u8  = vec2<u8_t>;
  using vec2u16 = vec2<u16_t>;
  using vec2u32 = vec2<u32_t>;
  using vec2u64 = vec2<u64_t>;
}
