#pragma once

#include "fonts/font.hpp"
#include "containers/vector.hpp"

J_DECLARE_EXTERN_VECTOR(j::fonts::font);

namespace j::fonts {
  using font_set = vector<font>;
}
