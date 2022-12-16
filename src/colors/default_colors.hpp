#pragma once

#include "colors/color.hpp"

namespace j::colors::colors {
  inline constexpr color black         { rgb8(0U,   0U,   0U),   ansi_color::black};
  inline constexpr color red           { rgb8(170U, 0U,   0U),   ansi_color::red};
  inline constexpr color green         { rgb8(0U,   170U, 0U),   ansi_color::green};
  inline constexpr color yellow        { rgb8(160U, 160U, 0U),   ansi_color::yellow};
  inline constexpr color blue          { rgb8(40U,   50U,   255U), ansi_color::blue};
  inline constexpr color magenta       { rgb8(180U, 0U,   180U), ansi_color::magenta};
  inline constexpr color cyan          { rgb8(0U,   190U, 190U), ansi_color::cyan};
  inline constexpr color light_gray    { rgb8(192U, 192U, 192U), ansi_color::white};

  inline constexpr color gray          { rgb8(90U,  90U,  90U),  ansi_color::bright_black};
  inline constexpr color bright_red    { rgb8(255U, 0U,   0U),   ansi_color::bright_red};
  inline constexpr color bright_green  { rgb8(0U,   255U, 0U),   ansi_color::bright_green};
  inline constexpr color bright_yellow { rgb8(255U, 255U, 0U),   ansi_color::bright_yellow};
  inline constexpr color bright_blue   { rgb8(90U,   120U,   255U), ansi_color::bright_blue};
  inline constexpr color bright_magenta{ rgb8(255U, 0U,   255U), ansi_color::bright_magenta};
  inline constexpr color bright_cyan   { rgb8(0U,   255U, 255U), ansi_color::bright_cyan};
  inline constexpr color white         { rgb8(255U, 255U, 255U), ansi_color::bright_white};

  inline constexpr color orange { rgb8(224U, 150U, 20U), ansi_color::yellow};
  inline constexpr color bright_orange { rgb8(255U, 200U, 80U), ansi_color::bright_yellow};
}
