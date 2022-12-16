#pragma once

#include "backtrace/stack_frame.hpp"

namespace j::backtrace {
  using stack_trace_fn_t = void (*)(stack_frame frame, void * userdata);
  void backtrace(stack_trace_fn_t callback, void * userdata, int skip = 0);
}
