#include "math/fixed.hpp"
#include "strings/formatters/floats.hpp"
#include "strings/formatting/formatter.hpp"

namespace j::math {
  namespace s = j::strings;
  namespace f = s::formatters;
  namespace {
    template<typename Fixed>
    class J_TYPE_HIDDEN fixed_formatter final : public strings::formatter_known_length<Fixed> {
    public:
      void do_format(
        const strings::const_string_view & format_options,
        const Fixed & value,
        strings::styled_sink & target,
        strings::style current_style
      ) const override {
        f::float_formatter_v<double>.do_format(format_options, value.as_double(), target, current_style);
      }

      [[nodiscard]] u32_t do_get_length(
        const strings::const_string_view & format_options,
        const Fixed & value
      ) const noexcept override {
        return f::float_formatter_v<double>.do_get_length(format_options, value.as_double());
      }
    };

    J_A(ND, NODESTROY) const fixed_formatter<s16_16> s16_16_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<u16_16> u16_16_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<s24_8> s24_8_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<u24_8> u24_8_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<s8_24> s8_24_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<u8_24> u8_24_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<s32_32> s32_32_formatter;
    J_A(ND, NODESTROY) const fixed_formatter<u32_32> u32_32_formatter;
  }
}
