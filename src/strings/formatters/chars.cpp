#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/unicode/utf8.hpp"

namespace j::strings::formatters {
  namespace {
    struct J_AT(HIDDEN) char_formatter final : formatter_known_length<char> {
      void do_format(const const_string_view &, const char & value, styled_sink & target, style) const override {
        target.write((const char*)&value, 1);
      }

      [[nodiscard]] u32_t do_get_length(const const_string_view &, const char &) const noexcept override {
        return 1;
      }
    };

    struct J_AT(HIDDEN) code_point_formatter final : formatter_known_length<u32_t, i32_t, u64_t, i64_t> {
      J_A(AI,ND) inline code_point_formatter() noexcept : formatter_known_length("codepoint", false) { }
      void do_format(const const_string_view &, const u32_t & cp, styled_sink & target, style) const override {
        char utf[4] = {0, 0, 0, 0};
        u32_t sz = utf8_code_point_bytes(cp);
        utf8_encode(utf, cp);
        target.write(utf, sz);
      }

      [[nodiscard]] u32_t do_get_length(const const_string_view &, const u32_t & cp) const noexcept override {
        return utf8_code_point_bytes(cp);
      }
    };

    J_A(ND,NODESTROY) const char_formatter g_char_formatter;
    J_A(ND,NODESTROY) const code_point_formatter g_code_point_formatter;
  }
}
