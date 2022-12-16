#pragma once

#include "geometry/axis.hpp"
#include "util/flags.hpp"

namespace j::geometry {
  enum class side : u8_t {
    top,
    right,
    left,
    bottom,
  };

  using sides = util::flags<side, u8_t>;
  J_FLAG_OPERATORS(side, u8_t)

  inline constexpr sides all_sides{side::top, side::bottom, side::left, side::right};

  J_INLINE_GETTER constexpr side start_of(axis a) noexcept {
    return a == axis::vertical ? side::top : side::left;
  }

  J_INLINE_GETTER constexpr side end_of(axis a) noexcept {
    return a == axis::vertical ? side::bottom : side::right;
  }
}
