#include "meta/expr.hpp"
#include "algo/quicksort.hpp"
#include "parsing/precedence_parser.hpp"
#include "logging/global.hpp"
#include "meta/attr_value_hash.hpp"
#include "containers/hash_map.hpp"
#include "meta/expr_scope.hpp"
#include "meta/attr_value_iteration.hpp"
#include "meta/expr_fn_def.hpp"
#include "meta/term.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/module.hpp"
#include "meta/attr_value_hash.hpp"
#include "strings/formatting/pad.hpp"
#include "meta/attr_context.hpp"
#include "meta/dump.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/format.hpp"
#include "meta/attr_value_interfaces.hpp"

namespace j::meta {
  namespace s = strings;
  namespace p = parsing;
  namespace st = strings::styling::styles;
  namespace {
    J_A(AI,ND,NODISC) inline u8_t expr_precedence(expr_type t) noexcept {
      return exprs[t].precedence;
    }

    void dump_call(dump_context & ctx, const expr & e) {
      if (e.type == expr_method_call) {
        u8_t prec = ctx.set_precedence(expr_precedence(e.type));
        dump(ctx, *e.call.args[0]);
        ctx.to.write(".", st::bright_yellow.with_bold());
        ctx.set_precedence(prec);
      }

      u8_t prec = ctx.enter_call_like();
      ctx.to.write(e.name, st::bright_green);
      ctx.to.write("(", st::bright_yellow.with_bold());
      bool is_first = true;
      for (u32_t i = e.type == expr_method_call ? 1U : 0U; i < e.call.size; ++i) {
        if (!is_first) {
          ctx.to.write(", ", st::bold);
        }
        is_first = false;
        dump(ctx, *e.call.args[i]);
      }
      ctx.to.write(")", st::bright_yellow.with_bold());
      ctx.exit_call_like(prec);
    }

    template<typename T>
    void wr(dump_context & ctx, bool & is_first, const T & val) {
      if (!is_first) {
        ctx.to.write(", ");
      }
      is_first = false;
      if constexpr (is_same_v<const char *, T> || is_same_v<s::const_string_view, T> || is_same_v<s::string, T>) {
        ctx.to.write(s::const_string_view(val), st::magenta);
      } else {
        dump(ctx, val);
      }
    }

    template<typename... Args>
    void dump_call_like(dump_context & ctx, const char * J_NOT_NULL name, s::style st, const Args & ... args) {
      u8_t prec = ctx.enter_call_like();
      st = st.with_bold();
      ctx.to.write(name, st);
      ctx.to.write("(", st);
      bool is_first = true;
      (wr(ctx, is_first, args), ...);
      ctx.to.write(")", st);
      ctx.exit_call_like(prec);
    }

    void dump_infix(dump_context & ctx, const expr & e, const char * J_NOT_NULL op) {
      u8_t prec = ctx.enter_precedence(expr_precedence(e.type));
      dump(ctx, *e.ops[0]);
      ctx.to.write(op);
      dump(ctx, *e.ops[1]);
      ctx.exit_precedence(prec);
    }

    void dump_comprehension(dump_context & ctx, const expr & e, const char * J_NOT_NULL fn_name) {
      u8_t prec = ctx.set_precedence(expr_precedence(e.type));
      dump(ctx, *e.ops[0]);
      ctx.to.write(".", st::bright_magenta.with_bold());
      ctx.to.write(fn_name, st::bright_magenta);
      ctx.to.write("[", st::bright_magenta.with_bold());
      ctx.enter_call_like();
      if (e.name) {
        ctx.to.write(e.name, st::bright_green.with_bold());
      }
      if (e.idx_name) {
        if (e.name) {
          ctx.to.write(", ");
        }
        ctx.to.write(e.idx_name, st::green.with_bold());
      }
      if (e.name || e.idx_name) {
        ctx.to.write(": ", st::bright_green.with_bold());
      }
      dump(ctx, *e.ops[1]);
      if (e.ops[2]) {
        ctx.to.write(", ", st::bright_green.with_bold());
        dump(ctx, *e.ops[2]);
      }
      ctx.to.write("]", st::bright_magenta.with_bold());
      ctx.exit_call_like(prec);
    }

