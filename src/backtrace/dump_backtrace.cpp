#include "backtrace/dump_backtrace.hpp"

#include "exceptions/assert_lite.hpp"
#include "hzd/demangle.hpp"
#include "services/services.hpp"
#include "strings/formatting/formatted_sink.hpp"
#include "backtrace/backtrace.hpp"

namespace j::backtrace {
  namespace {
    void dump_backtrace_callback(stack_frame frame, void * userdata) {
      J_ASSERT_NOT_NULL(userdata);
      strings::formatted_sink * sink = reinterpret_cast<strings::formatted_sink *>(userdata);
      const char * type = "        ";
      if (frame.type == frame_type::signal) {
        type = "Signal ";
      } else if (frame.type == frame_type::unknown) {
        type = "Unknown ";
      } else if (frame.type == frame_type::lisp) {
        type = "Lisp    ";
      }
      strings::const_string_view name = frame.func_name;
      const char * demangled = nullptr;
      if (!name) {
        name = "Unknown function";
      } else if (frame.type != frame_type::lisp) {
        try {
          demangled = demangle(name.data());
          if (demangled) {
            name = demangled;
          }
        } catch (...) {
          demangled = nullptr;
        }
      }
      sink->write("{#bright_red,bold}{}{/}{#bright_green,bold}{:2}.{/} {#bright_yellow,bold}{}{/} {#light_gray}{:0X12}{/}\n",
                  type, frame.index, name, frame.ip);

      if (demangled) {
        demangle_free(demangled);
      }
    }
  }

  void dump_backtrace(strings::formatted_sink & sink, int skip) {
    backtrace(dump_backtrace_callback, &sink, skip + 1);
  }

  void dump_backtrace(int skip) {
    auto sink = services::service_container->get<strings::formatted_sink>(services::service_reference("stderr"));
    J_ASSERT_NOT_NULL(sink);
    backtrace(dump_backtrace_callback, sink.get(), skip + 1);
  }
}
