// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/debug/graphviz_format.hpp"
#include "lisp/air/exprs/exprs.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/common/metadata.hpp"
#include "graphviz/table.hpp"
#include "strings/format.hpp"

namespace j::lisp::air::inline debug {
  namespace e = exprs;
  namespace g = graphviz;
  namespace s = strings;

  namespace {
    inline const g::table_style s_title      {{},        "#FFFFFF", 10.0f, g::align_center, g::valign_middle, false, false, 1U};
    inline const g::table_style s_arg        {"#D0C8F4", "#000000", 7.5f, g::align_center, g::valign_middle, false, false, 1U};
    inline const g::table_style s_arg_name   {{}, "#FFFFFF", 10.0f, g::align_center, g::valign_middle, false, false, 0};
    inline const g::table_style s_input      {"#80C0FF", "#000000"};
    inline const g::table_style s_result     {"#F8A6F8", "#000000", 0.0f, g::align_center};
    inline const g::table_style s_source_loc {{}, "#F0F0F0", 0.0f, g::align_right, g::valign_none, false, false, 0};
    inline const g::table_style s_default    {"#FFE090", "#000000"};

    inline const g::table_style s_true       {"#20D820", "#FFFFF0", 0.0f, g::align_center};
    inline const g::table_style s_false      {"#D01212", "#FFFFF0", 0.0f, g::align_center};
    inline const g::table_style s_nil        {"#7E7E8E", "#FFFFFF", 0.0f, g::align_center};
    inline const g::table_style s_sym        {"#2020C8", "#FFFFFF", 0.0f, g::align_center};
    inline const g::table_style s_i64        {"#20C820", "#FFFFFF", 0.0f, g::align_center};

    struct J_TYPE_HIDDEN expr_debug_flags final {
      bool force_result:1;
      bool hide_result:1;
      bool hide_rep:1;
      bool hide_type:1;
      bool hide_inputs:1;
      bool inline_result:1;
   };

    constexpr inline expr_debug_flags flag_tbl[num_expr_types_v]{
      [(u8_t)expr_type::nop] = {false,true,false,false,false,false},
      [(u8_t)expr_type::ld_const] = {false,true,false,false,false,false},
      [(u8_t)expr_type::fn_arg] = {false,true,false,false,false,false},
      [(u8_t)expr_type::fn_rest_arg] = {false,false,false,false,false,true},
      [(u8_t)expr_type::fn_sc_arg] = {false,false,false,false,false,true},
      [(u8_t)expr_type::fn_body] = {false,false,false,false,true,true},
      [(u8_t)expr_type::progn] = {false,false,false,false,true,true},
      [(u8_t)expr_type::prog1] = {false,false,false,false,true,true},
      [(u8_t)expr_type::let] = {false,false,false,false,true,true},
      [(u8_t)expr_type::b_if] = {false,false,false,false,false,false},
      [(u8_t)expr_type::do_until] = {true,false,false,false,true,true},
      [(u8_t)expr_type::lex_rd] = {true,false,false,false,false,false},
      [(u8_t)expr_type::lex_rd_bound] = {true,false,false,false,false,false},
      [(u8_t)expr_type::lex_wr] = {true,false,false,false,true,false},
      [(u8_t)expr_type::lex_wr_bound] = {true,false,false,false,true,false},
      [(u8_t)expr_type::sym_val_rd] = {false,false,true,false,false,true},
      [(u8_t)expr_type::sym_val_wr] = {false,false,true,false,false,true},
      [(u8_t)expr_type::type_assert] = {false,true,false,false,true,false},
      [(u8_t)expr_type::dyn_type_assert] = {false,true,false,false,true,false},
      [(u8_t)expr_type::ineg] = {false,false,false,true,false,true},
      [(u8_t)expr_type::iadd] = {false,false,false,true,false,true},
      [(u8_t)expr_type::isub] = {false,false,false,true,false,true},
      [(u8_t)expr_type::imul] = {false,false,false,true,false,true},
      [(u8_t)expr_type::idiv] = {false,false,false,true,false,true},
      [(u8_t)expr_type::lor] = {false,true,false,false,false,false},
      [(u8_t)expr_type::land] = {false,true,false,false,false,false},
      [(u8_t)expr_type::lnot] = {false,true,false,false,false,true},
      [(u8_t)expr_type::to_bool] = {false,true,false,false,false,true},
      [(u8_t)expr_type::eq] = {false,true,false,false,false,true},
      [(u8_t)expr_type::neq] = {false,true,false,false,false,true},
      [(u8_t)expr_type::tag] = {false,false,true,false,true,true},
      [(u8_t)expr_type::untag] = {false,false,true,false,true,true},
      [(u8_t)expr_type::as_range] = {false,false,true,false,false,false},
      [(u8_t)expr_type::as_vec] = {false,false,true,false,false,false},
      [(u8_t)expr_type::call] = {false,false,false,false,false,false},
      [(u8_t)expr_type::full_call] = {false,false,true,false,false,false},
      [(u8_t)expr_type::vec] = {false,false,false,true,false,true},
      [(u8_t)expr_type::make_vec] = {false,false,false,true,false,true},
      [(u8_t)expr_type::vec_append] = {false,false,false,true,false,true},
      [(u8_t)expr_type::vec_build] = {false,false,false,true,false,true},
      [(u8_t)expr_type::range_length] = {false,false,false,true,false,true},
      [(u8_t)expr_type::range_get] = {false,false,true,false,false,false},
      [(u8_t)expr_type::range_set] = {false,false,true,false,false,false},
      [(u8_t)expr_type::range_copy] = {false,false,false,true,false,true},
      [(u8_t)expr_type::act_rec_alloc] = {false,false,false,true,false,true},
      [(u8_t)expr_type::act_rec_wr] = {false,false,false,false,false,true},
      [(u8_t)expr_type::act_rec_rd] = {false,false,false,false,true,true},
      [(u8_t)expr_type::act_rec_parent] = {false,true,false,false,true,false},
      [(u8_t)expr_type::closure] = {false,false,false,true,false,false},
   };

