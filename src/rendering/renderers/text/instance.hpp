#pragma once

#include "geometry/rect.hpp"

namespace j::rendering::renderers::text {
  struct J_PACKED instance {
    geometry::rect_u16 texture_rect;
    geometry::vec2i16 screen_pos;
  };
}
