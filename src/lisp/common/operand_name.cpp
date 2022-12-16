#include "operand_name.hpp"
#include "strings/format.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/integers.hpp"
#include "lisp/env/debug_formatters.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::lisp::inline common {
  namespace s = strings;

  strings::string operand_name::format() const noexcept {
    if (!name && !id) {
      return "";
    }
    strings::const_string_view format_str = "{}[{}]";
    strings::string index_fmt = index ? strings::format("{}", index) : "0";
    switch (name_format) {
    case operand_name_format::index_offset:
      if (!index) {
        return name ? strings::string{name} : strings::format("{}", id);
      }
      format_str = "{}+{}";
      [[fallthrough]];
    case operand_name_format::index_bracket:
      return name ? strings::format(format_str, name, index_fmt) : strings::format(format_str, id, index_fmt);
    }
  }

  void operand_name::format(strings::styling::styled_sink & to, strings::styling::style cs) const noexcept {
    if (!name && !id) {
      return;
    }

    switch (name_format) {
    case operand_name_format::index_offset:
    case operand_name_format::index_bracket:
      if (name) {
        to.write_styled(s::styles::bright_cyan, name);
      } else if (id) {
        env::g_id_formatter.do_format("", id, to, cs);
      }
      if (name_format == operand_name_format::index_offset) {
        if (index > 0) {
        to.write_styled(s::styles::bright_green, "+");
        } else if (!index) {
          return;
        }
      } else {
        to.write_styled(s::styles::light_gray, "[");
      }
      s::formatters::integer_formatter_v<i32_t>.do_format("", (i32_t)index, to, cs);
      if (name_format == operand_name_format::index_bracket) {
        to.write_styled(s::styles::light_gray, "]");
      }
    }
  }

  u32_t operand_name::get_length() const noexcept {
    if (!name && !id) {
      return 0U;
    }

    u32_t result = 0U;
    switch (name_format) {
    case operand_name_format::index_offset:
    case operand_name_format::index_bracket:
      if (name) {
        result = name.size();
      } else if (id) {
        result = env::g_id_formatter.do_get_length("", id);
      }
      if (name_format == operand_name_format::index_offset) {
        if (index > 0) {
          ++result;
        } else if (!index) {
          return result;
        }
      } else {
        ++result;
      }
      result += s::formatters::integer_formatter_v<i32_t>.do_get_length("", (i32_t)index);
      if (name_format == operand_name_format::index_bracket) {
        ++result;
      }
      break;
    }
    return result;
  }
}
