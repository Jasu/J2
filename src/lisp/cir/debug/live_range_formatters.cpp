#include "lisp/cir/debug/op_formatter.hpp"
#include "lisp/cir/liveness/live_range.hpp"
#include "lisp/cir/locs/debug_formatters.hpp"
#include "colors/default_colors.hpp"
#include "strings/formatting/pad.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::cir::inline debug {
  namespace {
    namespace s = strings;
    namespace c = colors::colors;

    const s::style color_gap{c::white, c::gray};
    const s::style color_active{c::black, c::yellow, s::bold};
    // const s::style color_defined{c::green, c::bright_yellow, s::bold};

    class live_range_formatter final : public s::formatter_known_length<live_range> {
    public:
      void do_format(
        const s::const_string_view &,
        const live_range & o,
        s::styled_sink & to,
        s::style cs
        ) const override {
        op_index prev{0U};
        for (auto & seg : o) {
          if (prev < seg.index_begin) {
            auto col = to.get_column();
            to.set_style(color_gap);
            to.write(" ");
            g_op_index_formatter.do_format("", prev, to, color_gap);
            to.write("-");
            g_op_index_formatter.do_format("", seg.index_begin, to, color_gap);
            if (seg.loc) {
              to.write(" in ");
              g_loc_formatter.do_format("", seg.loc, to, color_gap);
            }
            prev = seg.index_begin;
            s::write_spaces(to, 48 - (to.get_column() - col));
            to.set_style(cs);
            to.write("\n");

          }

          auto col = to.get_column();
          to.set_style(color_active);
          to.write(" ");
          g_op_index_formatter.do_format("", prev, to, color_active);
          to.write("-");
          g_op_index_formatter.do_format("", seg.index_end, to, color_active);
          if (seg.loc) {
            to.write(" in ");
            g_loc_formatter.do_format("", seg.loc, to, color_active);
          }
          to.write(" Spec: ");
          g_loc_specifier_formatter.do_format("", seg.loc_specifier, to, color_active);
          prev = seg.index_end;
          s::write_spaces(to, 48 - (to.get_column() - col));
          to.set_style(cs);
          to.write("\n");
        }
      }

      [[nodiscard]] u32_t do_get_length(
        const s::const_string_view &,
        const live_range & o
        ) const noexcept override {
        op_index prev{0U};
        u32_t result = 0;
        for (auto & seg : o) {
          if (prev < seg.index_begin) {
            result += 49;
          }
          result += 49;
          prev = seg.index_end;
        }
        return result;
      }
    };
    J_A(ND, NODESTROY) const live_range_formatter live_range_fmt;
  }
}
