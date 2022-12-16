#include "backtrace/stored_backtrace.hpp"

#include "strings/string.hpp"
#include "containers/trivial_array.hpp"
#include "exceptions/assert_lite.hpp"

#define UNW_LOCAL_ONLY
#include <libunwind.h>

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::backtrace::stored_stack_frame);

namespace j::backtrace {
  struct J_TYPE_HIDDEN stored_stack_frame_local final {
    stored_stack_frame_local * prev;
    stored_stack_frame frame;
  };

  stored_backtrace get_backtrace(int skip) {
    unw_context_t context;
    unw_cursor_t cursor;
    char name_buffer[512];
    J_REQUIRE(unw_getcontext(&context) == 0, "unw_getcontext failed.");
    J_REQUIRE(unw_init_local(&cursor, &context) == 0, "unw_init_local failed.");
    u16_t index = 0;
    stored_stack_frame_local * bottom = nullptr;
    while (unw_step(&cursor) > 0) {
      if (skip) { --skip; continue; }

      bottom = J_ALLOCA_NEW(stored_stack_frame_local){bottom, { .index = index++ }};

      {
        auto t = unw_is_signal_frame(&cursor);
        if (J_LIKELY(t == 0))       { bottom->frame.type = frame_type::normal; }
        else if (J_UNLIKELY(t < 0)) { bottom->frame.type = frame_type::unknown; }
        else                        { bottom->frame.type = frame_type::signal; }
      }

      unw_word_t offset;
      if (J_LIKELY(unw_get_proc_name(&cursor, name_buffer, 512, &offset) == 0)) {
        bottom->frame.func_name = (const char *)name_buffer;
        if (!j::strcmp(name_buffer, "main")) {
          break;
        }
      }
    }

    stored_backtrace result(index);
    while (index--) {
      result[index] = static_cast<stored_stack_frame &&>(bottom->frame);
      bottom = bottom->prev;
    }
    return result;
  }
}
