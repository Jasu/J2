#include "meta/parsing.hpp"
#include "hzd/ctype.hpp"
#include "exceptions/exceptions.hpp"
#include "meta/rules/dump.hpp"
#include "parsing/precedence_parser.hpp"
#include "meta/expr.hpp"
#include "logging/global.hpp"
#include "meta/module.hpp"
#include "meta/rules/rule.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/rules/nonterminal.hpp"
#include "meta/term.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"

namespace j::meta {
  inline namespace rules {
    void parse_pat_terms(base_parser_state & state, pat_terms_t * J_NOT_NULL out);
    J_A(NODISC) pat_terms_t parse_pat_terms(base_parser_state & state);
  }
namespace {
  namespace s = j::strings;
  namespace p = parsing;

  inline void parse_te_args(base_parser_state & state, vector<term_expr*> & to, nt_p this_nt = nullptr, term * term = nullptr) {
    term_member ** cur_member = term ? &term->ctor[0] : nullptr;
    term_member ** end_member = term ? &term->ctor[term->num_inputs + term->num_consts + term->num_data] : nullptr;
    for (; *state.cursor != ')';) {
      te_type parse_as;
        if (cur_member != end_member) {
          bool is_at_terms = cur_member[0]->type->is_trs_terms_type;
          i32_t index = is_at_terms && cur_member + 1 != end_member;
          if (cur_member[index]->type->parse_as) {
            parse_as = cur_member[index]->type->parse_as;
          } else if (!index && (is_at_terms || cur_member[0]->type->is_trs_term_type)) {
            parse_as = te_type::term_or_terms;
          }
        }
        term_expr * parsed = parse_term_expr(state, this_nt, parse_as ? &parse_as : nullptr);
        to.push_back(parsed);
        if (cur_member != end_member) {
          if (cur_member[0]->type->is_trs_terms_type) {
            if (cur_member + 1 != end_member && (parsed->type == term_expr_type::arg || parsed->type == term_expr_type::constant
                || parsed->type == term_expr_type::prop_read || parsed->type == term_expr_type::member_fn_call
                                                 || parsed->type == term_expr_type::binop)) {
              cur_member += 2;
            }
          } else {
            ++cur_member;
          }
        }
      }
      ++state.cursor;
    }

    inline void parse_te_args_out(base_parser_state & state, vector<term_expr*> & to, nt_p this_nt = nullptr, term * term = nullptr) {
      skip_ws_past(state, "(");
      parse_te_args(state, to, this_nt, term);
    }

    inline void maybe_parse_reduction_group_opts(base_parser_state & state, reduction_group & rg, nt_p this_nt = nullptr) {
      if (skip_ws(state) == '[') {
        for (;;) {
          skip_ws(state);
          --state.cursor;
          auto kwd = parse_name(state, "reading pattern context keyword");
          if (kwd == "Pred" || kwd == "Predicate") {
            rg.predicate = parse_term_expr(state, this_nt);
          } else if (kwd == "Name") {
            rg.name = parse_name(state, "Reading name");
          } else {
            state.fail("Expected 'Predicate', 'Pred', or 'Name'", "in reduction group opts");
          }
          if (skip_ws_past(state, "],") == ']') {
            break;
          }
        }
      } else {
        --state.cursor;
      }
    }

