#include "meta/parsing.hpp"
#include "exceptions/exceptions.hpp"
#include "meta/dump.hpp"
#include "meta/cpp_codegen.hpp"
#include "parsing/precedence_parser.hpp"
#include "meta/basic_node_set.hpp"
#include "logging/global.hpp"
#include "meta/term.hpp"
#include "meta/module.hpp"
#include "meta/expr.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"

/*!rules:re2c:common
  re2c:api:style             = free-form;
  re2c:define:YYCTYPE        = "char";
  re2c:define:YYCURSOR       = "state.cursor";
  re2c:define:YYLIMIT        = "state.limit";
  re2c:define:YYMARKER       = "marker";
  re2c:define:YYFILL         = "(void)0;";

  // Allow rules to have @var_name to store the position in the variable.
  re2c:flags:tags = 1;

  NL         = [\n];
  COMMENT    = "#" [^\n]*;
  WS         = [ ]+;
  NAME       = [A-Z][A-Za-z0-9]*;
  ID_OR_NAME = [A-Za-z][A-Za-z0-9_-]*;
*/

namespace j::meta {
  namespace s = j::strings;
  namespace p = j::parsing;
  namespace {
    J_A(RNN,NODISC,AI) inline const p::operator_info * get_op_info(const expr * J_NOT_NULL op) noexcept {
      return &exprs[(i32_t)op->type];
    }

    J_A(RNN,NODISC) expr * preduce_binary(expr * J_NOT_NULL J_RESTRICT op, expr * J_NOT_NULL J_RESTRICT lhs, expr * J_RESTRICT rhs) noexcept {
      op->ops[0] = lhs;
      op->ops[1] = rhs;
      return op;
    }

    J_A(RNN,NODISC) expr * preduce_unary(expr * J_NOT_NULL J_RESTRICT op, expr * J_NOT_NULL J_RESTRICT lhs) noexcept {
      if (op->type == expr_method_call) {
        op->call.args[0] = lhs;
      } else {
        op->ops[0] = lhs;
      }
      return op;
    }

    using precedence_parser = p::precedence_parser<expr*, expr*, get_op_info, preduce_binary, preduce_unary>;

    [[nodiscard]] span<expr*> parse_body(base_parser_state & state, expr ** J_NOT_NULL target) {
      u32_t num = 0U;
      char ch = skip_ws(state);
      for (--state.cursor; ch != ')';) {
        target[num++] = parse_expr(state);
        ch = *state.cursor;
        if (ch == ',') {
          ++state.cursor;
        }
      }
      ++state.cursor;
      return span(target, num);
    }


    J_A(NODISC) attr_value parse_node_set(base_parser_state & state, attr_type type) {
      basic_node_set nodes;
      for (;;) {
        nodes.add(*state.mod->node_at(parse_name_or_id(state)));
        if (skip_ws_past(state, ",)") == ')') {
          break;
        }
      }
      return attr_value(type, nodes);
    }

    const pair<const char*, expr_type> comprehension_names[]{
      { "Filter",    expr_filter },
      { "Find",      expr_find },
      { "FindIndex", expr_find_index },
      { "Map",       expr_map },
      { "Reduce",    expr_reduce },
      { "Every",     expr_every },
      { "SortBy",    expr_sort_by },
      { "GroupBy",   expr_group_by },
      { "Some",      expr_some },
    };

    J_A(RNN,NODISC) expr * parse_comprehension(base_parser_state & state, s::const_string_view name) {
      expr_type t = expr_none;
      for (auto & p : comprehension_names) {
        if (name == p.first) {
          t = p.second;
        }
      }

      s::const_string_view it_name, idx_name;
      expr * e = parse_expr(state);
      expr * acc_init = nullptr;
      expr * dummy_e = nullptr;
      if (*state.cursor == ':' || *state.cursor == ',') {
        state.check(e->type == expr_var, "Expected var name before ':' or ','", "in comprehension");
        it_name = e->name;
        dummy_e = e;
        if (*state.cursor == ',') {
          ++state.cursor;
          idx_name = parse_name(state);
          skip_ws_up_to(state, ":", "in comprehension");
        }
        ++state.cursor;
        e = parse_expr(state);
      }
      if (t == expr_reduce) {
        state.check(*state.cursor++ == ';', "Expected ';'", "in Reduce");
        acc_init = parse_expr(state);
      }
      state.check(*state.cursor++ == ']', "Expected end of comprehension", "in comprehension");
      e = ::new expr(t, it_name, idx_name, nullptr, e, acc_init, state.loc());
      ::delete dummy_e;
      return e;
    }

    J_A(RNN,NI) expr * push_op_or_expr(base_parser_state & state, precedence_parser & prec, bool is_op, expr * J_NOT_NULL e) {
      try {
        if (is_op) {
          prec.push_op(e);
        } else {
          prec.push_expr(e);
        }
        return e;
      } catch (...) {
        state.fail("Unexpected", dump_str(state.mod, *e));
      }
    }

