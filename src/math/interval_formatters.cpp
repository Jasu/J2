#include "math/interval.hpp"

#include "strings/formatting/formatter.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::math {
  namespace s = j::strings;
  namespace f = j::strings::formatters;
  namespace {
    template<typename Num>
    class J_TYPE_HIDDEN interval_formatter final : public s::formatter_known_length<interval<Num>> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const interval<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        bool did_set_style = false;
        // if (value.is_unbounded()) {
        //   target.write_styled(s::styles::bright_yellow, "Unbounded", 9U);
        //   return;
        // }
        if (value.is_overflown()) {
          did_set_style = true;
          target.set_style(s::styles::bright_red);
        } else if (!value) {
          did_set_style = true;
          target.set_style(s::styles::red);
        }

        const auto & fo = f::number_formatter_v<Num>;
        target.write_styled(s::styles::bold, "[", 1);
        fo.do_format(format_options, value.left(), target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        fo.do_format(format_options, value.right(), target, current_style);
        target.write_styled(s::styles::bold, ")", 1);
        if (did_set_style) {
          target.set_style(current_style);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const interval<Num> & value
      ) const noexcept override {
        // if (value.is_unbounded()) {
        //   return 9U;
        // }
        const auto & fo = f::number_formatter_v<Num>;
        return 4U
          + fo.do_get_length(format_options, value.left())
          + fo.do_get_length(format_options, value.right());
      }
    };

    J_A(ND, NODESTROY) const interval_formatter<i8_t> g_i8_formatter;
    J_A(ND, NODESTROY) const interval_formatter<i32_t> g_i32_formatter;
    J_A(ND, NODESTROY) const interval_formatter<u8_t> g_u8_formatter;
    J_A(ND, NODESTROY) const interval_formatter<u32_t> g_u32_formatter;
  }
}