    void dump_prefix(dump_context & ctx, const expr & e, const char * J_NOT_NULL op, s::style s) {
      u8_t prec = ctx.enter_precedence(expr_precedence(e.type));
      ctx.to.write(op, s.with_bold());
      dump(ctx, *e.ops[0]);
      ctx.exit_precedence(prec);
    }

    void dump_access(dump_context & ctx, const expr & e) {
      u8_t prec = ctx.set_precedence(expr_precedence(e.type));
      dump(ctx, *e.ops[0]);
      ctx.set_precedence(prec);
      ctx.to.write(e.type == expr_maybe_at_prop ? ".?" : ".", e.type == expr_maybe_at_prop ? st::bright_green.with_bold() : st::green);
      ctx.to.write(e.name, st::bright_cyan);
    }

#define WR(A, ...) ctx.to.write(A __VA_OPT__(, st::__VA_ARGS__))
#define WF(A, ...) ctx.indent.write_formatted(ctx.to, A __VA_OPT__(, __VA_ARGS__))
#define D0 dump(ctx, *e.ops[0])
#define D1 dump(ctx, *e.ops[1])
#define D2 dump(ctx, *e.ops[2])
#define DUMP(...) [](dump_context & ctx, [[maybe_unused]] const expr & e) { __VA_ARGS__ }
#define INFIX(S) DUMP(dump_infix(ctx, e, S);)
#define PREFIX(S, ST) DUMP(dump_prefix(ctx, e, S, st::ST);)
#define CALL_LIKE(S, ST, ...) DUMP(dump_call_like(ctx, S, st::ST __VA_OPT__(, __VA_ARGS__));)
#define J_EXPR(N, T, F, P, ...) [expr_##N] = {{P, p::T, #N}, form_##F, __VA_ARGS__ }
#define J_PRE(N, P, ...) J_EXPR(N, prefix, unary, P __VA_OPT__(, __VA_ARGS__))
#define J_POSTFIX(N, P, ...) J_EXPR(N, postfix, unary, P __VA_OPT__(, __VA_ARGS__))
#define J_BINARY(N, P, ...) J_EXPR(N, infix, binary, P __VA_OPT__(, __VA_ARGS__))
#define J_COMPREHENSION(N, P, ...) J_EXPR(N, postfix, binary, P __VA_OPT__(, __VA_ARGS__))
#define J_COMPREHENSION3(N, P, ...) J_EXPR(N, postfix, trinary, P __VA_OPT__(, __VA_ARGS__))
#define J_TRINARY(N, P, ...) J_EXPR(N, infix, trinary, P __VA_OPT__(, __VA_ARGS__))

  }