  reduction parse_reduction(base_parser_state & state, nt_p reduce_as, bool & had_reduce) {
    reduction_args args{};
    char ch = skip_ws(state);
    source_location loc = state.loc();
    loc.column--;
    if (ch == '!') {
      if (*state.cursor == ' ') {
        args.emplace_back(parse_term_expr(state, reduce_as), rewrite_action::none, 0);
        return reduction(reduction_type::stmt, static_cast<reduction_args &&>(args), reduce_as, 0, loc);
      }
      auto s = parse_name(state, "when parsing reduction type");
      if (s == "Copy") {
        state.check(!reduce_as, "!Copy not available", "in repetition");
        had_reduce = true;
        args.emplace_back(parse_term_expr(state), rewrite_action::none, 0);
        return reduction(reduction_type::reduce_copy, static_cast<reduction_args &&>(args), {}, -1, loc);
      } else if (s == "None") {
        return reduction(reduction_type::reduce, loc);
      } else if (s == "Truncate") {
        return reduction(reduction_type::truncate, reduce_as, loc);
      } else if (s == "Erase") {
        had_reduce = true;
        return reduction(reduction_type::erase, loc);
      } else if (s == "SetType") {
        args.emplace_back(parse_term_expr(state, reduce_as), rewrite_action::none, 0);
        args.emplace_back(parse_term_expr(state, reduce_as), rewrite_action::none, 0);
        return reduction(reduction_type::change_type, static_cast<reduction_args &&>(args), reduce_as, 0, loc);
      } else {
        state.fail("Unsupported reduction type", s);
      }
    }

    had_reduce = true;
    --state.cursor;
    const char * begin = state.cursor;
    if (is_upper(*state.cursor)) {
      auto name = parse_name(state);
      if (nt_p red_nt = (nonterminal*)state.mod->node_maybe_at(name, node_nonterminal)) {
        reduction_type type = reduction_type::reduce;
        if (state.cursor[0] == ':') {
          ++state.cursor;
          args.emplace_back(parse_term_expr(state, reduce_as));
          type = reduction_type::reduce_copy;
        } else if (state.cursor[0] == '(') {
          auto loc = state.loc();
          ++state.cursor;
          skip_ws(state);
          --state.cursor;
          i32_t i = 0, max = red_nt->num_fields();
          while (*state.cursor != ')') {
            if (*state.cursor == '_') {
              args.emplace_back(copy(red_nt->fields[i].initializer, state.loc()));
              ++state.cursor;
              skip_ws(state);
              --state.cursor;
            } else {
              args.emplace_back(parse_term_expr(state, reduce_as));
            }
            ++i;
          }
          state.check(i <= max, "Too many reduction args");
          for (; i < max; ++i) {
            state.check(red_nt->fields[i].initializer, "Too few reduction args");
            args.emplace_back(copy(red_nt->fields[i].initializer, loc));
          }
          ++state.cursor;
        }
        return reduction(type, static_cast<reduction_args &&>(args), red_nt, -1, loc);
      }
      state.cursor = begin;
    }

    i32_t offset = 0;
    bool is_ahead = false;
    te_type type(term_expr_type_kind::term_or_terms);
    while ((state.cursor[0] != '-' || state.cursor[1] != '>') && state.cursor[0] != ';' && state.cursor[0] != '{') {
      if (*state.cursor == '$' && !is_digit(state.cursor[1])) {
        state.check(!is_ahead, "Duplicate '$'", "in pat");
        is_ahead = true;
        ++state.cursor;
        skip_ws(state);
        --state.cursor;
        offset = 0;
        continue;
      }
      if (is_ahead) {
        --offset;
      }
      args.emplace_back(parse_term_expr(state, reduce_as, &type), rewrite_action::replace, offset);
      if (!is_ahead) {
        ++offset;
      }
    }
    // The size check avoids placing special reductions like `if(not($1) / $2 $3) -> if($1 / $3 $2)` in Any.
    nonterminal * nt = (reduce_as ? reduce_as : state.mod->nonterminals()[0]);
    return reduction(reduction_type::rewrite, static_cast<reduction_args &&>(args), nt, -1, state.loc());
  }

