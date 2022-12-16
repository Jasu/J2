#include "hzd/ctype.hpp"
#include "meta/parsing.hpp"
#include "meta/rules/parse_test_case.hpp"
#include "meta/term.hpp"
#include "meta/module.hpp"
#include "meta/rules/test_case.hpp"

namespace j::meta::inline rules {
  namespace {
    namespace s = j::strings;

    J_A(NODISC,RNN) terms_matcher * parse_terms_matcher(base_parser_state & state, val_type * J_NOT_NULL type) {
      J_ASSERT(type->is_trs_terms_type);
      vector<term_matcher*> terms;
      char ch = skip_ws(state);
      while (ch != '?' && ch != ')') {
        if (ch == '_') {
          terms.push_back(nullptr);
        } else {
          --state.cursor;
          terms.push_back(parse_term_matcher(state));
        }
        ch = skip_ws(state);
      }
      --state.cursor;
      return ::new terms_matcher(terms, type);
    }

    J_A(NODISC) val_eq_matcher * parse_val_eq(base_parser_state & state, val_type * J_NOT_NULL mem_type) {
      if (*state.cursor == '?') {
        ++state.cursor;
        return nullptr;
      } else if (is_upper(*state.cursor)) {
        auto name = parse_name(state);
        if (name == "True" || name == "False") {
          return ::new val_eq_matcher(attr_value(name[0] == 'T'), mem_type);
        }
        return ::new val_eq_matcher(state.mod->get_val_const(name).value, mem_type);
      } else if (is_lower(*state.cursor)) {
        matcher_base * m = state.matcher_macro_maybe_at(parse_id(state));
        state.check(m, "Macro not found");
        state.check(m->get_type() == matcher_type::val_eq, "Macro is not a val matcher");
        return (val_eq_matcher*)m;
      } else {
        return ::new val_eq_matcher(parse_attr_value(state, mem_type->parse_as ? &mem_type->parse_as : nullptr), mem_type);
      }
    }
  }

  J_A(NODISC,RNN) term_matcher * parse_term_matcher(base_parser_state & state) {
    char ch = skip_ws(state);
    --state.cursor;
    if (is_lower(ch)) {
      auto id = parse_id(state, "parsing term id");
      if (matcher_base * m = state.matcher_macro_maybe_at(id)) {
        state.check(m->get_type() == matcher_type::match_term, "Macro is not a term matcher");
        return (term_matcher*)m;
      }
      term * t = state.term_at(id);
      auto operands = t->operands();
      matcher_base * children[operands.size()];
      j::memzero(children, J_PTR_SZ * operands.size());
      i32_t i = 0;
      if (skip_ws(state) == '(') {
        for (term_member * mem : operands) {
          char ch = skip_ws(state);
          if (ch == ')') {
            --state.cursor;
            break;
          }
          --state.cursor;
          if (mem->type->is_trs_term_type) {
            children[i] = parse_term_matcher(state);
          } else if (mem->type->is_trs_terms_type) {
            if (ch == '*') {
              ++state.cursor;
              children[i] = nullptr;
            } else {
              children[i] = parse_terms_matcher(state, mem->type);
            }
          } else {
            children[i] = parse_val_eq(state, mem->type);
          }
          ++i;
        }
        skip_ws_past(state, ")", "expected end of ctor args");
      } else {
        --state.cursor;
      }
      return ::new term_matcher(t, span(children, i));
    }
    term * t = state.mod->trs_const_term;
    matcher_base * matcher = parse_val_eq(state, t->ctor[0]->type);
    return ::new term_matcher(t, span(&matcher, 1));
  }
  J_A(RNN,NODISC) matcher_base * parse_matcher(base_parser_state & state) {
    char c = skip_ws(state);
    --state.cursor;
    if (c == '\'') {
      return parse_val_eq(state, state.mod->trs_id_type);
    }
    return parse_term_matcher(state);
  }

  J_A(RNN,NODISC) test_case * parse_test_case(base_parser_state & state) {
    test_case * result = ::new test_case(parse_string_out(state));
    te_type type(term_expr_type_kind::term_or_terms);
    result->construct = parse_term_expr(state, nullptr, &type);
    state.check(state.cursor[0] == '-', "Expected \"->\" after test case constructor.");
    state.check(state.cursor[1] == '>', "Expected \"->\" after test case constructor.");
    state.cursor += 2;
    result->matcher = parse_term_matcher(state);
    skip_ws_past(state, ";", "Expected ';' after a TestCase.");
    return result;
  }
}
