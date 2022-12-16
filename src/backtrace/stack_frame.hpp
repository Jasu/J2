#pragma once

#include "strings/string_view.hpp"

namespace j::backtrace {
  enum class frame_type : u8_t {
    normal,
    signal,
    unknown,
    lisp,
  };

  struct stack_frame final {
    strings::const_string_view func_name;
    u16_t index;
    frame_type type;
    void * ip = nullptr;
  };
}