  void parse_reduction_group(base_parser_state & state, rule * J_NOT_NULL target, nt_p reduce_as) {
    reduction_group rg(reduce_as, state.loc());
    maybe_parse_reduction_group_opts(state, rg, reduce_as);
    bool had_reduce = false;
    if (skip_ws(state) == '{') {
      while (skip_ws(state) != '}') {
        --state.cursor;
        nt_p cur_reduce = rg.push_back(parse_reduction(state, reduce_as, had_reduce)).nt;
        if (cur_reduce) {
          rg.target_nt = cur_reduce;
        }
        skip_ws_past(state, ";");
      }
      if (!rg.target_nt) {
        rg.target_nt = reduce_as;
      }
    } else {
      --state.cursor;
      rg.target_nt = rg.push_back(parse_reduction(state, reduce_as, had_reduce)).nt;
    }
    if (!had_reduce) {
      rg.push_back(reduction(reduction_type::pop, reduce_as, -2));
    }
    target->push_reduction_group(static_cast<reduction_group &&>(rg));
  }

  void parse_rule_reductions(base_parser_state & state, rule * J_NOT_NULL target, nt_p reduce_as) {
    for (char ch = *state.cursor; ch != ';' && ch != '{'; ch = skip_ws_up_to(state, "-;{", "After rule reduction")) {
      state.check(state.cursor[0] == '-' && state.cursor[1] == '>', "Expected '->'", "in subrule.");
      state.cursor += 2;
      parse_reduction_group(state, target, reduce_as);
    }
  }

  void parse_pattern_context(base_parser_state & state, rule * J_NOT_NULL target) {
    for (;;) {
      skip_ws(state);
      --state.cursor;
      auto kwd = parse_name(state, "reading pattern context keyword");
      if (kwd == "In") {
        state.check(!target->has_parent, "Duplicate 'In'", "in rule context");
        target->has_parent = true;
        target->parent_terms = parse_term_set(state);
      } else if (kwd == "Debug") {
        target->debug = true;
      } else if (kwd == "Name") {
        target->name = parse_name(state, "reading pattern name");
      } else if (kwd == "Precedence") {
        target->precedence = parse_int(state);
      } else if (kwd == "NotAtExit") {
        target->not_at_exit = true;
      } else {
        state.fail("Expected 'In', 'Debug', or 'NotAtExit'", "in rule context");
      }
      if (skip_ws_past(state, "],") == ']') {
        break;
      }
    }
  }

  // J_A(NODISC,RNN) pat_tree * parse_pat_tree_front(base_parser_state & state, pat_builder && builder) {
  //   i32_t offset = -1;
  //   bool has_ended = false;
  //   while (*state.cursor != '-') {
  //     switch (*state.cursor++) {
  //     case '$':
  //       builder.push_select_term_ahead(parse_int(state) - 1, offset);
  //       [[fallthrough]];
  //     case '_':
  //       state.check(!has_ended, "Unexpected '$' or '_'", "in pat");
  //       --offset;
  //       break;
  //     case '\r': case '\n': state.newline(); break;
  //     case ')': has_ended = true; break;
  //     case ' ': break;
  //     default: state.fail("Unexpeceted character after //", "in pat");
  //     }
  //   }
  //   return builder.build();
  // }

