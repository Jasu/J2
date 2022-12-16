#include "mem/debug/region_formatters.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "colors/default_colors.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::mem::debug {
  namespace {
    namespace s = strings;
    namespace c = colors::colors;
    using color = colors::color;
    using rgb8 = colors::rgb8;
    namespace f = s::formatters;
    namespace st = strings::styles;

    const static s::style divider_style(c::white, c::black);

    const static s::style even_style(
      c::white,
      color{rgb8(0x60, 0x60, 0x60), colors::ansi_color::black}
    );

    const static s::style odd_style(
      c::white,
      color{rgb8(0x6A, 0x6A, 0x6A), colors::ansi_color::bright_black}
    );
    const static s::style even2_style(
      c::white,
      color{rgb8(0x42, 0x42, 0x42), colors::ansi_color::black}
    );

    const static s::style odd2_style(
      c::white,
      color{rgb8(0x38, 0x38, 0x38), colors::ansi_color::bright_black}
    );

    template<typename Reg>
    class region_formatter final : public s::formatter_known_length<Reg> {
    public:
      void do_format(
        const s::const_string_view &,
        const Reg & v,
        s::styled_sink & out,
        s::style cs
      ) const override {
        out.write_styled(st::bold, "Memory region at 0x");
        f::integer_formatter_v<u64_t>.do_format("012X", (u64_t)v.begin(), out, cs);
        out.write_styled(st::bold, "-0x");
        f::integer_formatter_v<u64_t>.do_format("012X", (u64_t)v.end(), out, cs);
        out.write_styled(st::bold, " (");
        f::integer_formatter_v<u64_t>.do_format("", v.size(), out, cs);
        out.write_styled(st::bold, " bytes)");
        out.write("\n");
        const char * ptr = (const char *)v.begin();
        for (u32_t i = 0U; i < v.size(); i += 32U) {
          out.write("  ");
          f::integer_formatter_v<u64_t>.do_format("012X", (u64_t)ptr, out, cs);
          out.write(": ");
          for (u32_t j = 0U; j < 32U; ++j) {
            s::style style = (j & 1)
              ? ((j & 4) ? odd_style : odd2_style)
              : ((j & 4) ? even_style : even2_style);
            if (j == 16) {
              out.write_styled(divider_style, " ");
            }
            if (ptr < v.end()) {
              out.set_style(style);
              f::integer_formatter_v<u8_t>.do_format("02X", (u8_t)*ptr++, out, style);
            } else {
              out.write_styled(style, "  ");
            }
          }
          out.set_style(cs);
          out.write("\n");
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const Reg & v
      ) const noexcept override {
        u32_t result = 19U + 12U + 3U + 12U + 2U + f::integer_formatter_v<u64_t>.do_get_length("", v.size())
          + 7U + 1U;
        result += ((v.size() + 31U) / 32U) * (2U + 12U + 2U + 32U + 1U + 32U + 1U);
        return result;
      }
    };

    J_A(ND, NODESTROY) const region_formatter<mem::memory_region> reg_formatter;
    J_A(ND, NODESTROY) const region_formatter<mem::const_memory_region> const_reg_formatter;
  }

  const s::formatter_known_length<memory_region> & g_memory_region_formatter = reg_formatter;
  const s::formatter_known_length<const_memory_region> & g_const_memory_region_formatter = const_reg_formatter;
}
