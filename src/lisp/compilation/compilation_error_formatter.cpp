#include "lisp/compilation/compilation_error_formatter.hpp"

#include "strings/styling/default_styles.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/formatting/sink_formatter.hpp"
#include "strings/styling/styled_sink.hpp"
#include "lisp/sources/debug_formatters.hpp"
#include "lisp/env/debug_formatters.hpp"

namespace j::lisp::compilation {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    constexpr s::const_string_view error_messages[] = {
      [(u8_t)compilation_error_type::none]                     = "Unknown error in {}.",
      [(u8_t)compilation_error_type::symbol_not_found]         = "Symbol {} not found at {}.",
      [(u8_t)compilation_error_type::variable_not_found]       = "Variable {} not found at {}.",
      [(u8_t)compilation_error_type::function_not_found]       = "Function {} not found at {}.",
      [(u8_t)compilation_error_type::variable_recursive]       = "Variable definition {} at {} depends on its own value.",
      [(u8_t)compilation_error_type::function_recursive]       = "Function definition {} at {} calls it self through macro expansion.",
      [(u8_t)compilation_error_type::not_function]             = "Symbol {} is not a function at {}.",
      [(u8_t)compilation_error_type::not_writable]             = "Symbol {} is not writable at {}.",

      [(u8_t)compilation_error_type::arg_count_mismatch]       = "{} expects exactly {} arguments, got {} at {}.",
      [(u8_t)compilation_error_type::too_few_args]             = "{} expects at least {} arguments, got only {} at {}.",
      [(u8_t)compilation_error_type::too_many_args]            = "{} expects at most {} arguments, got {} at {}.",

      [(u8_t)compilation_error_type::arg_count_mismatch_range] = "{} expects between {} and {} arguments, got {} at {}.",
    };

    [[nodiscard]] u32_t get_arg_size(id id) {
      return env::g_id_formatter.do_get_length("", id);
    }

    template<Integral T>
    [[nodiscard]] u32_t get_arg_size(T num) {
      return f::integer_formatter_v<T>.do_get_length("", num);
    }

    struct J_TYPE_HIDDEN size_visitor final {
      template<typename... Args>
      [[nodiscard]] u32_t operator()(compilation_error_type type, const sources::source_location & location, Args && ... args) {
        return error_messages[(u8_t)type].size() - 2U * sizeof...(args) - 2U
          + sources::g_source_location_formatter.do_get_length("", location)
          + (0U + ... + get_arg_size(static_cast<Args &&>(args)));
      }
    };

    struct J_TYPE_HIDDEN format_visitor final {
      s::styled_sink & to;
      s::style cs;
      s::sink_formatter sink_fmt = {};

      template<typename... Args>
        void operator()(compilation_error_type type, const sources::source_location & location, Args && ... args) {
        sink_fmt.write(to, cs, error_messages[(u8_t)type],
                       static_cast<Args &&>(args)...,
                       location);
      }
    };

    struct compilation_error_formatter final : s::formatter_known_length<compilation_error> {
      void do_format(
        const s::const_string_view &,
        const compilation_error & e,
        s::styled_sink & to,
        s::style cs
      ) const override {
        format_visitor v{to, cs};
        e.visit(v);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const compilation_error & e
      ) const noexcept override {
        size_visitor v;
        return e.visit(v);
      }
    };

    J_A(ND, NODESTROY) const compilation_error_formatter error_fmt;
  }

  const strings::formatter_known_length<compilation_error> & g_compilation_error_formatter = error_fmt;
}