  J_A(NODISC) pat_tree * parse_pat_tree(base_parser_state & state, pat_builder & builder, rule * J_NOT_NULL target) {
    // for (;;) {
      char ch = skip_ws(state);
      if (ch != '*' && ch != '+') {
        --state.cursor;
        builder.visit(parse_pat_terms(state));
        return builder.build();
      }
        const bool has_epsilon = ch == '*';
        nt_p reduce_nt = state.nt_at(parse_name(state, "when parsing pattern nonterminal"));
        if (has_epsilon) {
          reduction_args args;
          auto loc = state.loc();
          if (skip_ws_past(state, "({") == '(') {
            loc = state.loc();
            while (*state.cursor != ')') {
              args.emplace_back(parse_term_expr(state, reduce_nt));
            }
            ++state.cursor;
            skip_ws_past(state, "{", "after LeftRecurse");
          }
          i32_t i = args.size(), max = reduce_nt->num_fields();
          state.check(i <= max, "Too many reduction args");
          for (; i < max; ++i) {
            state.check(reduce_nt->fields[i].initializer, "Too few reduction args");
            args.emplace_back(copy(reduce_nt->fields[i].initializer, loc));
          }
          rule * epsilon_rule = ::new rule(make_empty_pat_tree(), pat_layer::inner, builder.precedence, target->debug, state.loc());
          reduction_group rg(reduce_nt, state.loc());
          rg.push_back(reduction(reduction_type::reduce, static_cast<reduction_args &&>(args), reduce_nt, 0, state.loc()));
          epsilon_rule->push_reduction_group(static_cast<reduction_group &&>(rg));
          state.mod->rules.push_back(epsilon_rule);
        } else {
          skip_ws_past(state, "{", "after LeftRecurse");
        }
        i32_t subindex = 0;
        while (char c = skip_ws(state)) {
          if (c ==  '}') {
            break;
          }
          pat_builder sub_builder(*state.mod, pat_layer::inner, builder.precedence, subindex++);
          pat_term_p root = j::mem::make_shared<pat_term>(reduce_nt, 0);

          rule * aux_rule = ::new rule(state.loc());
          aux_rule->layer = pat_layer::inner;
          aux_rule->precedence = builder.precedence;
          aux_rule->debug = target->debug;

          if (c == '[') {
            parse_pattern_context(state, aux_rule);
            skip_ws(state);
          }
          --state.cursor;
          J_ASSERT(!aux_rule->has_parent, "'In' is not supported for repeated rules.");

          parse_pat_terms(state, &root->next);
          J_ASSERT(root->next);
          sub_builder.visit(*root);
          aux_rule->match = sub_builder.build(); //parse_pat_tree(state, sub_builder, target);
          parse_rule_reductions(state, aux_rule, reduce_nt);
          state.mod->rules.push_back(aux_rule);
          ++state.cursor;
        }
        return nullptr;


      // switch (ch) {
      // case '-': case ';':
      // case '*': case '+': {
      //   break;
      // }
      // default:
      //   builder.visit(parse_pat_terms(state));
      //   --state.cursor;
      //   break;
      // }
    // }
  }


    J_A(NODISC,AI) inline pat_tree * parse_pat_tree(base_parser_state & state, i8_t precedence, i32_t index, rule * J_NOT_NULL target) {
      pat_builder builder(*state.mod, pat_layer::main, precedence, index);
      return parse_pat_tree(state, builder, target);
    }
  }

  J_A(NODISC) rule * parse_rule(base_parser_state & state, i32_t index, nt_p reduce_as) {
    rule * result = ::new rule(state.loc());
    if (skip_ws(state) == '[') {
      parse_pattern_context(state, result);
      skip_ws(state);
    }
    --state.cursor;
    result->match = parse_pat_tree(state, result->precedence, index, result);
    if (!result->match) {
      ::delete result;
      return nullptr;
    }
    skip_ws(state);
    --state.cursor;
    parse_rule_reductions(state, result, reduce_as);
    return result;
  }

  namespace {
    struct te_op final {
      operator_type op = operator_type::none;
      term_expr * te;
    };

    J_A(RNN,NODISC) inline term_expr * reduce_te_binary(const te_op & op, term_expr * J_AA(NN,NOALIAS) lhs,
                                                        term_expr * J_AA(NN,NOALIAS) rhs) noexcept {
      op.te->binop.lhs = lhs;
      op.te->binop.rhs = rhs;
      return op.te;
    }

    J_A(RNN,NODISC) inline term_expr * reduce_te_unary(const te_op & op, term_expr * J_AA(NN) lhs) {
      if (op.op == operator_type::fn_call) {
        op.te->member_fn_call.val = lhs;
      } else {
        J_ASSERT(op.op == operator_type::prop_get);
        op.te->prop_read.val = lhs;
      }
      return op.te;
    }

