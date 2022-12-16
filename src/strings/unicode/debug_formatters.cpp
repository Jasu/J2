#include "strings/formatting/formatter.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/formatters/integers.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/unicode/line_breaks_fwd.hpp"
#include "strings/unicode/graphics.hpp"

namespace j::strings::inline unicode {
  namespace {
    namespace f = formatters;
    J_A(ND, NODESTROY) const f::enum_formatter<box_type> box_type_formatter{{
        {box_type::none,                 "None"},
        {box_type::light,                "Light"},
        {box_type::light_round,          "Light round"},
        {box_type::dashed,               "Dashed"},
        {box_type::light_triple_dash,    "Light triple-dash"},
        {box_type::light_quadruple_dash, "Light quadruple-dash"},
        {box_type::double_light,         "Double light"},
        {box_type::heavy,                "Heavy"},
        {box_type::heavy_dashed,         "Heavy dashed"},
        {box_type::heavy_triple_dash,    "Heavy triple-dash"},
        {box_type::heavy_quadruple_dash, "Heavy quadruple-dash"},
      }};

    J_A(ND, NODESTROY) const f::enum_formatter<lbr_state> lbr_state_formatter{
      f::visual = span<const f::enum_value>({
        { lbr_state::none, "None" },
        { lbr_state::bk, "Hard break" },
        { lbr_state::cr, "CR" },
        { lbr_state::lf, "LF" },
        { lbr_state::zw, "Zero-width-space" },
        { lbr_state::zwj, "ZWJ" },
        { lbr_state::cm, "Combining mark" },
        { lbr_state::sp, "Space" },
        { lbr_state::al, "Alnum" },
        { lbr_state::wj, "Word joiner" },
        { lbr_state::gl, "NBSP" },
        { lbr_state::ba, "Break after" },
        { lbr_state::bb, "Break before" },
        { lbr_state::hy, "Hyphen" },
        { lbr_state::cl, "Close paren/punct" },
        { lbr_state::cl_sp, "Close paren/punct + SP" },
        { lbr_state::op, "Open paren" },
        { lbr_state::ex, "Excalmation" },
        { lbr_state::is, "Infix number separator" },
        { lbr_state::sy, "Break after, not before" },
        { lbr_state::qu, "Quote" },
        { lbr_state::qu_sp, "Quote + SP" },
        { lbr_state::b2, "Break after with SP" },
        { lbr_state::cb, "Conditional break" },
        { lbr_state::hl, "Hebrew letter" },
        { lbr_state::hl_hyba, "Hebrew letter + break-after" },
        { lbr_state::nu, "Number" },
        { lbr_state::pr, "Num prefix" },
        { lbr_state::po, "Num postfix" },
        { lbr_state::id, "Ideographic" },
        { lbr_state::eb, "Emoji base" },
        { lbr_state::em, "Emoji modifier" },
        { lbr_state::jl, "Leading Jamo" },
        { lbr_state::jv, "Vowel Jamo" },
        { lbr_state::jt, "Trailing Jamo" },
        { lbr_state::h2, "Hangul LV" },
        { lbr_state::h3, "Hangul LVT" },
        { lbr_state::ri_odd, "Odd regional indicator" },
        { lbr_state::ri_even, "Even regional indicator" },
        { lbr_state::cp_narrow, "Closing paren, narrow" },
        { lbr_state::cp_wide, "Closing paren, wide" },
        { lbr_state::cp_sp, "Close paren + SP" },
        { lbr_state::ns, "Non-starter" },
        { lbr_state::in, "Ellipsis" },
        { lbr_state::b2_sp, "Break after + SP" },
        }),
      f::visual_short = span<const f::enum_value>({
        { lbr_state::none, "∅" },
        { lbr_state::bk, "BK" },
        { lbr_state::cr, "CR" },
        { lbr_state::lf, "LF" },
        { lbr_state::zw, "ZW" },
        { lbr_state::zwj, "ZWJ" },
        { lbr_state::cm, "CM" },
        { lbr_state::sp, "SP" },
        { lbr_state::al, "AL" },
        { lbr_state::wj, "WJ" },
        { lbr_state::gl, "GL" },
        { lbr_state::ba, "BA" },
        { lbr_state::bb, "BB" },
        { lbr_state::hy, "HY" },
        { lbr_state::cl, "CL" },
        { lbr_state::cl_sp, "CL+SP" },
        { lbr_state::op, "OP" },
        { lbr_state::ex, "EX" },
        { lbr_state::is, "IS" },
        { lbr_state::sy, "SY" },
        { lbr_state::qu, "QU" },
        { lbr_state::qu_sp, "QU+SP" },
        { lbr_state::b2, "B2" },
        { lbr_state::cb, "CB" },
        { lbr_state::hl, "HL" },
        { lbr_state::hl_hyba, "HL+HYBA" },
        { lbr_state::nu, "NU" },
        { lbr_state::pr, "PR" },
        { lbr_state::po, "PO" },
        { lbr_state::id, "ID" },
        { lbr_state::eb, "EB" },
        { lbr_state::em, "EM" },
        { lbr_state::jl, "JL" },
        { lbr_state::jv, "JV" },
        { lbr_state::jt, "JT" },
        { lbr_state::h2, "H2" },
        { lbr_state::h3, "H3" },
        { lbr_state::ri_odd, "RI1" },
        { lbr_state::ri_even, "RI2" },
        { lbr_state::cp_narrow, "CPN" },
        { lbr_state::cp_wide, "CPW" },
        { lbr_state::cp_sp, "CP+SP" },
        { lbr_state::ns, "NS" },
        { lbr_state::in, "IN" },
        { lbr_state::b2_sp, "B2+SP" },
      })
    };

    class J_TYPE_HIDDEN line_break_state_formatter final : public formatter_known_length<line_break_state> {
    public:
      void do_format(
        const const_string_view & format_options,
        const line_break_state & value,
        styled_sink & target,
        style init_style
      ) const override {
        lbr_state_formatter.do_format(format_options, value.m_state, target, init_style);
        if (value.m_eccs_base != lbr_state::none) {
          target.write(" (", 2);
          lbr_state_formatter.do_format(format_options, value.m_eccs_base, target, init_style);
          target.write(")", 1);
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const const_string_view & format_options,
        const line_break_state & value
      ) const noexcept override {
        return lbr_state_formatter.do_get_length(format_options, value.m_state)
          + (value.m_eccs_base != lbr_state::none
             ? lbr_state_formatter.do_get_length(format_options, value.m_eccs_base) + 3U
             : 0U);
      }
    };

    J_A(ND, NODESTROY) const line_break_state_formatter g_line_break_state_formatter;
  }
}
