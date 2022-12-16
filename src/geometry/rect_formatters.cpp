#include "geometry/rect.hpp"
#include "strings/formatting/formatter.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::geometry {
  namespace {
    namespace s = ::j::strings;
    namespace f = s::formatters;
    template<typename Num>
    class J_TYPE_HIDDEN rect_formatter final : public s::formatter_known_length<rect<Num>> {
    public:
      void do_format(
        const s::const_string_view & format_options,
        const rect<Num> & value,
        s::styled_sink & target,
        s::style current_style
      ) const override {
        const auto & fo = f::number_formatter_v<Num>;
        target.write_styled(s::styles::bold, "[(", 2);
        fo.do_format(format_options, value.left(), target, current_style);
        target.write_styled(s::styles::bold, ", ", 2);
        fo.do_format(format_options, value.top(), target, current_style);
        target.write_styled(s::styles::bold, "), (", 4);
        fo.do_format(format_options, value.width(), target, current_style);
        target.write_styled(s::styles::bold, "x", 1);
        fo.do_format(format_options, value.height(), target, current_style);
        target.write_styled(s::styles::bold, ")]", 2);
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view & format_options,
        const rect<Num> & value
      ) const noexcept override {
        const auto & fo = f::number_formatter_v<Num>;
        return 2U // "[("
          + fo.do_get_length(format_options, value.left())
          + 2U // ", "
          + fo.do_get_length(format_options, value.top())
          + 4U // "), ("
          + fo.do_get_length(format_options, value.width())
          + 1U // "x"
          + fo.do_get_length(format_options, value.height())
          + 2U; // ")]"
      }
    };

    J_A(ND, NODESTROY) const rect_formatter<float> rect_float_formatter;
    J_A(ND, NODESTROY) const rect_formatter<u16_t> rect_u16_formatter;
    J_A(ND, NODESTROY) const rect_formatter<u32_t> rect_u32_formatter;
    J_A(ND, NODESTROY) const rect_formatter<i16_t> rect_i16_formatter;
    J_A(ND, NODESTROY) const rect_formatter<i32_t> rect_i32_formatter;
  }
}