    J_A(RNN,AI,NODISC) inline const p::operator_info * get_te_op_info(const te_op & op) noexcept {
      return &operator_info[(u8_t)op.op];
    }

    using precedence_parser = p::precedence_parser<term_expr*, te_op, get_te_op_info, reduce_te_binary, reduce_te_unary>;

    J_A(RNN) term_expr * push_expr(base_parser_state & state, precedence_parser & prec, term_expr * J_NOT_NULL te, const source_location * loc = nullptr) {
      te->loc = loc ? *loc : (te->type == term_expr_type::constant && te->constant.init_expr && te->constant.init_expr->loc) ? te->constant.init_expr->loc :  state.loc();
      try {
        prec.push_expr(te);
      } catch (j::exceptions::exception &) {
        state.fail("Pushing expr failed", dump_str(state.mod, te));
      }
      return te;
    }

    J_A(RNN) inline term_expr * push_op(base_parser_state & state, precedence_parser & prec, operator_type t, term_expr * J_NOT_NULL te, const source_location * loc = nullptr) {
      te->loc = loc ? *loc : state.loc();
      try {
        prec.push_op({ t, te });
      } catch (j::exceptions::exception &) {
        state.fail("Pushing op failed", dump_str(state.mod, te));
      }
      return te;
    }

    template<typename... Ts>
    J_A(RNN) inline term_expr * emplace_expr(const source_location & loc, base_parser_state & state, precedence_parser & prec, Ts && ... args) {
      return push_expr(state, prec, ::new term_expr(static_cast<Ts &&>(args)...), &loc);
    }

    template<typename... Ts>
    J_A(RNN,AI,ND) inline term_expr * emplace_expr(base_parser_state & state, precedence_parser & prec, Ts && ... args) {
      return emplace_expr(state.loc(), state, prec, static_cast<Ts &&>(args)...);
    }

    template<typename... Ts>
    J_A(RNN) inline term_expr * emplace_op(const source_location & loc, base_parser_state & state, precedence_parser & prec, operator_type t, Ts && ... args) {
      return push_op(state, prec, t, ::new term_expr(static_cast<Ts &&>(args)...), &loc);
    }


    template<typename... Ts>
    J_A(RNN,AI,ND) inline term_expr * emplace_op(base_parser_state & state, precedence_parser & prec, operator_type t, Ts && ... args) {
      return emplace_op(state.loc(), state, prec, t, static_cast<Ts &&>(args)...);
    }

  }

