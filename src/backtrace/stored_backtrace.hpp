#pragma once

#include "backtrace/stack_frame.hpp"
#include "strings/string.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::backtrace {
  struct stored_stack_frame final {
    strings::string func_name;
    u16_t index;
    frame_type type;
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::backtrace::stored_stack_frame);

namespace j::backtrace {
  using stored_backtrace = trivial_array<stored_stack_frame>;

  stored_backtrace get_backtrace(int skip = 1);
}
