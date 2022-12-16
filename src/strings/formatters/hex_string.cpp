#include "strings/formatters/strings.hpp"
#include "strings/formatting/format_digits.hpp"
#include "strings/string.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::strings::formatters {
  namespace {
    template<typename Str>
    class hex_string_formatter final : public formatter_known_length<Str> {
    public:
      hex_string_formatter() noexcept
        : formatter_known_length<Str>("hex", false)
      {
      }
      void do_format(
        const const_string_view & fo,
        const Str & value,
        styled_sink & target,
        style
      ) const override {
        const const_string_view v(value);
        char buf[3] = {' ', 0, 0};
        bool is_first = true;
        for (char c : v) {
          format_hex2((u8_t)c, buf + 1, fo == "U" ? 'A' - 10 : 'a' - 10);
          if (is_first) {
            target.write(buf + 1, 2);
            is_first = false;
          } else {
            target.write(buf, 3);
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view &,
        const Str & value
      ) const noexcept override {
        const const_string_view v(value);
        return v.size() ? v.size() * 3U - 1U : 0U;
      }
    };
    J_A(ND, NODESTROY) const hex_string_formatter<string_view> g_hex_string_view_formatter;
    J_A(ND, NODESTROY) const hex_string_formatter<const_string_view> g_hex_const_string_view_formatter;
    J_A(ND, NODESTROY) const hex_string_formatter<string> g_hex_string_formatter;
    J_A(ND, NODESTROY) const hex_string_formatter<const char *> g_hex_const_char_formatter;
  }
}