  J_A(RNN,NODISC) term_expr * parse_term_expr(base_parser_state & state, nt_p this_nt, te_type * parse_as) {
    precedence_parser prec;
    using enum operator_type;
    source_location loc;
    for (;;) {
      const bool after_expr = prec.accepts_infix();
      char ch = skip_ws(state);
      if (!loc) {
        loc = state.loc();
      }
      --loc.column;
      switch (ch) {
      case '$': {
        char ch2 = state.cursor[0];
        if (after_expr) {
          switch (ch2) {
          case ' ': case '\t': case '\r': case '\n': case ')': case '(': case '\0': case '#':
            goto out;
          default:
            if (is_digit(ch2)) {
              goto out;
            }
          }
          --state.cursor;
          const operator_type op_type = parse_te_operator(state);
          emplace_op(loc, state, prec, op_type, te_binop{op_type});
          break;
        }
        if (!is_digit(ch2)) {
          goto out;
        }
        emplace_expr(loc, state, prec, reduction_arg(parse_int(state) - 1));
        break;
      }
      case '-':
        if (*state.cursor == '>') {
          goto out;
        } else if (is_digit(*state.cursor)) {
          if (after_expr) {
            goto out;
          }
          --state.cursor;
          goto parse_const;
        }
        [[fallthrough]];
      case '<': case '>':
      case '/': case '|': case '&': case '+': case '*': case '=': case '!': case '^': {
        if (!after_expr) {
          goto out;
        }
        --state.cursor;
        const operator_type op_type = parse_te_operator(state);
        // if (!is_non_assign_operator(op_type)) {
        //   state.cursor = before;
        //   goto out;
        // }
        emplace_op(loc, state, prec, op_type, te_binop{op_type});
        break;
      }
      case '(':
        if (after_expr) {
          goto out;
        }
        push_expr(state, prec, parse_term_expr(state, this_nt, parse_as));
        skip_ws_past(state, ")", "in term expr");
        break;
      case '~': case ')': case ';': case '[': case ']':
        goto out;
      case '.': {
        if (!after_expr) {
          goto out;
        }
        auto n = parse_name_or_id(state);
        if (*state.cursor == '(') {
          parse_te_args_out(state, emplace_op(loc, state, prec, fn_call, te_member_fn_call{nullptr, n})->member_fn_call.args, this_nt);
        } else {
          emplace_op(loc, state, prec, prop_get, te_prop_read{nullptr, n});
        }
        break;
      }
      default: {
        if (after_expr) {
          goto out;
        }
        --state.cursor;
        const char * begin = state.cursor;
        if (is_lower(ch)) {
          auto id = parse_id(state, "parsing term id");
          if (term_expr * te = state.term_expr_macro_maybe_at(id)) {
            push_expr(state, prec, te, &loc);
          } else {
            term * t = state.term_at(id);
            if (*state.cursor == '(') {
              parse_te_args_out(state, emplace_expr(loc, state, prec, te_term_construct{t})->term_construct.args, this_nt, t);
            } else {
              emplace_expr(state, prec, ::new expr(attr_value(t), loc));
            }
          }
        } else if (!is_upper(ch)) {
          goto parse_const;
        } else {
          s::string name = parse_name(state);
          if (*state.cursor != '(') {
            auto f = this_nt ? this_nt->field_maybe_at(name) : nullptr;
            if (f) {
              emplace_expr(loc, state, prec, te_prop_read{
                  ::new term_expr(reduction_arg(-1)),
                  name,
                });
            } else if (auto v = state.mod->maybe_get_val_const(name)) {
              emplace_expr(loc, state, prec, attr_value(v->value), v->type);
            } else {
              state.cursor = begin;
              goto parse_const;
            }
          } else {
            ++state.cursor;
            if (name == "If") {
              term_expr * condition    = parse_term_expr(state, this_nt);
              term_expr * true_branch  = parse_term_expr(state, this_nt, parse_as);
              term_expr * false_branch = parse_term_expr(state, this_nt, parse_as);
              skip_ws_past(state, ")");
              emplace_expr(loc, state, prec, te_ternary{condition, true_branch, false_branch});
            } else if (name == "Construct") {
              parse_te_args(state, emplace_expr(loc, state, prec, te_term_construct{parse_term_expr(state, this_nt)})->term_construct.args, this_nt);
            } else if (state.mod->te_functions.contains(name)) {
              parse_te_args(state, emplace_expr(loc, state, prec, te_global_fn_call{name})->global_fn_call.args, this_nt);
            } else {
              state.cursor = begin;
              goto parse_const;
            }
          }
        }
        break;
      }
      }
      continue;

      parse_const:
      {
        term_expr * e = ::new term_expr(::new expr(parse_attr_value(state, parse_as && parse_as->is_attr_value() ? &parse_as->attr_value_type : nullptr)));
        // When expecting a term value, decode constants as ld_consts.
        if (parse_as && parse_as->is_term_or_terms()) {
          term_exprs args(1);
          args.push_back(e);
          e = ::new term_expr(te_term_construct(state.mod->trs_const_term, static_cast<term_exprs &&>(args)));
        }
        push_expr(state, prec, e, &loc);
      }
    }

  out:
    --state.cursor;
    try {
      return prec.finish();
    } catch (exceptions::exception & e) {
      state.fail("Parse error", "after expr");
    }
  }
}

#pragma clang diagnostic pop
