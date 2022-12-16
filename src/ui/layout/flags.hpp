#pragma once

#include "util/flags.hpp"
#include "geometry/axis.hpp"

namespace j::ui::inline layout {
  enum class orientation : u8_t {
    columns,
    rows,
  };

  J_INLINE_GETTER constexpr orientation perpendicular(orientation o) noexcept {
    return o == orientation::columns ? orientation::rows : orientation::columns;
  }

  J_INLINE_GETTER constexpr geometry::axis main_axis(orientation o) noexcept {
    return o == orientation::columns ? geometry::axis::horizontal : geometry::axis::vertical;
  }

  J_INLINE_GETTER constexpr geometry::axis cross_axis(orientation o) noexcept {
    return o == orientation::columns ? geometry::axis::vertical : geometry::axis::horizontal;

  }

  enum class gravity : u8_t {
    start,
    center,
    end,

    left = start,
    right = end,
    top = start,
    bottom = end,
  };

  enum layout_flags_t : u8_t {
    space_before,
    space_after,
    space_between,

    vertical_gravity_start,
    vertical_gravity_end,

    horizontal_gravity_start,
    horizontal_gravity_end,

    orientation_columns,

    collapse_margin_top,
    collapse_margin_right,
    collapse_margin_bottom,
    collapse_margin_left,
  };

  using layout_flags = util::flags<layout_flags_t, u16_t>;
  J_FLAG_OPERATORS(layout_flags_t, u16_t)
}