    template<typename... Args>
    expr * emplace_expr(base_parser_state & state, precedence_parser & prec, Args && ... args) {
      return push_op_or_expr(state, prec, false, ::new expr{static_cast<Args &&>(args)...});
    }

    J_A(RNN) expr * emplace_expr(source_location loc, base_parser_state & state, precedence_parser & prec, attr_value && attr_v) {
      return emplace_expr(state, prec, static_cast<attr_value &&>(attr_v), loc);
    }

    template<typename... Args>
    expr * emplace_op(base_parser_state & state, precedence_parser & prec, Args && ... args) {
      return push_op_or_expr(state, prec, true, ::new expr{static_cast<Args &&>(args)...});
    }
  }

  J_A(RNN,NODISC) expr * parse_expr(base_parser_state & state) {
    s::const_string_view name;
    const char * begin = nullptr;
    const char * marker;
    precedence_parser prec;
    expr * args[16];
    /*!stags:re2c:expr format = "const char *@@ = nullptr;\n"; */
  loop:
    source_location loc = state.loc();
    /*!local:re2c:expr
      !use:common;
      WS { goto loop; }
      NL { state.newline(); goto loop; }

      @begin ("0b" [01]+ | "0x" [0-9a-fA-F]+ | [-]? [0-9]+) {
        i64_t num;
        if (begin[1] == 'x') {
          num = read_hex(begin, state.cursor);
        } else if (begin[1] == 'b') {
          num = read_bin(begin, state.cursor);
        } else {
          num = read_dec(begin, state.cursor);
        }
        emplace_expr(loc, state, prec, attr_value(num));
        goto loop;
      }
      ["] {
        emplace_expr(loc, state, prec, attr_value(parse_string(state)));
        goto loop;
      }

      @begin ("<=" | ">=" | "==" | "!=" | "||" | "&&" | "-" | [|&~!/+*<>]) {
        expr_type et = expr_none;
        switch (*begin) {
        case '=': et = expr_eq; break;
        case '<': et = begin[1] == '=' ? expr_le : expr_lt; break;
        case '>': et = begin[1] == '=' ? expr_ge : expr_gt; break;
        case '!': et = begin[1] == '=' ? expr_neq : expr_not; break;
        case '*': et = expr_mul; break;
        case '+': et = expr_add; break;
        case '/': et = expr_div; break;
        case '-': et = prec.prev_was_expr_or_suffix ? expr_sub : expr_neg; break;
        case '~': et = expr_bnot; break;
        case '&': et = begin[1] == '&' ? expr_and : expr_band; break;
        case '|': et = begin[1] == '|' ? expr_or : expr_bor; break;
        }
        emplace_op(state, prec, et, loc);
        goto loop;
      }

      @begin ("-]" | "-}" | "]" | [,)};:=]) {
        state.cursor = begin;
        return prec.finish();
      }

      "." [?]? @begin [@]? NAME [[(]? {
        name = s::const_string_view{begin, state.cursor - 1};
        if (state.cursor[-1] == '[')  {
          push_op_or_expr(state, prec, true, parse_comprehension(state, name));
        } else if (state.cursor[-1] != '(') {
          emplace_op(state, prec, begin[-1] == '?' ? expr_maybe_at_prop : expr_at_prop, nullptr, s::const_string_view{begin, state.cursor}, loc);
        } else if (name == "Has") {
          emplace_op(state, prec, expr_has, nullptr, parse_name_or_id(state), loc);
          skip_ws_past(state, ")");
        } else {
          emplace_op(state, prec, name, nullptr, parse_body(state, args), loc);
        }
        goto loop;
      }

      @begin ID_OR_NAME "(" ? {
        bool is_call = state.cursor[-1] == '(';
        name = s::const_string_view{begin, state.cursor - is_call};
        if (is_call) {
          bool is_types = name == "Types";
          if (is_types || name == "Terms") {
            emplace_expr(loc, state, prec, parse_node_set(state, is_types ? attr_val_type_set : attr_term_set));
          } else {
            span<expr*> children = parse_body(state, args);
            bool is_if = name == "If";
            if (is_if || name == "IsNone") {
              emplace_expr(state, prec, is_if ? expr_if : expr_is_none, args[0], is_if ? args[1] : nullptr, is_if ? args[2] : nullptr);
            } else {
              emplace_expr(state, prec, name, children, loc);
            }
          }
        } else {
          bool is_true = name == "True";
          if (is_true || name == "False") {
            emplace_expr(loc, state, prec, attr_value(is_true));
          } else {
            emplace_expr(state, prec, name, loc);
          }
        }
        goto loop;
      }

      "[" {
        emplace_expr(loc, state, prec, attr_value(parse_template(state)));
        goto loop;
      }

      [(] {
        push_op_or_expr(state, prec, false, parse_expr(state));
        skip_ws_past(state, ")");
        goto loop;
      }

      * { state.fail("Unexpected token", "when parsing expr"); }
     */
  }
}
#pragma clang diagnostic pop