  constinit const expr_info exprs[num_expr_types_v]{
    J_EXPR(none, infix, none, 0U, DUMP(WR("None", error);)),
    J_EXPR(var, infix, var, 255U, DUMP(WR(e.name, bright_yellow.with_bold());)),
    J_EXPR(const, infix, const, 255U, DUMP(dump(ctx, e.const_val);)),
    J_EXPR(call, infix, call, 255U, DUMP(dump_call(ctx, e);)),
    J_EXPR(method_call, postfix, call, 20U, DUMP(dump_call(ctx, e);)),

    J_POSTFIX(at_prop, 20, DUMP(dump_access(ctx, e);)),
    J_POSTFIX(maybe_at_prop, 20, DUMP(dump_access(ctx, e);)),
    J_POSTFIX(has, 20, CALL_LIKE("Has", magenta, *e.ops[0], e.name)),

    J_PRE(is_none, 20, CALL_LIKE("IsNone", bright_yellow)),
    J_PRE(not,   6, PREFIX("!", bright_red)),
    J_PRE(neg,   6, PREFIX("-", bright_yellow)),
    J_PRE(bnot,  6, PREFIX("~", bright_red)),

    J_COMPREHENSION(map,        20, DUMP(dump_comprehension(ctx, e, "Map");)),
    J_COMPREHENSION3(reduce,    20, DUMP(dump_comprehension(ctx, e, "Reduce");)),
    J_COMPREHENSION(filter,     20, DUMP(dump_comprehension(ctx, e, "Filter");)),
    J_COMPREHENSION(find,       20, DUMP(dump_comprehension(ctx, e, "Find");)),
    J_COMPREHENSION(find_index, 20, DUMP(dump_comprehension(ctx, e, "FindIndex");)),
    J_COMPREHENSION(group_by,   20, DUMP(dump_comprehension(ctx, e, "GroupBy");)),
    J_COMPREHENSION(sort_by,    20, DUMP(dump_comprehension(ctx, e, "SortBy");)),
    J_COMPREHENSION(some,       20, DUMP(dump_comprehension(ctx, e, "Some");)),
    J_COMPREHENSION(every,      20, DUMP(dump_comprehension(ctx, e, "Every");)),

    J_BINARY(or,   1, INFIX(" || ")),
    J_BINARY(and,  2, INFIX(" && ")),

    J_BINARY(eq,   3, INFIX(" == ")),
    J_BINARY(neq,  3, INFIX(" != ")),
    J_BINARY(lt,   3, INFIX(" < ")),
    J_BINARY(le,  3, INFIX(" <= ")),
    J_BINARY(gt,   3, INFIX(" > ")),
    J_BINARY(ge,  3, INFIX(" >= ")),

    J_BINARY(bor,  4, INFIX(" | ")),
    J_BINARY(sub,  4, INFIX(" - ")),
    J_BINARY(add,  4, INFIX(" + ")),

    J_BINARY(band, 5, INFIX(" & ")),

    J_BINARY(mul,  5, INFIX(" * ")),
    J_BINARY(div,  5, INFIX(" / ")),

    J_TRINARY(if, 255, CALL_LIKE("If", bright_cyan, *e.ops[0], *e.ops[1], *e.ops[2])),
  };

  static void copy_expr(expr * J_AA(NOALIAS,NN) to, const expr & J_AA(NOALIAS) rhs) {
    to->type = rhs.type;
    to->name = rhs.name;
    to->loc = rhs.loc;
    to->idx_name = rhs.idx_name;
    switch (exprs[rhs.type].form) {
    case form_none:
      break;
    case form_const:
      ::new (&to->const_val) attr_value(rhs.const_val);
      break;
    case form_var:
      break;
    case form_call:
      to->call.size = rhs.call.size;
      to->call.args = ::new expr*[rhs.call.size];
      for (u32_t i = 0U; i < rhs.call.size; ++i) {
        to->call.args[i] = ::new expr(*rhs.call.args[i]);
        to->call.args[i]->loc = rhs.call.args[i]->loc;
      }
      break;
    case form_trinary:
      to->ops[2] = rhs.ops[2] ? ::new expr(*rhs.ops[2]) : nullptr;
      if (rhs.ops[2]) {
        to->ops[2]->loc = rhs.ops[2]->loc;
      }
      [[fallthrough]];
    case form_binary:
      to->ops[1] = rhs.ops[1] ? ::new expr(*rhs.ops[1]) : nullptr;
      if (rhs.ops[1]) {
        to->ops[1]->loc = rhs.ops[1]->loc;
      }
      [[fallthrough]];
    case form_unary:
      to->ops[0] = rhs.ops[0] ? ::new expr(*rhs.ops[0]) : nullptr;
      if (rhs.ops[0]) {
        to->ops[0]->loc = rhs.ops[0]->loc;
      }
      break;
    }
  }

  expr::expr(attr_value && const_val, source_location loc) noexcept
    : type(expr_const),
      const_val{static_cast<attr_value &&>(const_val)},
      loc(loc)
    { }

