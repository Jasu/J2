#pragma once

#include "hzd/types.hpp"

namespace j::ansi {
  enum class ascii_code : u8_t {
    bell            = 7U,
    backspace       = 8U,
    tab             = 9U,
    line_feed       = 10U,
    vertical_tab    = 11U,
    form_feed       = 12U,
    carriage_return = 13U,
    escape          = 27U,
    del             = 127U,
  };
}
