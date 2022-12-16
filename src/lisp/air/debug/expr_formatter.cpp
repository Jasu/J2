// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/common/metadata.hpp"
#include "lisp/air/debug/debug_formatters.hpp"
#include "lisp/air/exprs/exprs.hpp"
#include "lisp/env/debug_formatters.hpp"
#include "lisp/sources/debug_formatters.hpp"
#include "lisp/values/debug_formatters.hpp"
#include "lisp/values/type_formatters.hpp"
#include "strings/formatters/numbers.hpp"
#include "strings/formatting/pad.hpp"
#include "strings/styling/styled_sink.hpp"

namespace j::lisp::air::inline debug {
  namespace e = exprs;
  namespace s = strings;

  namespace {
    struct J_TYPE_HIDDEN visitor final {
      s::styled_sink * to = nullptr;
      s::style cs{};
      u32_t indent = 0U;

      u32_t visit(id value) noexcept{
        if (to) {
          to->write(" ");
          env::g_id_formatter.do_format("", value, *to, cs);
        }
        return 1U + env::g_id_formatter.do_get_length("", value);
      }

      J_A(AI,ND) u32_t visit(const air_var * J_NOT_NULL value) noexcept{
        return visit(value->name);
      }

      u32_t visit(lisp_imm value) noexcept {
        if (to) {
          to->write(" ");
          g_lisp_imm_formatter.do_format("", value, *to, cs);
        }
        return 1U + g_lisp_imm_formatter.do_get_length("", value);
      }

      u32_t visit(u64_t value) noexcept {
        if (to) {
          to->write(" ");
          s::formatters::number_formatter_v<u64_t>.do_format("", value, *to, cs);
        }
        return 1U + s::formatters::number_formatter_v<u64_t>.do_get_length("", value);
      }

      u32_t visit(const imm_type_mask & value) noexcept {
        if (to) {
          to->write(" ");
          g_imm_type_mask_formatter.do_format("", value, *to, cs);
        }
        return 1U + g_imm_type_mask_formatter.do_get_length("", value);
      }

      [[nodiscard]] u32_t newline() noexcept {
        if (to) {
          to->write("\n");
          s::write_spaces(*to, indent);
        }
        return 1U + indent;
      }

      u32_t visit(const exprs::expr & e) noexcept {
        u32_t result = 2U + g_expr_type_formatter.do_get_length("", e.type);
        if (to) {
          to->write("(");
          g_expr_type_formatter.do_format("", e.type, *to, cs);
        }
        switch (e.type) {
          case expr_type::ld_const:
            result += visit(reinterpret_cast<const exprs::ld_const&>(e).const_val
);
            break;
          case expr_type::fn_arg:
            result += visit(reinterpret_cast<const exprs::fn_arg&>(e).index
);
            break;
          case expr_type::lex_rd:
            result += visit(reinterpret_cast<const exprs::lex_rd&>(e).var
);
            break;
          case expr_type::lex_rd_bound:
            result += visit(reinterpret_cast<const exprs::lex_rd_bound&>(e).var
);
            break;
          case expr_type::lex_wr:
            result += visit(reinterpret_cast<const exprs::lex_wr&>(e).var
);
            break;
          case expr_type::lex_wr_bound:
            result += visit(reinterpret_cast<const exprs::lex_wr_bound&>(e).var
);
            break;
          case expr_type::sym_val_rd:
            result += visit(reinterpret_cast<const exprs::sym_val_rd&>(e).var
);
            break;
          case expr_type::sym_val_wr:
            result += visit(reinterpret_cast<const exprs::sym_val_wr&>(e).var
);
            break;
          case expr_type::type_assert:
            result += visit(reinterpret_cast<const exprs::type_assert&>(e).types
);
            break;
          case expr_type::dyn_type_assert:
            result += visit(reinterpret_cast<const exprs::dyn_type_assert&>(e).types
);
            break;
          case expr_type::full_call:
            result += visit(reinterpret_cast<const exprs::full_call&>(e).fn()
);
            break;
          case expr_type::act_rec_alloc:
            result += visit(reinterpret_cast<const exprs::act_rec_alloc&>(e).index
);
            break;
          case expr_type::act_rec_wr:
            result += visit(reinterpret_cast<const exprs::act_rec_wr&>(e).var
);
            break;
          case expr_type::act_rec_rd:
            result += visit(reinterpret_cast<const exprs::act_rec_rd&>(e).var
);
            break;
          default:
            break;
        }
        indent += 2U;
        for (auto & in : e.inputs()) {
          result += newline() + visit(*in.expr);
        }
        indent -= 2U;
        if (to) {
          to->write(")");
        }
        return result;
      }
    };

    struct J_TYPE_HIDDEN air_expr_formatter final : s::formatter_known_length<exprs::expr> {
      void do_format(const s::const_string_view &, const exprs::expr & e,
                     s::styled_sink & to, s::style cs) const override
      { visitor{&to, cs}.visit(e); }

      [[nodiscard]] u32_t do_get_length(const s::const_string_view &,
                                        const exprs::expr & e) const noexcept override
      { return visitor{}.visit(e); }
    };

    J_A(ND,NODESTROY) const air_expr_formatter expr_fmt;
  }
  const strings::formatter_known_length<exprs::expr> & g_expr_formatter = expr_fmt;
}
