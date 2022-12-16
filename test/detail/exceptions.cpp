#include <detail/preamble.hpp>

#include "containers/vector.hpp"
#include "exceptions/backtrace.hpp"
#include "exceptions/exceptions.hpp"
#include "hzd/demangle.hpp"
#include "strings/format.hpp"
#include "strings/formatting/context.hpp"

namespace e = j::exceptions;
namespace b = j::backtrace;
namespace s = j::strings;

REGISTER_EXCEPTION_TRANSLATOR(e::exception & e) {
  s::string backtrace;
  if (auto bt = e.maybe_get_tag_value(e::backtrace)) {
    backtrace = "\nBacktrace:\n";
    for (auto & frame : *bt) {
      s::string fn("<Unknown function>");
      if (frame.func_name) {
        try {
          s::string func_name{frame.func_name + s::string(1, '\0')};
          const char * demangled = j::demangle(func_name.data());
          if (demangled) {
            fn = demangled;
            j::demangle_free(demangled);
          } else {
            fn = frame.func_name;
          }
        } catch (...) {
          fn = frame.func_name;
        }
      }
      backtrace += s::format("  {:2} {}{}\n",
                             frame.index,
                             frame.type == b::frame_type::signal
                              ? "Signal "
                              : (frame.type == b::frame_type::unknown ? "Unknown " : ""),
                             fn);
    }
  }
  if (auto msg = e.maybe_get_tag_value(e::message)) {
    auto param = e.maybe_get_tag(e::value);
    if (param) {
      j::vector<j::strings::formatting::format_value> values;
      while (param != e.end()) {
        if (param->is(e::value)) {
          values.emplace_back(param->value().as_void_star(), param->value().type());
        }
        ++param;
      }
      auto g = s::formatting_context.enter(s::attributes::context_type = s::formatting::context_type::visual);
      return toString(s::detail::format(s::const_string_view(*msg),
                                        values.size(),
                                        values.begin())
                      + backtrace);
    } else {
      return toString(*msg + backtrace);
    }
  } else if (auto desc = e.maybe_get_tag_value(e::description)) {
    return toString(*desc + backtrace);
  }
  return "Unknown exception";
}
