#include "geometry/perimeter.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::geometry {
  namespace {
    namespace s = ::j::strings;
    namespace f = s::formatters;
    template<typename Num>
    class J_TYPE_HIDDEN perimeter_formatter final : public s::formatter_known_length<perimeter<Num>> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const perimeter<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        const auto & fo = f::number_formatter_v<Num>;
        target.write_styled(s::styles::bold, "(L=", 3);
        fo.do_format(format_options, value.left, target, current_style);
        target.write_styled(s::styles::bold, ", T=", 4);
        fo.do_format(format_options, value.top, target, current_style);
        target.write_styled(s::styles::bold, ", R=", 4);
        fo.do_format(format_options, value.right, target, current_style);
        target.write_styled(s::styles::bold, ", B=", 4);
        fo.do_format(format_options, value.bottom, target, current_style);
        target.write_styled(s::styles::bold, ")", 1);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const perimeter<Num> & value
      ) const noexcept override {
        const auto & fo = f::number_formatter_v<Num>;
        return 3U // "(L="
          + fo.do_get_length(format_options, value.left)
          + 4U // ", T="
          + fo.do_get_length(format_options, value.top)
          + 4U // ", R="
          + fo.do_get_length(format_options, value.right)
          + 4U // ", B="
          + fo.do_get_length(format_options, value.bottom)
          + 1U; // ")";
      }
    };

    J_A(ND, NODESTROY) const perimeter_formatter<u8_t> perimeter_u8_formatter;
  }
}