    inline g::color expr_colors[num_expr_types_v]{
      [(u8_t)expr_type::nop] = "#A4A4A4",
      [(u8_t)expr_type::ld_const] = "#809A9A",
      [(u8_t)expr_type::fn_arg] = "#3400E0",
      [(u8_t)expr_type::fn_rest_arg] = "#C0B090",
      [(u8_t)expr_type::fn_sc_arg] = "#B8D0A4",
      [(u8_t)expr_type::fn_body] = "#A0A0C0",
      [(u8_t)expr_type::progn] = "#A0A0A0",
      [(u8_t)expr_type::prog1] = "#A0A0A0",
      [(u8_t)expr_type::let] = "#C0C0A4",
      [(u8_t)expr_type::b_if] = "#7878B4",
      [(u8_t)expr_type::do_until] = "#780EB4",
      [(u8_t)expr_type::lex_rd] = "#00A800",
      [(u8_t)expr_type::lex_rd_bound] = "#50A800",
      [(u8_t)expr_type::lex_wr] = "#B85800",
      [(u8_t)expr_type::lex_wr_bound] = "#C88010",
      [(u8_t)expr_type::sym_val_rd] = "#70A400",
      [(u8_t)expr_type::sym_val_wr] = "#C86800",
      [(u8_t)expr_type::type_assert] = "#F80000",
      [(u8_t)expr_type::dyn_type_assert] = "#D89A18",
      [(u8_t)expr_type::ineg] = "#000000",
      [(u8_t)expr_type::iadd] = "#000000",
      [(u8_t)expr_type::isub] = "#000000",
      [(u8_t)expr_type::imul] = "#000000",
      [(u8_t)expr_type::idiv] = "#000000",
      [(u8_t)expr_type::lor] = "#000000",
      [(u8_t)expr_type::land] = "#000000",
      [(u8_t)expr_type::lnot] = "#000000",
      [(u8_t)expr_type::to_bool] = "#000000",
      [(u8_t)expr_type::eq] = "#000000",
      [(u8_t)expr_type::neq] = "#000000",
      [(u8_t)expr_type::tag] = "#00FFD0",
      [(u8_t)expr_type::untag] = "#C800E8",
      [(u8_t)expr_type::as_range] = "#B8B800",
      [(u8_t)expr_type::as_vec] = "#D8A020",
      [(u8_t)expr_type::call] = "#999900",
      [(u8_t)expr_type::full_call] = "#999900",
      [(u8_t)expr_type::vec] = "#000000",
      [(u8_t)expr_type::make_vec] = "#000000",
      [(u8_t)expr_type::vec_append] = "#000000",
      [(u8_t)expr_type::vec_build] = "#000000",
      [(u8_t)expr_type::range_length] = "#000000",
      [(u8_t)expr_type::range_get] = "#000000",
      [(u8_t)expr_type::range_set] = "#000000",
      [(u8_t)expr_type::range_copy] = "#000000",
      [(u8_t)expr_type::act_rec_alloc] = "#000000",
      [(u8_t)expr_type::act_rec_wr] = "#000000",
      [(u8_t)expr_type::act_rec_rd] = "#000000",
      [(u8_t)expr_type::act_rec_parent] = "#000000",
      [(u8_t)expr_type::closure] = "#303099",
    };

