#include "ui/layout/size.hpp"

#include "exceptions/assert_lite.hpp"
#include "strings/format.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::ui::formatters {
  namespace {
    namespace s = j::strings;

    class J_TYPE_HIDDEN span_formatter final : public s::formatter_known_length<span> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const span & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        const auto u16_f = &j::strings::formatters::integer_formatter_v<u16_t>;
        const auto u8_f = &j::strings::formatters::integer_formatter_v<u8_t>;
        target.write_styled(s::styles::bold, "(", 1);
        u8_f->format(format_options, &value.margin_before, target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        u8_f->format(format_options, &value.border_before, target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        u16_f->format(format_options, &value.content_size, target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        u8_f->format(format_options, &value.border_after, target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        u8_f->format(format_options, &value.margin_after, target, current_style);
        target.write_styled(s::styles::bold, ")", 1);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const span & value
      ) const noexcept override {
        const auto u16_f = &j::strings::formatters::integer_formatter_v<u16_t>;
        const auto u8_f = &j::strings::formatters::integer_formatter_v<u8_t>;
        return 10U
          + u8_f->get_length(format_options, &value.border_before)
          + u8_f->get_length(format_options, &value.margin_before)
          + u16_f->get_length(format_options, &value.content_size)
          + u8_f->get_length(format_options, &value.border_after)
          + u8_f->get_length(format_options, &value.margin_after);
      }
    };

    J_A(ND, NODESTROY) const span_formatter g_span_formatter;

    class J_TYPE_HIDDEN size_formatter final : public s::formatter_known_length<size> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const size & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        target.write_styled(s::styles::bold, "(H=", 3);
        g_span_formatter.do_format(format_options, value.horizontal, target, current_style);
        target.write_styled(s::styles::bold, ", V=", 4);
        g_span_formatter.do_format(format_options, value.vertical, target, current_style);
        target.write_styled(s::styles::bold, ")", 1);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const size & value
      ) const noexcept override {
        return 8U
          + g_span_formatter.do_get_length(format_options, value.vertical)
          + g_span_formatter.do_get_length(format_options, value.horizontal);
      }
    };
  }
}
