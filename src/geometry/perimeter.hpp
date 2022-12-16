#pragma once

#include "geometry/vec2.hpp"

namespace j::geometry {
  template<typename Num>
  struct perimeter {
    Num left, top, right, bottom;

    perimeter() noexcept = default;

    explicit constexpr perimeter(Num value) noexcept
      : left(value), top(value), right(value), bottom(value)
    {
    }

    constexpr Num & start(axis a) noexcept {
      return a == axis::horizontal ? left : top;
    }
    constexpr Num start(axis a) const noexcept {
      return a == axis::horizontal ? left : top;
    }
    constexpr Num & end(axis a) noexcept {
      return a == axis::horizontal ? right : bottom;
    }
    constexpr Num end(axis a) const noexcept {
      return a == axis::horizontal ? right : bottom;
    }
    constexpr Num total(axis a) const noexcept {
      return a == axis::horizontal ? left + right : top + bottom;
    }
    constexpr Num width() const noexcept {
      return left + right;
    }
    constexpr Num height() const noexcept {
      return top + bottom;
    }

    constexpr vec2<Num> size() const noexcept {
      return vec2<Num>(left + right, top + bottom);
    }

    constexpr perimeter(Num left, Num top, Num right, Num bottom) noexcept
      : left(left), top(top), right(right), bottom(bottom)
    { }

    constexpr perimeter operator+(const perimeter & rhs) const noexcept {
      return perimeter(left + rhs.left, top + rhs.top, right + rhs.right, bottom + rhs.bottom);
    }

    constexpr perimeter operator-(const perimeter & rhs) const noexcept {
      return perimeter(left - rhs.left, top - rhs.top, right - rhs.right, bottom - rhs.bottom);
    }

    constexpr perimeter & operator+=(const perimeter & rhs) const noexcept {
      left += rhs.left, top += rhs.top, right += rhs.right, bottom += rhs.bottom;
      return *this;
    }

    constexpr perimeter & operator-=(const perimeter & rhs) const noexcept {
      left -= rhs.left, top -= rhs.top, right -= rhs.right, bottom -= rhs.bottom;
      return *this;
    }

    constexpr bool operator==(const perimeter & rhs) const noexcept = default;
  };

  using perimeter_u8 = perimeter<u8_t>;
}
