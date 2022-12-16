#pragma once

#include "util/flags.hpp"

#define _J_WEIGHT_ITEMS                   \
    thin, extra_light, light,  demilight, \
    book, regular,     medium, demibold,  \
    bold, extra_bold,  black,  extra_black

#define _J_SLANT_ITEMS roman, oblique, italic

#define _J_WIDTH_ITEMS                                        \
  ultra_condensed, extra_condensed, condensed, semicondensed, \
  normal, semiexpanded, expanded, extra_expanded,           \
  ultra_expanded
#define _J_SPACING_ITEMS proportional, monospace, dual_width, char_cell

namespace j::fonts::selection {
  enum class font_weight : u8_t {
    _J_WEIGHT_ITEMS
  };

  enum class font_slant : u8_t {
    _J_SLANT_ITEMS
  };

  enum class font_width : u8_t {
    _J_WIDTH_ITEMS
  };

  enum class font_spacing : u8_t {
    _J_SPACING_ITEMS
  };

  using font_weight_flags = util::flags<font_weight, u16_t>;
  J_FLAG_OPERATORS(font_weight, u16_t)

  using font_slant_flags = util::flags<font_slant, u8_t>;
  J_FLAG_OPERATORS(font_slant, u8_t)

  using font_width_flags = util::flags<font_width, u16_t>;
  J_FLAG_OPERATORS(font_width, u16_t)

  using font_spacing_flags = util::flags<font_spacing, u8_t>;
  J_FLAG_OPERATORS(font_spacing, u8_t)
}
