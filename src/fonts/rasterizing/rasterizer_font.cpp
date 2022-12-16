#include "fonts/rasterizing/rasterizer_font.hpp"

#include "exceptions/assert_lite.hpp"

namespace j::fonts::rasterizing {
  rasterizer_font::rasterizer_font(math::s16_16 pixel_size)
    : pixel_size(pixel_size)
  {
    J_ASSERT(pixel_size > math::s16_16{0}, "Font pixel size was zero.");
  }

  rasterizer_font::~rasterizer_font() {
  }
}
