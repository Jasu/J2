#pragma once

#include "hzd/types.hpp"

namespace j::colors {
  enum class ansi_color : u8_t {
    black = 0U,
    red,
    green,
    yellow,
    blue,
    magenta,
    cyan,
    white,

    bright_start,
    bright_black = bright_start,
    bright_red,
    bright_green,
    bright_yellow,
    bright_blue,
    bright_magenta,
    bright_cyan,
    bright_white,
  };
}
