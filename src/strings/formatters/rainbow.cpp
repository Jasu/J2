#include "strings/formatters/strings.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/string.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "hzd/string.hpp"
#include "hzd/crc32.hpp"

namespace j::strings::formatters {
  namespace {
    const style & rainbow(const_string_view str) noexcept {
      u32_t result = crc32(12U, str.data(), str.size());
      switch (result & 0xFU) {
      case 0U:  return styles::tag1;
      case 1U:  return styles::tag2;
      case 2U:  return styles::tag3;
      case 3U:  return styles::tag4;
      case 4U:  return styles::tag5;
      case 5U:  return styles::tag6;
      case 6U:  return styles::tag7;
      case 7U:  return styles::tag8;
      case 8U:  return styles::tag9;
      case 9U:  return styles::tag10;
      case 10U: return styles::tag11;
      case 11U: return styles::tag12;
      case 12U: return styles::tag13;
      case 13U: return styles::tag14;
      case 14U: return styles::tag15;
      case 15U: return styles::tag16;
      default: J_UNREACHABLE();
      }
    }


    template<typename Str>
    class rainbow_string_formatter final : public formatter_known_length<Str> {
    public:
      rainbow_string_formatter() noexcept
        : formatter_known_length<Str>("rainbow", false)
      {
      }
      void do_format(
        const const_string_view & format_options,
        const Str & value,
        styled_sink & target,
        style current_style
      ) const override {
        const const_string_view v(value);
        if (v) {
          const auto st = rainbow(v);
          target.set_style(st);
          g_const_string_view_formatter.do_format(format_options, v, target, st);
          target.set_style(current_style);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const Str & value
      ) const noexcept override {
        return g_const_string_view_formatter.do_get_length(format_options, value);
      }
    };
    J_A(ND, NODESTROY) const rainbow_string_formatter<string_view> g_rainbow_string_view_formatter;
    J_A(ND, NODESTROY) const rainbow_string_formatter<const_string_view> g_rainbow_const_string_view_formatter;
    J_A(ND, NODESTROY) const rainbow_string_formatter<string> g_rainbow_string_formatter;
    J_A(ND, NODESTROY) const rainbow_string_formatter<const char *> g_rainbow_const_char_formatter;
  }
}
