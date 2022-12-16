#include "backtrace/backtrace.hpp"
#include "exceptions/assert_lite.hpp"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace j::backtrace {
  void backtrace(stack_trace_fn_t callback, void * userdata, int skip) {
    J_ASSERT_NOT_NULL(callback);
    unw_context_t context;
    unw_cursor_t cursor;
    unw_proc_info_t proc;
    char name_buffer[512];
    J_REQUIRE(unw_getcontext(&context) == 0, "unw_getcontext failed.");
    J_REQUIRE(unw_init_local(&cursor, &context) == 0, "unw_init_local failed.");
    u16_t index = 0;
    while (unw_step(&cursor) > 0) {
      if (skip) {
        --skip;
        continue;
      }
      unw_get_proc_info(&cursor, &proc);
      frame_type type = frame_type::unknown;
      auto t = unw_is_signal_frame(&cursor);
      if (t > 0) {
        type = frame_type::signal;
      } else if (t == 0) {
        type = frame_type::normal;
        if (proc.handler == 0 && proc.format == UNW_INFO_FORMAT_DYNAMIC) {
          type = frame_type::lisp;
        }
      }
      unw_word_t ip;
      unw_get_reg(&cursor, UNW_REG_IP, &ip);
      unw_word_t offset;
      auto result = unw_get_proc_name(&cursor, name_buffer, 512, &offset);
      strings::const_string_view name{};
      switch (result) {
      case 0: name = name_buffer; break;
      case UNW_ENOINFO: case UNW_EUNSPEC: case UNW_ENOMEM: default: break;
      }
      callback(stack_frame{name, index, type, (void*)ip}, userdata);
      if (name == "main") {
        return;
      }
      ++index;
    }
  }
}
