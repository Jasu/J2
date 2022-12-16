#pragma once

#include "colors/rgb.hpp"
#include "geometry/rect.hpp"
#include "geometry/perimeter.hpp"

namespace j::rendering::renderers::box {
  struct instance final {
    geometry::rect_i16 rectangle_px;
    colors::rgba8 background_color;
    colors::rgba8 border_left_color;
    colors::rgba8 border_right_color;
    colors::rgba8 border_top_color;
    colors::rgba8 border_bottom_color;
    geometry::perimeter_u8 border_size;
    geometry::perimeter_u8 corner_radius;

    constexpr bool operator==(const instance &) const noexcept = default;
  };
}