  expr::expr(const expr & rhs) noexcept {
    copy_expr(this, rhs);
  }
  expr & expr::operator=(const expr & rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      copy_expr(this, rhs);
    }
    return *this;
  }

  void expr::clear() noexcept {
    switch (exprs[type].form) {
    case form_none:
      break;
    case form_const:
      const_val.~attr_value();
      break;
    case form_var:
      break;
    case form_call:
      for (u32_t i = 0U; i < call.size; ++i) {
        ::delete call.args[i];
      }
      ::delete[] call.args;
      break;
    case form_trinary:
      ::delete ops[2];
      [[fallthrough]];
    case form_binary:
      ::delete ops[1];
      [[fallthrough]];
    case form_unary:
      ::delete ops[0];
      break;
    }
    type = expr_none;
    name.clear();
    idx_name.clear();
    ops[0] = ops[1] = ops[2] = nullptr;
  }

  expr::expr(strings::const_string_view var_name, source_location loc) noexcept
    : type(expr_var),
      name(var_name),
      loc(loc)
  {
  }

  expr::expr(strings::const_string_view fn_name, span<expr*> args, source_location loc) noexcept
    : type(expr_call),
      name(fn_name),
      call{::new expr*[args.size()], args.size()},
      loc(loc)
  {
    ::j::memcpy(call.args, args.begin(), args.size() * J_PTR_SZ);
  }

  expr::expr(expr_type type, s::const_string_view it_name, s::const_string_view idx_name,
             expr * base, expr * J_NOT_NULL body, expr * acc_init, source_location loc) noexcept
    : type(type),
      name(it_name),
      idx_name(idx_name),
      ops{base, body, acc_init},
      loc(loc)
  {
    J_ASSERT(exprs[type].form == form_binary || exprs[type].form == form_trinary);
    J_ASSERT((exprs[type].form == form_binary) == !acc_init);
  }


  expr::expr(strings::const_string_view fn_name, expr* this_arg, span<expr*> args, source_location loc) noexcept
    : type(expr_method_call),
      name(fn_name),
      call{::new expr*[args.size() + 1], args.size() + 1U},
      loc(loc)
  {
    call.args[0] = this_arg;
    ::j::memcpy(call.args + 1U, args.begin(), args.size() * J_PTR_SZ);
  }

  expr::expr(expr_type type, expr * base, strings::const_string_view at, source_location loc) noexcept
    : type(type),
      name(at),
      ops{base, nullptr, nullptr},
      loc(loc)
  {
    J_ASSERT(exprs[type].form == form_unary);
  }

  expr & expr::operator=(expr && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      ::j::memcpy(this, &rhs, sizeof(expr));
      ::new (&name) s::string(static_cast<s::string&&>(rhs.name));
      ::new (&idx_name) s::string(static_cast<s::string&&>(rhs.idx_name));
      rhs.type = expr_none;
    }
    return *this;
  }

  void dump(dump_context & ctx, const expr & value) noexcept {
    exprs[value.type].dump(ctx, value);
  }

  namespace {
    [[nodiscard]] attr_value eval_unary(expr_scope & scope, const expr * J_NOT_NULL e, const attr_value & val) {
      switch (e->type) {
      case expr_is_none:
        return attr_value(!val);
      case expr_not:
        return attr_value(val.value_empty());
      case expr_bnot:
        if (!val.is_int()) {
          scope.throw_expr_error("Expected int");
        }
        return attr_value(~val.as_int());
      case expr_neg:
        if (!val.is_int()) {
          scope.throw_expr_error("Expected int");
        }
        return attr_value(-val.as_int());
      default:
        J_FAIL("Unexpected unary expr {}", e->type);
      }
    }


    [[nodiscard]] attr_value eval_binary(const expr * J_NOT_NULL e, i64_t lhs, i64_t rhs) {
      switch (e->type) {
      case expr_bor:  return attr_value(lhs | rhs);
      case expr_band: return attr_value(lhs & rhs);
      case expr_sub:  return attr_value(lhs - rhs);
      case expr_add:  return attr_value(lhs + rhs);
      case expr_mul:  return attr_value(lhs * rhs);
      case expr_div:  return attr_value(lhs / rhs);
      case expr_lt:  return attr_value(lhs < rhs);
      case expr_le:  return attr_value(lhs <= rhs);
      case expr_gt:  return attr_value(lhs > rhs);
      case expr_ge:  return attr_value(lhs >= rhs);
      default:
        J_FAIL("Unexpected binary expr {}", e->type);
      }
    }

    [[nodiscard]] attr_value eval_binary(const expr * J_NOT_NULL e, u64_t lhs, u64_t rhs, const enum_def * J_NOT_NULL def) {
      J_REQUIRE(def->is_flag_enum, "Tried to perform arithmetic on a non-flag enum.");
      switch (e->type) {
      case expr_add:
      case expr_bor:  return attr_value(def, lhs | rhs);
      case expr_band: return attr_value(def, lhs & rhs);
      case expr_sub:  return attr_value(def, lhs & ~rhs);
      default:
        J_FAIL("Unexpected binary expr {} for enums.", e->type);
      }
    }

    [[nodiscard]] attr_value eval_binary(const expr * J_NOT_NULL e, const basic_node_set & lhs, const basic_node_set & rhs, attr_type type) {
      switch (e->type) {
      case expr_add:
      case expr_bor:  return attr_value(type, basic_node_set{lhs.bitmask | rhs.bitmask});
      case expr_band: return attr_value(type, basic_node_set{lhs.bitmask & rhs.bitmask});
      case expr_sub:  return attr_value(type, basic_node_set{lhs.bitmask - rhs.bitmask});
      default:
        J_FAIL("Unexpected binary expr {} for enums.", e->type);
      }
    }

    [[nodiscard]] attr_value eval_binary(expr_scope & scope, const expr * J_NOT_NULL e, attr_value lhs, attr_value rhs) {
      switch (e->type) {
      case expr_eq:
        return attr_value(lhs == rhs);
      case expr_neq:
        return attr_value(lhs != rhs);
      case expr_add:
        if (lhs.is_str() || rhs.is_str()) {
          return attr_value(arg_parser<s::string>::parse(scope, lhs) + arg_parser<s::string>::parse(scope, rhs));
        }
        [[fallthrough]];
      case expr_bor:
      case expr_band:
      case expr_sub:
        if (lhs.is_enum() || rhs.is_enum()) {
          J_REQUIRE(lhs.is_enum() && rhs.is_enum() && lhs.enum_val.def == rhs.enum_val.def,
                    "Enum arithmetic can only be performed among enums of the same type.");
          return eval_binary(e, lhs.enum_val.value, rhs.enum_val.value, lhs.enum_val.def);
        }
        if (lhs.is_node_set() || rhs.is_node_set()) {
          J_REQUIRE(lhs.type == rhs.type, "Both arguments must be node sets of same type.");
          return eval_binary(e, lhs.node_set, rhs.node_set, lhs.type);
        }
        [[fallthrough]];
      case expr_lt: case expr_le:
      case expr_gt: case expr_ge:
      case expr_mul: case expr_div:
        if (!lhs.is_int() || !rhs.is_int()) {
          scope.throw_expr_error("Expected int");
        }
        return eval_binary(e, lhs.as_int(), rhs.as_int());
      default:
        J_FAIL("Unexpected binary expr {}", e->type);
      }
    }

    [[nodiscard]] attr_value eval_map(expr_scope & scope, const expr * J_NOT_NULL e) {
      auto sub = arg_parser<span<const attr_value>>::parse(scope, eval_expr(scope, e->ops[0]));
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      auto & idx_var = ch[e->idx_name ? e->idx_name : "ItIndex"];
      i64_t i = 0;
      for (auto & s : sub) {
        var = s;
        idx_var = attr_value(i);
        s = eval_expr(ch, e->ops[1]);
        ++i;
      }
      return attr_value(sub);
    }

    [[nodiscard]] attr_value eval_find(expr_scope & scope, const expr * J_NOT_NULL e, bool is_index) {
      auto sub = arg_parser<span<const attr_value>>::parse(scope, eval_expr(scope, e->ops[0]));
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      auto & idx_var = ch[e->idx_name ? e->idx_name : "ItIndex"];
      i64_t i = 0;
      for (auto & s : sub) {
        var = s;
        idx_var = attr_value(i);
        if (!eval_expr(ch, e->ops[1]).value_empty()) {
          return is_index ? static_cast<attr_value &&>(idx_var) : static_cast<attr_value &&>(var);
        }
        ++i;
      }
      return attr_value();
    }

    [[nodiscard]] attr_value eval_reduce(expr_scope & scope, const expr * J_NOT_NULL e) {
      auto sub = arg_parser<span<const attr_value>>::parse(scope, eval_expr(scope, e->ops[0]));
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & acc = ch["Acc"];
      acc = eval_expr(scope, e->ops[2]);
      auto & var = ch[e->name ? e->name : "It"];
      auto & idx_var = ch[e->idx_name ? e->idx_name : "ItIndex"];
      i64_t i = 0;
      for (auto & s : sub) {
        var = s;
        idx_var = attr_value(i++);
        acc = eval_expr(ch, e->ops[1]);
      }
      return attr_value(static_cast<attr_value &&>(acc));
    }

    J_A(AI) inline static const attr_value & get_sort_key(const pair<attr_value, attr_value> & p) noexcept {
      return p.first;
    }

    [[nodiscard]] attr_value eval_sort_by(expr_scope & scope, const expr * J_NOT_NULL e) {
      auto sub = arg_parser<span<const attr_value>>::parse(scope, eval_expr(scope, e->ops[0]));
      const i32_t sz = sub.size();
      pair<attr_value, attr_value> pairs[sz];
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      auto & idx_var = ch[e->idx_name ? e->idx_name : "ItIndex"];
      i64_t i = 0;
      for (auto & s : sub) {
        var = s;
        idx_var = attr_value(i);
        ::new (pairs + i) pair<attr_value, attr_value>{eval_expr(ch, e->ops[1]), static_cast<attr_value &&>(s)};
        ++i;
      }
      algo::quicksort(pairs, pairs + sz, get_sort_key);
      auto from = pairs;
      for (auto & s : sub) {
        s = static_cast<attr_value &&>(from++->second);
      }
      return attr_value(sub);
    }

    [[nodiscard]] attr_value eval_filter(expr_scope & scope, const expr * J_NOT_NULL e, attr_value && sub) {
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      for (auto & s : sub.node_set.iterate_plain(scope.root->mod, node_set_type(sub.type))) {
        var = attr_value(&s);
        if (eval_expr(ch, e->ops[1]).value_empty()) {
          sub.node_set.del(s);
        }
      }
      return static_cast<attr_value &&>(sub);
    }

    [[nodiscard]] attr_value eval_filter(expr_scope & scope, const expr * J_NOT_NULL e, trivial_array_copyable<attr_value> * J_NOT_NULL sub) {
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto wr = sub->begin();
      auto & var = ch[e->name ? e->name : "It"];
      auto & idx_var = ch[e->idx_name ? e->idx_name : "ItIndex"];
      i64_t i = 0;
      for (auto & s : *sub) {
        var = s;
        idx_var = attr_value(i);
        if (!eval_expr(ch, e->ops[1]).value_empty()) {
          *wr++ = s;
        }
        ++i;
      }
      return attr_value(span(sub->begin(), wr));
    }

    [[nodiscard]] attr_value eval_group_by(expr_scope & scope, const expr * J_NOT_NULL e, const attr_value & sub) {
      hash_map<attr_value, basic_node_set, attr_value_hash> groups;
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      for (auto & s : sub.node_set.iterate_plain(scope.root->mod, node_set_type(sub.type))) {
        var = attr_value(&s);
        groups[eval_expr(ch, e->ops[1])].add(s);
      }
      trivial_array_copyable<attr_value> result(containers::uninitialized, groups.size());
      for (auto & p : groups) {
        trivial_array_copyable<attr_value> pair{containers::uninitialized, 2};
        pair.initialize_element(p.first);
        pair.initialize_element(sub.type, p.second);
        result.initialize_element(static_cast<trivial_array_copyable<attr_value> &&>(pair));
      }
      return attr_value(static_cast<trivial_array_copyable<attr_value> &&>(result));
    }

    [[nodiscard]] attr_value eval_group_by(expr_scope & scope, const expr * J_NOT_NULL e, const trivial_array_copyable<attr_value> & sub) {
      hash_map<attr_value, vector<attr_value>, attr_value_hash> groups;
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      for (auto & s : sub) {
        var = s;
        groups[eval_expr(ch, e->ops[1])].push_back(s);
      }
      trivial_array_copyable<attr_value> result(containers::uninitialized, groups.size());
      for (auto & p : groups) {
        trivial_array_copyable<attr_value> pair{containers::uninitialized, 2};
        pair.initialize_element(p.first);
        pair.initialize_element(span(p.second.begin(), p.second.size()));
        result.initialize_element(static_cast<trivial_array_copyable<attr_value> &&>(pair));
      }
      return attr_value(static_cast<trivial_array_copyable<attr_value> &&>(result));
    }

    [[nodiscard]] attr_value eval_quantifier(expr_scope & scope, const expr * J_NOT_NULL e, bool every) {
      bool had_every = true, had_some = false;
      expr_scope ch{&scope};
      auto g = ch.enter_expr(e);
      auto & var = ch[e->name ? e->name : "It"];
      auto sub = arg_parser<span<const attr_value>>::parse(scope, eval_expr(scope, e->ops[0]));
      for (auto & s : sub) {
        var = s;
        bool r = !eval_expr(ch, e->ops[1]).value_empty();
        had_every &= r;
        had_some |= r;
      }
      return attr_value(every ? had_every : had_some);
    }
  }

  [[nodiscard]] attr_value eval_expr(expr_scope & scope, const expr * J_NOT_NULL e) {
    auto guard = scope.enter_expr(e);
    switch (e->type) {
    case expr_none:
      J_THROW("Tried to evaluate an empty expression.");
    case expr_const:
      return e->const_val;
    case expr_var:
      return scope.get(e->name);
    case expr_method_call:
    case expr_call: {
      i32_t sz = e->call.size;
      attr_value values[sz];
      for (i32_t i = 0; i < sz; ++i) {
        ::new (values + i) attr_value(eval_expr(scope, e->call.args[i]));
      }
      return e->type == expr_method_call
        ? interface_of(values[0]).call(scope, e->name, span(values, sz))
        : scope.get_fn(e->name, values, e->call.size)(scope, values, sz);
    }
    case expr_sort_by:
      return eval_sort_by(scope, e);
    case expr_map:
      return eval_map(scope, e);
    case expr_find:
    case expr_find_index:
      return eval_find(scope, e, e->type == expr_find_index);
    case expr_reduce:
      return eval_reduce(scope, e);
    case expr_some:
    case expr_every:
      return eval_quantifier(scope, e, e->type == expr_every);
    case expr_filter: {
      auto subject = eval_expr(scope, e->ops[0]);
      return subject.is_tuple()
        ? eval_filter(scope, e, subject.tuple)
        : eval_filter(scope, e, static_cast<attr_value &&>(subject));
    }
    case expr_group_by: {
      auto subject = eval_expr(scope, e->ops[0]);
      return subject.is_tuple()
        ? eval_group_by(scope, e, *subject.tuple)
        : eval_group_by(scope, e, subject);
    }
    case expr_at_prop:
    case expr_maybe_at_prop:
    case expr_has: {
      auto sub = eval_expr(scope, e->ops[0]);
      if (!sub) {
        if (e->type == expr_has) {
          return attr_value(false);
        } else if (e->type == expr_maybe_at_prop) {
          return {};
        } else {
          scope.throw_prop_not_found_error(e->name, sub);
        }
      }
      auto & iface = interface_of(sub);
      bool has = e->type == expr_at_prop || iface.has_prop(scope, sub, e->name);
      if (e->type == expr_has) {
        return attr_value(has);
      } else if (e->type == expr_at_prop) {
        if (!has) {
          scope.throw_prop_not_found_error(e->name, sub);
        }
      }
      return has ? iface.get_prop(scope, sub, e->name) : attr_value();
    }
    case expr_if:
      return eval_expr(scope, e->ops[1 + eval_expr(scope, e->ops[0]).value_empty()]);
    case expr_is_none:
    case expr_not:
    case expr_bnot:
    case expr_neg:
      return eval_unary(scope, e, eval_expr(scope, e->ops[0]));
    case expr_or:
    case expr_and: {
      attr_value lhs = eval_expr(scope, e->ops[0]);
      if ((e->type == expr_and) == lhs.value_empty()) {
        return static_cast<attr_value &&>(lhs);
      }
      return eval_expr(scope, e->ops[1]);
    }
    case expr_bor:
    case expr_band:
    case expr_sub: case expr_add:
    case expr_mul: case expr_div:
    case expr_eq: case expr_neq:
    case expr_lt: case expr_le:
    case expr_gt: case expr_ge:
      return eval_binary(scope, e, eval_expr(scope, e->ops[0]), eval_expr(scope, e->ops[1]));
    }
  }

  [[nodiscard]] bool expr::operator==(const expr & rhs) const noexcept {
    if (type != rhs.type || name != rhs.name || idx_name != rhs.idx_name) {
      return false;
    }
    switch (exprs[type].form) {
    case form_var:
    case form_none: return true;
    case form_const: return const_val == rhs.const_val;
    case form_call:
      if (call.size != rhs.call.size) {
        return false;
      }
      for (u32_t i = 0U; i < call.size; ++i) {
        if (*call.args[i] != *rhs.call.args[i]) {
          return false;
        }
      }
      return true;
    case form_trinary:
      if (*ops[2] != *rhs.ops[2]) {
        return false;
      }
      [[fallthrough]];
    case form_binary:
      if (*ops[1] != *rhs.ops[1]) {
        return false;
      }
      [[fallthrough]];
    case form_unary:
      return *ops[0] == *rhs.ops[0];
    }
  }

  [[nodiscard]] bool expr::operator<(const expr & rhs) const noexcept {
    if (type != rhs.type) {
      return type < rhs.type;
    }
    if (name != rhs.name) {
      return name < rhs.name;
    }
    if (idx_name != rhs.idx_name) {
      return name < rhs.name;
    }
    switch (exprs[type].form) {
    case form_var:
    case form_none: return false;
    case form_const: return const_val < rhs.const_val;
    case form_call:
      if (call.size != rhs.call.size) {
        return call.size < rhs.call.size;
      }
      for (u32_t i = 0U; i < call.size; ++i) {
        if (*call.args[i] != *rhs.call.args[i]) {
          return *call.args[i] < *rhs.call.args[i];
        }
      }
      return false;
    case form_trinary:
      if (*ops[2] != *rhs.ops[2]) {
        return *ops[2] < *rhs.ops[2];
      }
      [[fallthrough]];
   case form_binary:
      if (*ops[1] != *rhs.ops[1]) {
        return *ops[1] < *rhs.ops[1];
      }
      [[fallthrough]];
    case form_unary:
      return *ops[0] < *rhs.ops[0];
    }
  }

  J_A(FLATTEN,NODISC) u32_t expr_hash::operator()(const expr & e) const noexcept {
    u32_t val = crc32(crc32(101 + e.type, e.idx_name.data(), e.idx_name.size()), e.name.data(), e.name.size());
    switch (exprs[e.type].form) {
    case form_var: case form_none: break;
    case form_const:
      val = crc32(val, (attr_value_hash{})(e.const_val));
      break;
    case form_call:
      for (u32_t i = 0U; i < e.call.size; ++i) {
        val = crc32(val, operator()(*e.call.args[i]));
      }
      break;
    case form_trinary:
      val = crc32(val, operator()(*e.ops[2]));
      [[fallthrough]];
    case form_binary:
      val = crc32(val, operator()(*e.ops[1]));
      [[fallthrough]];
    case form_unary:
      val = crc32(val, operator()(*e.ops[0]));
      break;
    }
    return val;
  }
}
