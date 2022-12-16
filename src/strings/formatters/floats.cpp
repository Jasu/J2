#include "strings/formatters/floats.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi-stmt"
#include <double-to-string.h>
#pragma clang diagnostic pop

namespace j::strings::formatters {
  namespace {
    namespace d = double_conversion;
    const d::DoubleToStringConverter g_converter{
      d::DoubleToStringConverter::NO_FLAGS,
      "Inf",
      "NaN",
      'e',
      -7,
      22,
      6,
      6
    };

    class double_formatter final : public formatter_known_length<double> {
    public:
      void do_format(
        const const_string_view &,
        const double & value,
        styled_sink & target,
        style current_style
      ) const override {
        if (current_style.foreground.empty() && current_style.background.empty()) {
          style num_style = styling::styles::bright_green;
          if (!value) {
            num_style = styling::styles::bright_cyan;
          } else if (value < 0) {
            num_style = styling::styles::bright_red;
          }
          target.set_style(override_style(current_style, num_style));
        }
        char scratch[30];
        d::StringBuilder b(scratch, 30);
        g_converter.ToShortest(value, &b);
        target.write(scratch, b.position());
        if (current_style.foreground.empty() && current_style.background.empty()) {
          target.set_style(current_style);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view &,
        const double & value) const noexcept override {
        char scratch[30];
        d::StringBuilder b(scratch, 30);
        g_converter.ToShortest(value, &b);
        return b.position();
      }
    };

    class float_formatter final : public formatter_known_length<float> {
    public:
      void do_format(
        const const_string_view &,
        const float & value,
        styled_sink & target,
        style
      ) const override {
        // Suppress destructor of the string builder, to prevent it from null-terminating the string.
        // u64_t d[(sizeof(d::StringBuilder) + sizeof(u64_t) - 1) / sizeof(u64_t)];
        // auto b = new (&d[0]) d::StringBuilder(target, 30);
        // g_converter.ToShortestSingle(value, b);
        // return b->position();
        char scratch[30];
        d::StringBuilder b(scratch, 30);
        g_converter.ToShortestSingle(value, &b);
        target.write(scratch, b.position());
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view &,
        const float & value
      ) const noexcept override {
        char scratch[30];
        d::StringBuilder b(scratch, 30);
        g_converter.ToShortestSingle(value, &b);
        return b.position();
      }
    };

    J_A(ND, NODESTROY) const float_formatter float_formatter;
    J_A(ND, NODESTROY) const double_formatter double_formatter;
  }
  template<> const formatter_known_length<float> & float_formatter_v<float> = float_formatter;
  template<> const formatter_known_length<double> & float_formatter_v<double> = double_formatter;
}
