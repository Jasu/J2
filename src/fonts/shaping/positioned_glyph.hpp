#pragma once

#include "math/fixed.hpp"
#include "fonts/rasterizing/glyph_key.hpp"

namespace j::fonts::shaping {
  struct positioned_glyph final {
    float x;
    float y;
    u32_t glyph_index;
  };
}
