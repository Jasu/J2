#include "strings/formatters/indent.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::strings::formatters {
  namespace {
    const char indent[] = "                                                                "
                          "                                                                "
                          "                                                                "
                          "                                                                ";
    static_assert(sizeof(indent) == 257);
    template<typename T>
    class indent_formatter final : public formatter_known_length<T> {
    public:
      indent_formatter() noexcept : formatter_known_length<T>("indent", false)
      { }

      void do_format(
        const const_string_view & format_options,
        const T & value,
        styled_sink & target,
        style
      ) const override {
        if (value > 0) {
          if (format_options.empty()) {
            int v = j::clamp<int>(0, value, 256);
            target.write(indent, v);
          } else {
            for (T i = 0; i < value; ++i) {
              target.write(format_options.data(), format_options.size());
            }
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const T & value
      ) const noexcept override {
        if (format_options.empty()) {
          return j::clamp<int>(0, value, 256);
        } else {
          return j::max(0, value * format_options.size());
        }
      }
    };

    J_A(ND, NODESTROY) const indent_formatter<i8_t> indent_formatter_i8_t;
    J_A(ND, NODESTROY) const indent_formatter<i16_t> indent_formatter_i16_t;
    J_A(ND, NODESTROY) const indent_formatter<i32_t> indent_formatter_i32_t;
    J_A(ND, NODESTROY) const indent_formatter<i64_t> indent_formatter_i64_t;
    J_A(ND, NODESTROY) const indent_formatter<u8_t> indent_formatter_u8_t;
    J_A(ND, NODESTROY) const indent_formatter<u16_t> indent_formatter_u16_t;
    J_A(ND, NODESTROY) const indent_formatter<u32_t> indent_formatter_u32_t;
    J_A(ND, NODESTROY) const indent_formatter<u64_t> indent_formatter_u64_t;
  }
  const formatter_known_length<i8_t> &  g_indent_formatter_i8_t = indent_formatter_i8_t;
  const formatter_known_length<i16_t> & g_indent_formatter_i16_t = indent_formatter_i16_t;
  const formatter_known_length<i32_t> & g_indent_formatter_i32_t = indent_formatter_i32_t;
  const formatter_known_length<i64_t> & g_indent_formatter_i64_t = indent_formatter_i64_t;
  const formatter_known_length<u8_t> &  g_indent_formatter_u8_t = indent_formatter_u8_t;
  const formatter_known_length<u16_t> & g_indent_formatter_u16_t = indent_formatter_u16_t;
  const formatter_known_length<u32_t> & g_indent_formatter_u32_t = indent_formatter_u32_t;
  const formatter_known_length<u64_t> & g_indent_formatter_u64_t = indent_formatter_u64_t;
}
