#pragma once

#include "hzd/types.hpp"

namespace j::strings::inline styling2 {
  inline constexpr u8_t st_style_width = 4;

  inline constexpr u8_t st_bold       = 0b0001;
  inline constexpr u8_t st_italic     = 0b0010;
  inline constexpr u8_t st_underline  = 0b0100;
  inline constexpr u8_t st_strike     = 0b1000;

  inline constexpr u8_t st_no_bold      = st_bold << st_style_width;
  inline constexpr u8_t st_no_italic    = st_italic << st_style_width;
  inline constexpr u8_t st_no_underline = st_underline << st_style_width;
  inline constexpr u8_t st_no_strike    = st_strike << st_style_width;
}
