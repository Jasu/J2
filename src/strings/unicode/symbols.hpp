#pragma once

#include "hzd/utility.hpp"

#define J_DEFINE_UNICODE_SYMBOL(NAME, STR, PLAIN) constexpr inline symbol NAME{ STR, PLAIN }

namespace j::strings::inline unicode {
  struct symbol final {
    const char * fancy;
    const char * plain;
  };

  namespace sym {
    J_DEFINE_UNICODE_SYMBOL(arrow_left,   "←", "<-");
    J_DEFINE_UNICODE_SYMBOL(arrow_right,  "→", "->");
    J_DEFINE_UNICODE_SYMBOL(arrow_up,     "↑", "^");
    J_DEFINE_UNICODE_SYMBOL(arrow_down,   "↓", "V");

    J_DEFINE_UNICODE_SYMBOL(empty_set,    "∅", "Empty");

    J_DEFINE_UNICODE_SYMBOL(ellipsis,     "…", "...");
    J_DEFINE_UNICODE_SYMBOL(mid_ellipsis, "⋯", "...");
  }
}
