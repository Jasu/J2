#include "lisp/air/debug/debug_formatters.hpp"
#include "lisp/air/exprs/expr_type.hpp"
#include "strings/formatters/enum_formatter.hpp"
#include "strings/styling/default_styles.hpp"

namespace j::lisp::air {
  namespace {
    namespace s = strings;
    namespace f = s::formatters;

    J_A(ND, NODESTROY) const f::enum_formatter<expr_type> expr_type_formatter{{
        { expr_type::nop,             "nop",           s::styles::gray },
        { expr_type::fn_body,         "fn-body",       s::styles::bold },
        { expr_type::fn_arg,          "arg",           s::styles::green },
        { expr_type::fn_rest_arg,     "rest-arg",      s::styles::bright_green },
        { expr_type::fn_sc_arg,       "sc-arg",        s::styles::bright_green },

        { expr_type::ld_const,        "const" },

        { expr_type::let,             "let",           s::styles::bright_green },
        { expr_type::lex_rd,          "lex-rd",        s::styles::green },
        { expr_type::lex_wr,          "lex-wr",        s::styles::bright_yellow },
        { expr_type::lex_rd_bound,    "rd-bound",      s::styles::bright_green },
        { expr_type::lex_wr_bound,    "wr-bound",      s::styles::bright_yellow },
        { expr_type::sym_val_rd,      "sym-rd",        s::styles::green },
        { expr_type::sym_val_wr,      "sym-wr",        s::styles::bright_yellow },

        { expr_type::ineg,            "ineg",          s::styles::bright_yellow },
        { expr_type::iadd,            "iadd",          s::styles::bright_yellow },
        { expr_type::isub,            "isub",          s::styles::bright_yellow },
        { expr_type::imul,            "imul",          s::styles::bright_yellow },
        { expr_type::idiv,            "idiv",          s::styles::bright_yellow },

        { expr_type::eq,              "eq",            s::styles::bright_yellow },
        { expr_type::neq,             "neq",           s::styles::bright_yellow },

        { expr_type::type_assert,     "type!",         s::styles::bright_red },
        { expr_type::dyn_type_assert, "dtype!",        s::styles::bright_red },


        { expr_type::call,            "call",          s::styles::bright_red },
        { expr_type::full_call,       "full-call",     s::styles::bright_red },

        { expr_type::b_if,            "if",            s::styles::bright_green },
        { expr_type::do_until,        "do-until",      s::styles::bright_green },

        { expr_type::lor,             "lor",           s::styles::bright_green },
        { expr_type::land,            "land",          s::styles::bright_green },
        { expr_type::lnot,            "lnot",          s::styles::green },

        { expr_type::to_bool,         "to-bool",       s::styles::magenta },

        { expr_type::progn,           "progn",         s::styles::light_gray },
        { expr_type::prog1,           "prog1",         s::styles::light_gray },

        { expr_type::tag,             "tag",           s::styles::magenta },
        { expr_type::untag,           "untag",         s::styles::magenta },
        { expr_type::as_range,        "as-range",      s::styles::magenta },
        { expr_type::as_vec,          "as-vec",        s::styles::magenta },

        { expr_type::vec,             "vec",           s::styles::bright_magenta },
        { expr_type::vec_build,       "vec-build",     s::styles::bright_red },
        { expr_type::make_vec,        "make-vec",      s::styles::bright_magenta },
        { expr_type::vec_append,      "vec-append",    s::styles::bright_magenta },

        { expr_type::range_copy,      "range-copy",    s::styles::bright_magenta },
        { expr_type::range_length,    "range-length",    s::styles::magenta },
        { expr_type::range_get,       "range-get",     s::styles::magenta },
        { expr_type::range_set,       "range-set",     s::styles::bright_yellow },

        { expr_type::act_rec_alloc,   "act-alloc",     s::styles::bright_red },
        { expr_type::act_rec_rd,      "act-rd",        s::styles::bright_yellow },
        { expr_type::act_rec_wr,      "act-wr",        s::styles::bright_red },
        { expr_type::act_rec_parent,  "act-parent",    s::styles::bright_yellow },
        { expr_type::closure,         "closure",       s::styles::bright_green },
      }};
  }

  const strings::formatter_known_length<expr_type> & g_expr_type_formatter = expr_type_formatter;
}