    [[nodiscard]] g::table make_table(const e::expr * J_NOT_NULL expr) {
      return g::table{g::table_style{expr_colors[(u8_t)expr->type], "#000000", 9.0f, g::align_left}};
    }

    void push_pair(g::table & tbl, const char * J_NOT_NULL title, s::string && val) {
      tbl.push_row(s_default, {strings::string(title), static_cast<strings::string &&>(val)});
    }

    void push_inline_title(g::table & tbl, const char * J_NOT_NULL title_fmt,
                           s::string && result, const e::expr * J_NOT_NULL expr) {
      tbl.push_flex_row((g::table_column[]){
        { s::format(title_fmt, expr->type), s_title, "r" },
        { static_cast<strings::string &&>(result), s_result},
      });
    }

    void push_lex_title(g::table & tbl, const char * J_NOT_NULL title, s::string & result, air_var * J_NOT_NULL var) {
      tbl.push_flex_row((g::table_column[]){
          {title, s_title, "r"},
          {static_cast<strings::string &&>(result), s_result},
          {s::format("{}", var->name), s_default, "op0"},
      });
    }
  }

  [[nodiscard]] g::table debug_to_table(const air_function * J_NOT_NULL fn,
                                        const e::expr * J_NOT_NULL expr, bool omit_result) {
    const auto flags = flag_tbl[(u8_t)expr->type];

    strings::string result = ((omit_result && !flags.force_result) || flags.hide_result) ? ""
      : (flags.hide_rep ? s::format("{}", expr->result.types)
      : (flags.hide_type ? s::format("{}", expr->result.reps)
      : s::format("{}", expr->result)));

    g::table tbl = make_table(expr);

    if (flags.inline_result && result) {
      push_inline_title(tbl, "{}", static_cast<strings::string &&>(result), expr);
    } else {
      switch(expr->type) {
      case expr_type::ld_const: {
        auto imm = reinterpret_cast<const exprs::ld_const*>(expr)->const_val;
        switch (imm.type()) {
        case imm_nil:
          tbl.push_row(s_nil, (s::string[]){"Nil"});
          break;
        case imm_bool:
          tbl.push_row(imm.is_true() ? s_true : s_false, (s::string[]){imm.is_true() ? "True" : "False"});
          break;
        case imm_sym_id:
          tbl.push_row(s_sym, (s::string[]){s::format("<b>'</b>{}", imm)});
          break;
        case imm_i64:
          tbl.push_flex_row((g::table_column[]){
            { s::format("{}", imm), s_i64 },
            { s::format("{}", expr->result.reps), s_result },
          });
          break;
        default:
            tbl.push_flex_row((g::table_column[]){
              { s::format("Const {}", imm.type()), s_title },
              { s::format("{}", expr->result.reps), s_result },
            });
            tbl.push_row(s_default, (s::string[]){ s::format("{}", imm) });
          break;
        }
        break;
      }
      case expr_type::fn_arg: {
        auto arg_idx = reinterpret_cast<const exprs::fn_arg*>(expr)->index;
        g::table_column cols[3]{
          {s::format("<font point-size=\"2.75\"><br/></font>&nbsp;Arg <b>#{}</b>", arg_idx), s_arg, "r"},
          {s::format("<font point-size=\"2.5\"><br/></font>&nbsp;&nbsp;{}&nbsp;", ((const exprs::fn_body*)fn->body)->vars()[arg_idx]->name), s_arg_name},
          {static_cast<strings::string &&>(result), s_result},
        };
        tbl.push_flex_row(cols, expr->result.types == taggable_imm_type ? 2U : 3U);
        break;
      }
      case expr_type::lex_rd: {
        push_lex_title(tbl, "lex-rd", result, reinterpret_cast<const exprs::lex_rd*>(expr)->var);
        break;
      }
      case expr_type::lex_rd_bound: {
        push_lex_title(tbl, "bnd-rd", result, reinterpret_cast<const exprs::lex_rd_bound*>(expr)->var);
        break;
      }
      case expr_type::lex_wr: {
        push_lex_title(tbl, "lex-wr", result, reinterpret_cast<const exprs::lex_wr*>(expr)->var);
        break;
      }
      case expr_type::lex_wr_bound: {
        push_lex_title(tbl, "bnd-wr", result, reinterpret_cast<const exprs::lex_wr_bound*>(expr)->var);
        break;
      }
      case expr_type::type_assert: {
          tbl.push_row((g::table_column[]){{s::format("<b>{}!</b>", reinterpret_cast<const exprs::type_assert*>(expr)->types), s_title, "op0"}});
        break;
      }
      case expr_type::dyn_type_assert: {
          tbl.push_row((g::table_column[]){{s::format("<b>{}!</b>", reinterpret_cast<const exprs::dyn_type_assert*>(expr)->types), s_title, "op0"}});
        break;
      }
      default:
        tbl.push_row(s_title, (s::string[]){s::format("{}", expr->type)});
        break;
      }
    }

    if (!flags.hide_inputs) {
      for (u8_t idx = 0U, max = expr->num_inputs; idx != max; ++idx) {
        tbl.push_row((g::table_column[]){
          {s::format("Op {}", idx), s_input},
          {s::format("{}", expr->input(idx).type), s_input, s::format("op{}", idx)},
        });
      }
    }

    switch(expr->type) {
      case expr_type::sym_val_rd:
        push_pair(tbl, "Src Var", s::format("{}", reinterpret_cast<const exprs::sym_val_rd*>(expr)->var
  ));
        break;
      case expr_type::sym_val_wr:
        push_pair(tbl, "Dst Var", s::format("{}", reinterpret_cast<const exprs::sym_val_wr*>(expr)->var
  ));
        break;
      case expr_type::full_call:
        push_pair(tbl, "Fn Name", s::format("{}", reinterpret_cast<const exprs::full_call*>(expr)->fn()
  ));
        break;
      case expr_type::act_rec_alloc:
        push_pair(tbl, "ActRec Index", s::format("#{}", reinterpret_cast<const exprs::act_rec_alloc*>(expr)->index
  ));
        break;
      case expr_type::act_rec_wr:
        push_pair(tbl, "Bound Lexvar", s::format("{}", reinterpret_cast<const exprs::act_rec_wr*>(expr)->var
  ->name));
        break;
      case expr_type::act_rec_rd:
        push_pair(tbl, "Bound Lexvar", s::format("{}", reinterpret_cast<const exprs::act_rec_rd*>(expr)->var
  ->name));
        break;
      default: break;
    }

    if (!flags.inline_result) {
      if (result) {
        tbl.push_row((g::table_column[]){{static_cast<strings::string &&>(result), s_result, "r"}});
      } else {
        tbl.port = "r";
      }
    }

    if(auto loc = expr->metadata().find(metadata_type::source_location)) {
      if (loc->source_location.line != 0 || loc->source_location.column != 0) {
        tbl.push_row(s_source_loc, (s::string[]){
          s::format("<font point-size=\"3.0\"><br/><font point-size=\"6.5\">{}</font></font>", loc->source_location),
        });
      }
    }
    return tbl;
  }

  [[nodiscard]] g::table format_result_stub(const e::expr * J_NOT_NULL expr) {
    g::table tbl = make_table(expr);
    push_inline_title(tbl, "{}<br/><font point-size=\"7.0\">result</font>", s::format("{}", expr->result), expr);
    return tbl;
  }
}
