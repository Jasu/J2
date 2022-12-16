#include "lisp/cir/debug/op_formatter.hpp"
#include "lisp/cir/ops/op.hpp"
#include "lisp/cir/locs/debug_formatters.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"
#include "strings/formatters/numbers.hpp"

namespace j::lisp::cir::inline debug {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    static const f::debug_enum_entry use_type_entries[] = {
      [(u8_t)use_type::none]      = { "None", s::styles::bright_red },
      [(u8_t)use_type::op_result] = { "Def", s::styles::bright_cyan },
      [(u8_t)use_type::op_input]  = { "Op input", s::styles::bright_green },
      [(u8_t)use_type::mem_input_base]  = { "Mem base input", s::styles::bright_green },
      [(u8_t)use_type::mem_input_index]  = { "Mem index input", s::styles::bright_green },
      [(u8_t)use_type::move]  = { "Mem index input", s::styles::bright_magenta },
      [(u8_t)use_type::callee_saved]  = { "Callee-saved", s::styles::bright_yellow },
    };
    J_A(ND, NODESTROY) const s::formatters::debug_enum_formatter<use_type> use_type_formatter(use_type_entries);

    class use_formatter final : public s::formatter_known_length<use> {
    public:
      void do_format(const s::const_string_view &, const use & v, s::styled_sink & to, s::style cs) const override {
        use_type_formatter.do_format("", v.type, to, cs);
        if (!v) {
          return;
        }
        to.write(" ");
        g_op_index_formatter.do_format("", v.op()->index, to, cs);
        to.write("@");
        g_op_type_formatter.do_format("", v.op()->type, to, cs);
        if (v.loc_in || v.loc_out) {
          to.write(" ");
          if (v.loc_in) {
            g_loc_formatter.do_format("", v.loc_in, to, cs);
          }
          if (v.loc_out && v.loc_out != v.loc_in) {
            if (v.loc_in) {
              to.write(" -> ");
            }
            g_loc_formatter.do_format("", v.loc_out, to, cs);
          }
        }
      }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &, const use & v) const noexcept override {
        u32_t result = use_type_formatter.do_get_length("", v.type);
        if (!v) {
          return result;
        }
        result += 1U + g_op_index_formatter.do_get_length("", v.op()->index)
          + 1U + g_op_type_formatter.do_get_length("", v.op()->type);
        if (v.loc_in || v.loc_out) {
          ++result;
          if (v.loc_in) {
            result += g_loc_formatter.do_get_length("", v.loc_in);
          }
          if (v.loc_out && v.loc_out != v.loc_in) {
            if (v.loc_in) {
              result += 2U;
            }
            result += g_loc_formatter.do_get_length("", v.loc_out);
          }
        }
        return result;
      }
    };

    J_A(ND, NODESTROY) const use_formatter use_fmt;
  }
}
