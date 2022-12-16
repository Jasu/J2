#pragma once

#include "input/modifier.hpp"
#include "strings/string_view.hpp"
#include "containers/span.hpp"

namespace j::input {
  struct key_event final {
    u32_t keycode = 0U;
    modifier_mask modifiers;
    bool is_repeat = false;
    span<const u32_t> keysyms;
    strings::const_string_view utf8;
  };
}
