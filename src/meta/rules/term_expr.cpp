#include "term_expr.hpp"
#include "meta/value_types.hpp"
#include "meta/rules/id_resolver.hpp"
#include "exceptions/exceptions.hpp"
#include "logging/global.hpp"
#include "meta/module.hpp"
#include "meta/attr_value_hash.hpp"
#include "meta/term.hpp"
#include "containers/hash_map.hpp"
#include "meta/rules/dump.hpp"
#include "meta/value_types.hpp"
#include "meta/expr.hpp"
#include "meta/expr_scope.hpp"
#include "meta/rules/nonterminal.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/format.hpp"

namespace j::meta::inline rules {
  namespace s = strings;
  namespace st = strings::styles;
  namespace p = parsing;

#define INFIX(OP, PREC, NAME) [(u8_t)operator_type::OP] = {PREC, p::operator_type::infix, NAME}
#define POSTFIX(OP, PREC, NAME) [(u8_t)operator_type::OP] = {PREC, p::operator_type::postfix, NAME}

  constinit const parsing::operator_info operator_info[num_operator_types_v]{
    POSTFIX(none,     99,  "None"),
    INFIX(assign,     1,   "="),
    INFIX(assign_add, 1,   "+="),
    INFIX(assign_sub, 1,   "-="),
    INFIX(assign_mul, 1,   "*="),
    INFIX(assign_div, 1,   "/="),
    INFIX(add_value,  1,   "+=$"),
    INFIX(add_fx,     1,   "+=!"),
    INFIX(prepend_fx, 15,   "!+$"),
    INFIX(append_fx,  15,   "$+!"),
    INFIX(add,        15,  "+"),
    INFIX(sub,        15,  "-"),
    INFIX(mul,        17,  "*"),
    INFIX(div,        17,  "/"),

    INFIX(eq,         10,  "=="),
    INFIX(neq,        10,  "!="),
    INFIX(lt,         10,  "<"),
    INFIX(le,         10,  "<="),
    INFIX(gt,         10,  ">"),
    INFIX(ge,         10,  ">="),

    INFIX(lor,        5,   "||"),
    INFIX(land,       6,   "&&"),
    INFIX(lxor,       5,   "^^"),
    POSTFIX(fn_call,  100, "Call"),
    POSTFIX(prop_get, 100, "Prop"),
    POSTFIX(index,    20,  "[]"),
  };

#undef INFIX
#undef POSTFIX
#define ST(OP, S) [(u8_t)operator_type::OP] = st::S
  const s::style operator_style[num_operator_types_v]{
    ST(none, error),
    ST(assign, bright_red),
    ST(assign_add, bright_red),
    ST(assign_sub, bright_red),
    ST(assign_mul, bright_red),
    ST(assign_div, bright_red),
    ST(add_fx, bright_red),
    ST(add_value, bright_red),
    ST(prepend_fx, bright_red),
    ST(append_fx, bright_red),
    ST(add, green),
    ST(sub, yellow),
    ST(mul, bright_magenta),
    ST(div, bright_magenta),
    ST(eq, bright_green),
    ST(neq, bright_yellow),
    ST(lt, bright_yellow),
    ST(le, bright_yellow),
    ST(gt, bright_yellow),
    ST(ge, bright_yellow),
    ST(lor, bright_cyan),
    ST(land, bright_cyan),
    ST(lxor, bright_cyan),
    ST(fn_call, error),
    ST(prop_get, error),
    ST(index, bright_magenta),
  };
#undef ST

  [[nodiscard]] const te_function * te_fn_set::maybe_find_overload(span<te_type> arg_types) const noexcept {
    const te_function * best_cand = nullptr;
    const u32_t sz = arg_types.size();
    i32_t matching_args = -1;
    for (const te_function & fn : fns) {
      bool is_variadic = fn.is_variadic();
      if (is_variadic ? fn.args.size() - 1 > sz : fn.args.size() != sz) {
        continue;
      }
      i32_t cur_matching_args = 0;
      auto it = fn.args.begin();
    for (u32_t i = 0U; i < sz; ++i) {
      if (arg_types[i] == it->type) {
        cur_matching_args++;
      }
      if (!it->is_variadic()) {
        ++it;
      }
    }
    if (cur_matching_args > matching_args || (cur_matching_args == matching_args && !is_variadic)) {
      matching_args = cur_matching_args;
      best_cand = &fn;
    }
  }
  return best_cand;
}

[[nodiscard]] const te_function & te_fn_set::find_overload(span<te_type> arg_types) const noexcept {
  const te_function * best_cand = maybe_find_overload(arg_types);
  J_REQUIRE(best_cand, "No overload found.");
  return *best_cand;
}

[[nodiscard]] const te_function * te_fn_set::maybe_find_overload(span<te_value_source> args) const noexcept {
  te_type arg_types[args.size()];
  te_type * wr = arg_types;
  for (auto & arg : args) {
    *wr++ = arg.type;
  }
  return maybe_find_overload(span(arg_types, args.size()));
}

[[nodiscard]] const te_function & te_fn_set::find_overload(span<te_value_source> args) const noexcept {
  const te_function * best_cand = maybe_find_overload(args);
  J_REQUIRE(best_cand, "No overload found.");
  return *best_cand;
}

[[nodiscard]] s::string te_value_source::as_str(i32_t precedence) const noexcept {
  if (value.is_int()) {
    return s::format("{}", value.as_int());
  } else if (value.is_bool()) {
    return value.as_bool() ? "true" : "false";
  } else {
    J_REQUIRE(value.is_str(), "Unknown value", value.type);
  }
  if (this->precedence > precedence || this->precedence == 100 ) {
    return value.as_str();
  }
  return "(" + value.as_str() + ")";
}

namespace {
  template<typename Type, typename V>
  J_A(AI,NODISC,ND) inline te_value_source make_opaque(Type && type, V && attr, i32_t precedence = 0) noexcept {
    return {te_source_type::opaque, static_cast<Type &&>(type), attr_value(static_cast<V &&>(attr)), precedence};
  }

  template<typename Type>
  J_A(AI,NODISC,ND) inline te_value_source make_opaque(Type && type, const attr_value & attr, i32_t precedence = 0) noexcept {
    return {te_source_type::opaque, static_cast<Type &&>(type), attr, precedence};
  }
  template<typename Type>
  J_A(AI,NODISC,ND) inline te_value_source make_opaque(Type && type, attr_value && attr, i32_t precedence = 0) noexcept {
    return {te_source_type::opaque, static_cast<Type &&>(type), static_cast<attr_value &&>(attr), precedence};
  }

  template<typename Type, typename V>
  J_A(AI,NODISC,ND) inline te_value_source make_conv_const(Type && type, V && attr, i32_t precedence, attr_value && original) noexcept {
    return {te_source_type::converted_constant, static_cast<Type &&>(type), attr_value(static_cast<V &&>(attr)), precedence, static_cast<attr_value &&>(original)};
  }

  template<typename Type>
  J_A(AI,NODISC,ND) inline te_value_source make_conv_const(Type && type, const attr_value & attr, i32_t precedence, attr_value && original) noexcept {
    return {te_source_type::converted_constant, static_cast<Type &&>(type), attr, precedence, static_cast<attr_value &&>(original)};
  }
  template<typename Type>
  J_A(AI,NODISC,ND) inline te_value_source make_conv_const(Type && type, attr_value && attr, i32_t precedence, attr_value && original) noexcept {
    return {te_source_type::converted_constant, static_cast<Type &&>(type), static_cast<attr_value &&>(attr), precedence, static_cast<attr_value &&>(original)};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_constant(T && attr) noexcept {
    return {te_source_type::constant, attr_type_def::type_of(attr), static_cast<T &&>(attr)};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_constant(val_type * J_NOT_NULL type, T && attr) noexcept {
    return {te_source_type::constant, type, static_cast<T &&>(attr)};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_stack_term(val_type * J_NOT_NULL type, T && sv) noexcept {
    return {te_source_type::stack_term, type, static_cast<T &&>(sv)};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_select_term_ahead(val_type * J_NOT_NULL type, T && sv) noexcept {
    return {te_source_type::select_term_ahead, type, static_cast<T &&>(sv)};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_stack_nt(nt_data_type * J_NOT_NULL type, T && sv) noexcept {
    return {te_source_type::stack_nt, type, static_cast<T &&>(sv)};
  }

  J_A(AI,NODISC,ND) inline te_value_source make_stack_blob(i32_t size, i32_t stack_offset) noexcept {
    return {te_source_type::stack_nt, te_type{term_expr_type_kind::blob, size}, stack_offset};
  }

  template<typename T>
  J_A(AI,NODISC,ND) inline te_value_source make_stack_nt(nt_p J_NOT_NULL nt, T && sv) noexcept {
    return {te_source_type::stack_nt, nt, static_cast<T &&>(sv)};
  }

  J_A(AI,NODISC,ND) inline te_value_source make_stack_nt_field(val_type * J_NOT_NULL type, i32_t stack_offset, i32_t field_offset) noexcept {
    return {te_source_type::stack_nt_field, type, {stack_offset, field_offset}};
  }

  J_A(AI,NODISC,ND) inline te_value_source make_construct_term(val_type * J_NOT_NULL type, term * J_NOT_NULL t) noexcept {
    return {te_source_type::construct_term, te_type(type), te_construct_term{t}};
  }

  J_A(AI,NODISC,ND) inline te_value_source make_construct_term_dynamic(val_type * J_NOT_NULL type, te_value_source * J_NOT_NULL src) noexcept {
    return {te_source_type::construct_term, te_type(type), te_construct_term{nullptr, src}};
  }
}

void dump(dump_context & ctx, const te_type & t) noexcept {
  using enum term_expr_type_kind;
  switch (t.kind) {
  case none: ctx.to.write("NoneType", st::error); return;
  case any: ctx.to.write("AnyType", st::bright_red); return;
  case attr_value_type:
    ctx.to.write_formatted("{#bright_magenta}{}{/}{}", t.attr_value_type.type, t.attr_value_type.enum_def ? " " : "");
    if (t.attr_value_type.enum_def) {
      ctx.to.write(t.attr_value_type.enum_def->name, st::bright_yellow);
    }
    return;
  case term_or_terms: ctx.to.write("Term(s)", st::bright_yellow); return;
  case val_type: ctx.to.write(t.value_type->name, st::bright_yellow); return;
  case blob:
    ctx.to.write_formatted("{#bright_blue,bold}Blob({}b){/}", t.size);
    return;
  case nt: ctx.to.write(t.nt->name, st::bright_magenta); return;
  case nt_type: ctx.to.write_formatted(t.nt_type->name, st::bright_cyan); return;
  }
}

void dump(const te_type & t) noexcept {
  using enum term_expr_type_kind;
  switch (t.kind) {
  case none: J_DEBUG("{#error}NoneType{/}"); return;
  case any: J_DEBUG("{#bright_red}AnyType{/}"); return;
  case attr_value_type:
    J_DEBUG("{#bright_magenta}AttrType{/} {}{#bright_yellow} {}{/}", t.attr_value_type.type, t.attr_value_type.enum_def ? t.attr_value_type.enum_def->name : "");
    return;
  case term_or_terms:
    J_DEBUG("{#bright_yellow}Term(s){/}");
    return;
  case val_type:
    J_DEBUG("{#bright_yellow}ValType{/} {}", t.value_type->name);
    return;
  case nt:
    J_DEBUG("{#bright_blue,bold}NonTerminal{/} {} (Type {})", t.nt->name, t.get_nt_type()->name);
    return;
  case blob:
    J_DEBUG("{#bright_blue,bold}Blob{/} (Size {})", t.size);
    return;
  case nt_type:
    J_DEBUG("{#bright_cyan,bold}NonTerminal type{/} (Type {})", t.get_nt_type()->name);
    return;
  }
}

J_A(RNN) [[nodiscard]] nt_data_type * te_type::get_nt_type() const noexcept {
  if (kind == term_expr_type_kind::nt_type) {
    return nt_type;
  } else {
    J_ASSERT(kind == term_expr_type_kind::nt, "Tried to get nt_type of {}", dump_str(nullptr, *this));
    return nt->data_type.get();
  }
}
[[nodiscard]] te_type te_type::unify(const te_type & rhs) const noexcept {
  if (kind == rhs.kind && *this == rhs) {
    return *this;
  } else if (kind == term_expr_type_kind::nt_type || rhs.kind == term_expr_type_kind::nt_type) {
    auto dt = get_nt_type();
    if (dt == rhs.get_nt_type()) {
      return te_type{dt};
    }
  } else if (kind == term_expr_type_kind::term_or_terms && rhs.kind == term_expr_type_kind::val_type) {
    return rhs;
  } else if (rhs.kind == term_expr_type_kind::term_or_terms && kind == term_expr_type_kind::val_type) {
    return *this;
  }
  return te_type{};
}

[[nodiscard]] u32_t te_type::get_nt_size() const noexcept {
  switch (kind) {
  case term_expr_type_kind::nt_type:
    return nt_type->size;
  case term_expr_type_kind::nt:
    return nt->data_type->size;
  case term_expr_type_kind::blob:
    return size;
  case term_expr_type_kind::val_type:
    J_FAIL("Tried to get nt_type of ValType {}", value_type->name);
  default:
    J_FAIL("Unsupported nt_type");
  }
}


te_type::te_type(const te_type & rhs) noexcept
  : kind(rhs.kind)
{
  switch(kind) {
  case term_expr_type_kind::none:
  case term_expr_type_kind::any:
  case term_expr_type_kind::term_or_terms:
    break;
  case term_expr_type_kind::val_type:
    value_type = rhs.value_type;
    break;
  case term_expr_type_kind::nt_type:
    nt_type = rhs.nt_type;
    break;
  case term_expr_type_kind::nt:
    nt = rhs.nt;
    break;
  case term_expr_type_kind::blob:
    size = rhs.size;
    break;
  case term_expr_type_kind::attr_value_type:
    ::new (&attr_value_type) attr_type_def(rhs.attr_value_type);
    break;
  }
}

te_type::te_type(te_type && rhs) noexcept {
  j::memcpy(this, &rhs, sizeof(te_type));
  rhs.kind = term_expr_type_kind::none;
}

te_type & te_type::operator=(const te_type & rhs) noexcept {
  if (J_LIKELY(this != &rhs)) {
    clear();
    kind = rhs.kind;
    switch(kind) {
    case term_expr_type_kind::term_or_terms:
    case term_expr_type_kind::none:
    case term_expr_type_kind::any:
      break;
    case term_expr_type_kind::val_type:
      value_type = rhs.value_type;
      break;
    case term_expr_type_kind::nt_type:
      nt_type = rhs.nt_type;
      break;
    case term_expr_type_kind::nt:
      nt = rhs.nt;
      break;
    case term_expr_type_kind::blob:
      size = rhs.size;
      break;
    case term_expr_type_kind::attr_value_type:
      ::new (&attr_value_type) attr_type_def(rhs.attr_value_type);
      break;
    }
  }
  return *this;
}

te_type::te_type(const attr_type_def & t) noexcept
  : kind{term_expr_type_kind::attr_value_type},
    attr_value_type(t)
{ }

te_type::te_type(attr_type_def && t) noexcept
  : kind{term_expr_type_kind::attr_value_type},
    attr_value_type(static_cast<attr_type_def &&>(t))
{ }

te_type & te_type::operator=(te_type && rhs) noexcept {
  if (J_LIKELY(this != &rhs)) {
    clear();
    j::memcpy(this, &rhs, sizeof(te_type));
      rhs.kind = term_expr_type_kind::none;
    }
    return *this;
  }

  void te_type::clear() noexcept {
    if (kind == term_expr_type_kind::attr_value_type) {
      attr_value_type.~attr_type_def();
    }
    kind = term_expr_type_kind::none;
  }

  [[nodiscard]] bool te_type::operator==(const te_type & rhs) const noexcept {
    if (kind != rhs.kind) {
      return false;
    }
    switch(kind) {
    case term_expr_type_kind::none:
    case term_expr_type_kind::any:
    case term_expr_type_kind::term_or_terms:
      return true;
    case term_expr_type_kind::val_type:
      return value_type == rhs.value_type;
    case term_expr_type_kind::nt:
      return nt == rhs.nt;
    case term_expr_type_kind::nt_type:
      return nt_type == rhs.nt_type;
    case term_expr_type_kind::attr_value_type:
      return attr_value_type == rhs.attr_value_type;
    case term_expr_type_kind::blob:
      return size == rhs.size;
    }
  }

  [[nodiscard]] bool te_type::operator==(const attr_type_def & at) const noexcept {
    return kind == term_expr_type_kind::attr_value_type && attr_value_type == at;
  }

  const te_type te_type::term_or_terms{term_expr_type_kind::term_or_terms};
  const te_type te_type::any{term_expr_type_kind::any};

  void te_value_source::clear() noexcept {
    if (source_type == te_source_type::construct_term) {
      construct_term.~te_construct_term();
    }
    source_type = te_source_type::none;
  }

  namespace {
    struct J_TYPE_HIDDEN resolver final {
      const pat_captures & caps;
      expr_scope & scope;
      id_resolver * id_res;

      void operator()(reduction_arg & arg, term_expr_use, term_expr * te) const {
        if (arg.type == arg_type::reference) {
          arg.resolve(caps.at(arg.capture_index));
        }
        te->hash = reduction_arg_hash{}(arg);
      }

      void operator()(te_binop & b, term_expr_use, term_expr * te) const {
        visit_with_expr(b.lhs, *this, term_expr_use::normal);
        visit_with_expr(b.rhs, *this, term_expr_use::normal);
        te->hash = crc32((int)b.type, b.lhs->hash - 4 * b.rhs->hash);
      }

      void operator()(te_constant & c, term_expr_use, term_expr * te) const {
        if (c.init_expr && !c.value) {
          c.value = eval_expr(scope, c.init_expr);
          if (c.value.is_id()) {
            c.value.id.resolved = id_res->resolve(c.value.id.name.pkg, c.value.id.name.name);
          }
        }
        J_ASSERT(c.value);
        c.init_expr = nullptr;
        te->hash = attr_value_hash{}(c.value);
        if (c.type) {
          te->hash = crc32(1+c.type->index, te->hash);
        }
      }

      void operator()(te_term_construct & f, term_expr_use, term_expr * te) const {
        visit_with_expr(f.args, *this, term_expr_use::normal);
        if (f.term) {
          te->hash = crc32(111, f.term->index);
        } else {
          visit_with_expr(f.term_type_expr, *this, term_expr_use::normal);
          te->hash = crc32(112, f.term_type_expr->hash);
        }
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }
      void operator()(te_prop_read & f, term_expr_use, term_expr * te) const {
        visit_with_expr(f.val, *this, term_expr_use::normal);
        te->hash = crc32(13 + f.val->hash, f.prop_name.begin(), f.prop_name.size());
      }

      void operator()(te_member_fn_call & f, term_expr_use, term_expr * te) const {
        visit_with_expr(f.val, *this, term_expr_use::normal);
        visit_with_expr(f.args, *this, term_expr_use::normal);
        te->hash = crc32(12 + f.val->hash, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }

      void operator()(te_global_fn_call & f, term_expr_use, term_expr * te) const {
        visit_with_expr(f.args, *this, term_expr_use::normal);
        te->hash = crc32(121, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }

      void operator()(te_ternary & f, term_expr_use use, term_expr * te) const {
        visit_with_expr(f.condition, *this, term_expr_use::normal);
        visit_with_expr(f.true_branch, *this, use);
        visit_with_expr(f.false_branch, *this, use);
        te->hash = crc32(12 + f.condition->hash, f.true_branch->hash - 2 * f.false_branch->hash);
      }
    };

    struct J_TYPE_HIDDEN hasher final {
      expr_scope & scope;

      void operator()(reduction_arg & arg, term_expr * te) const {
        te->hash = reduction_arg_hash{}(arg);
      }

      void operator()(te_binop & b, term_expr * te) const {
        visit_with_expr(b.lhs, *this);
        visit_with_expr(b.rhs, *this);
        te->hash = crc32((int)b.type, b.lhs->hash - 4 * b.rhs->hash);
      }

      void operator()(te_constant & c, term_expr * te) const {
        te->hash = attr_value_hash{}(c.value);
        if (c.type) {
          te->hash = crc32(1+c.type->index, te->hash);
        }
      }

      void operator()(te_term_construct & f, term_expr * te) const {
        visit_with_expr(f.args, *this);
        if (f.term) {
          te->hash = crc32(111, f.term->index);
        } else {
          visit_with_expr(f.term_type_expr, *this);
          te->hash = crc32(112, f.term_type_expr->hash);
        }
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }
      void operator()(te_prop_read & f, term_expr * te) const {
        visit_with_expr(f.val, *this);
        te->hash = crc32(13 + f.val->hash, f.prop_name.begin(), f.prop_name.size());
      }

      void operator()(te_member_fn_call & f, term_expr * te) const {
        visit_with_expr(f.val, *this);
        visit_with_expr(f.args, *this);
        te->hash = crc32(12 + f.val->hash, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }

      void operator()(te_global_fn_call & f, term_expr * te) const {
        visit_with_expr(f.args, *this);
        te->hash = crc32(121, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
      }

      void operator()(te_ternary & f, term_expr * te) const {
        visit_with_expr(f.condition, *this);
        visit_with_expr(f.true_branch, *this);
        visit_with_expr(f.false_branch, *this);
        te->hash = crc32(12 + f.condition->hash, f.true_branch->hash - 2 * f.false_branch->hash);
      }
    };

    struct J_TYPE_HIDDEN type_getter final {
      expr_scope & scope;
      module * mod;

      te_type operator()(te_term_construct &) const noexcept {
        J_ASSERT_NOT_NULL(mod->trs_term_type);
        return mod->trs_term_type;
      }

      te_type operator()(te_binop & op) const {
        using enum operator_type;
        switch (op.type) {
        case none: case fn_call: case prop_get: J_FAIL("Not a binop");
        case assign:
          return visit(op.lhs, *this);
        default: {
          te_type types[]{visit(op.lhs, *this), visit(op.rhs, *this)};
          J_ASSERT(types[0].is_val_type());
          return types[0].value_type->operators.get_operator(op.type, span(types, 2)).return_type;
        }
        }
      }

      te_type operator()(te_prop_read & f) const {
        te_type t = visit(f.val, *this);
        if (t.is_any_nt()) {
          const nt_data_field * field;
          if (t.is_nt()) {
            field = &t.nt->field_at(f.prop_name);
          } else {
            field = &t.nt_type->field_at(f.prop_name);
          }
          return field->type;
        }
        if (!t.is_val_type()) {
          scope.throw_expr_error(s::format("Type {} is not ValType or Nt.", dump_str(mod, t)));
        }
        return t.value_type->props.at(f.prop_name).result;
      }

      te_type operator()(te_constant & f) const {
        return f.type ? te_type(f.type) : attr_type_def::type_of(f.value);
      }

      te_type operator()(te_ternary & f) const {
        return visit(f.true_branch, *this).unify(visit(f.false_branch, *this));
      }

      te_type operator()(reduction_arg & f) const {
        using enum arg_type;
        switch (f.type) {
        case none:
        case reference:
          scope.throw_expr_error("Unsupported arg type");
        case capture_term:
        case select_term_ahead:
          return mod->trs_term_type;
        case capture_reduction_field:
        case capture_term_field:
          return f.value_type;
        case capture_reduction_copy:
          return f.data_type;
        case capture_reduction_copy_nt:
        case capture_this_reduction_nt:
          return f.nt;
        case capture_reduction_copy_sz:
          return te_type(term_expr_type_kind::blob, f.size);
        }
      }

      te_type operator()(te_member_fn_call & f) const {
        te_type types[f.args.size() + 1];
        types[0] = visit(f.val, *this);
        if (!types[0].is_val_type()) {
          scope.throw_expr_error("Type is not a val type.");
        }
        i32_t i = 1;
        for (auto & arg : f.args) {
          types[i++] = visit(arg, *this);
        }
        auto & fn = types[0].value_type->fns.at(f.fn_name);
        auto * overload = fn.maybe_find_overload(span(types, i));
        if (!overload) {
          scope.throw_expr_error(s::format("No overload found", f.fn_name));
        }
        return overload->return_type;
      }

      te_type operator()(te_global_fn_call & f) const {
        const u32_t sz = f.args.size();
        const te_fn_set * fns = mod->te_functions.maybe_at(f.fn_name);
        J_ASSERT_NOT_NULL(fns, "No function '{}'.", f.fn_name);
        te_type arg_types[sz];
        for (u32_t i = 0U; i < sz; ++i) {
          arg_types[i] = visit(f.args[i], *this);
        }
        return fns->find_overload(span(arg_types, sz)).return_type;
      }
    };

    struct J_TYPE_HIDDEN nt_clearer final {
      bool operator()(te_binop & b, term_expr_use, term_expr * te) const {
        visit_with_expr(b.lhs, *this, term_expr_use::normal);
        visit_with_expr(b.rhs, *this, term_expr_use::normal);
        te->hash = crc32((int)b.type, b.lhs->hash - 4 * b.rhs->hash);
        return true;
      }

      bool operator()(te_term_construct & f, term_expr_use, term_expr * te) const {
        bool did_change = false;
        for (term_expr * a : f.args) {
          did_change |= visit_with_expr(a, *this, term_expr_use::normal);
        }
        if (f.term) {
          te->hash = crc32(111, f.term->index);
        } else {
          did_change |= visit_with_expr(f.term_type_expr, *this, term_expr_use::normal);
          te->hash = crc32(112, f.term_type_expr->hash);
        }
        return did_change;
      }

      bool operator()(te_prop_read & f, term_expr_use, term_expr * te) const {
        bool did_change = visit_with_expr(f.val, *this, term_expr_use::normal);
        te->hash = crc32(13 + f.val->hash, f.prop_name.begin(), f.prop_name.size());
        return did_change;
      }

      bool operator()(te_ternary & f, term_expr_use use, term_expr * te) const {
        bool did_change = (i32_t)visit_with_expr(f.condition, *this, term_expr_use::normal) | visit_with_expr(f.true_branch, *this, use) | visit_with_expr(f.false_branch, *this, use);
        te->hash = crc32(12 + f.condition->hash, f.true_branch->hash - 2 * f.false_branch->hash);
        return did_change;
      }

      bool operator()(te_constant & c, term_expr_use, term_expr * te) const noexcept {
        te->hash = attr_value_hash{}(c.value);
        if (c.type) {
          te->hash = crc32(1+c.type->index, te->hash);
        }
        return false;
      }

      bool operator()(reduction_arg & arg, term_expr_use use, term_expr * te) const {
        if ((arg.type == arg_type::capture_this_reduction_nt || arg.type == arg_type::capture_reduction_copy_nt) && arg.nt) {
          arg.data_type = arg.nt->data_type.get();
          arg.type = arg_type::capture_reduction_copy;
        }
        if (use == term_expr_use::copy && arg.type == arg_type::capture_reduction_copy && arg.data_type) {
          arg.type = arg_type::capture_reduction_copy_sz;
          u32_t sz = arg.data_type->size;
          J_ASSERT(!sz == !arg.data_type->fields);
          arg.nt = nullptr;
          arg.size = 0;
        }
        te->hash = reduction_arg_hash{}(arg);
        return false;
      }

      bool operator()(te_global_fn_call & f, term_expr_use, term_expr * te) const {
        bool did_change = false;
        for (term_expr * a : f.args) {
          did_change |= visit_with_expr(a, *this, term_expr_use::normal);
        }
        te->hash = crc32(121, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
        return did_change;
      }

      bool operator()(te_member_fn_call & f, term_expr_use, term_expr * te) const {
        bool did_change = visit_with_expr(f.val, *this, term_expr_use::normal);
        for (term_expr * a : f.args) {
          did_change |= visit_with_expr(a, *this, term_expr_use::normal);
        }
        te->hash = crc32(12 + f.val->hash, f.fn_name.begin(), f.fn_name.size());
        for (term_expr * a : f.args) {
          te->hash = crc32(te->hash, a->hash);
        }
        return did_change;
      }
    };

    struct J_TYPE_HIDDEN dumper final {
      dump_context & ctx;

      void dump_args(const term_exprs & args, bool paren = true) const noexcept {
        if (paren) {
          ctx.to.write("(");
        }
        bool is_first = true;
        for (auto arg : args) {
          if (!is_first) {
            ctx.to.write(" ");
          }
          visit(arg, *this);
          is_first = false;
        }
        if (paren) {
          ctx.to.write(")");
        }
      }

      void operator()(const te_term_construct & f) const {
        if (f.term) {
          const s::string * name = f.term->attrs.maybe_string_at("ShortName");
          if (!name) {
            name = &f.term->name;
          }
          ctx.to.write(*name);
        } else {
          visit(f.term_type_expr, *this);
        }
        dump_args(f.args);
      }

      void operator()(const te_binop & f) const {
        using enum operator_type;
        visit(f.lhs, *this);
        if (f.type == operator_type::index) {
          ctx.to.write("[", st::bold);
          visit(f.rhs, *this);
          ctx.to.write("]", st::bold);
        } else {
          ctx.to.write(" ");
          ctx.to.write(operator_info[(u8_t)f.type].name, operator_style[(u8_t)f.type]);
          ctx.to.write(" ");
          visit(f.rhs, *this);
        }
      }

      void operator()(const te_prop_read & f) const {
        if (f.val->type != term_expr_type::arg || f.val->arg.capture_index != -1) {
          visit(f.val, *this);
          ctx.to.write(".", st::bold);
        }
        ctx.to.write(f.prop_name, st::bright_cyan);
      }

      void operator()(const te_ternary & f) const {
        ctx.to.write("If(", st::bright_cyan);
        visit(f.condition, *this);
        ctx.to.write(" ");
        visit(f.true_branch, *this);
        ctx.to.write(" ");
        visit(f.false_branch, *this);
        ctx.to.write(")", st::bright_cyan);
      }

      void operator()(const te_constant & f) const {
        if (f.type) {
          ctx.to.write(f.value.as_str(), st::green);
        } else if (f.value) {
          dump(ctx, f.value);
        } else {
          J_ASSERT(f.init_expr);
          dump(ctx, *f.init_expr);
        }
      }

      void operator()(const reduction_arg & f) const {
        using enum arg_type;
        const char * fmt;
        switch (f.type) {
        case none: case capture_term_field:
          J_FAIL("Unsupported arg");
        case reference:
          fmt = "{#bright_yellow,bold}${}{/}";
          break;
        case select_term_ahead:
        case capture_term:
          fmt = "{#bright_green,bold}${}{/}";
          break;
        case capture_reduction_field:
          fmt = "{#bright_magenta,bold}${}{/}";
          break;
        case capture_reduction_copy:
        case capture_reduction_copy_nt:
        case capture_this_reduction_nt:
        case capture_reduction_copy_sz:
          fmt = "{#bright_cyan,bold}${}{/}";
          break;
        }
        ctx.to.write_formatted(fmt, f.capture_index + 1);
      }

      void operator()(const te_member_fn_call & f) const {
        visit(f.val, *this);
        ctx.to.write(".", st::bold);
        ctx.to.write(f.fn_name, st::bright_yellow);
        dump_args(f.args);
      }

      void operator()(const te_global_fn_call & f) const {
        ctx.to.write(f.fn_name, st::bright_yellow);
        dump_args(f.args);
      }
    };

    struct J_TYPE_HIDDEN copier final {
      copy_cb_t callback;
      void* userdata;

      term_exprs copy_args(const term_exprs & rhs) const {
        term_exprs result{rhs.size()};
        for (auto & arg : rhs) {
          result.emplace_back(copy(arg, callback, userdata));
        }
        return result;
      }

      term_expr * operator()(const te_term_construct & f) const {
        return ::new term_expr(f.term
                               ? te_term_construct{f.term, copy_args(f.args)}
                               : te_term_construct{copy(f.term_type_expr, callback, userdata), copy_args(f.args)});
      }

      term_expr * operator()(const te_binop & f) const {
        return ::new term_expr(te_binop{f.type, copy(f.lhs, callback, userdata), copy(f.rhs, callback, userdata)});
      }

      term_expr * operator()(const te_prop_read & f) const {
        return ::new term_expr(te_prop_read{copy(f.val, callback, userdata), f.prop_name});
      }

      term_expr * operator()(const te_ternary & f) const {
        return ::new term_expr(te_ternary{copy(f.condition, callback, userdata),
            copy(f.true_branch, callback, userdata),
            copy(f.false_branch, callback, userdata)});
      }

      term_expr * operator()(const te_constant & f) const {
        return ::new term_expr(te_constant{f.init_expr, f.value, f.type});
      }

      term_expr * operator()(const reduction_arg & f) const {
        return ::new term_expr(f);
      }

      term_expr * operator()(const te_member_fn_call & f) const {
        return ::new term_expr(te_member_fn_call{copy(f.val, callback, userdata), f.fn_name, copy_args(f.args)});
      }

      term_expr * operator()(const te_global_fn_call & f) const {
        return ::new term_expr(te_global_fn_call{f.fn_name, copy_args(f.args)});
      }
    };

    struct J_TYPE_HIDDEN arg_collector final {
      arg_counts args;

      J_A(AI) inline void operator()(const te_term_construct & f) noexcept {
        if (f.term_type_expr) {
          visit(f.term_type_expr, *this);
        }
        visit(f.args, *this);
      }

      J_A(AI) inline void operator()(const te_binop & f) noexcept {
        visit(f.lhs, *this);
        visit(f.rhs, *this);
      }

      void operator()(const te_ternary & f) noexcept {
        visit(f.condition, *this);
        visit(f.true_branch, *this);
        visit(f.false_branch, *this);
      }

      J_A(AI) inline void operator()(const te_prop_read & f) noexcept {
        if (f.val->type == term_expr_type::arg) {
          auto & arg = f.val->arg;
          if (arg.type == arg_type::capture_this_reduction_nt || arg.type == arg_type::capture_reduction_copy_nt) {
            auto & field = arg.nt->field_at(f.prop_name);
            reduction_arg a{(i8_t)arg.stack_offset, arg_type::capture_reduction_field, field.type, (u8_t)field.offset};
            args.add(a);
            return;
          }
        }
        visit(f.val, *this);
      }

      J_A(AI) inline constexpr void operator()(const te_constant &) const noexcept { }

      void operator()(const reduction_arg & f) noexcept {
        args.add(f);
      }

      inline void operator()(const te_member_fn_call & f) noexcept {
        visit(f.val, *this);
        visit(f.args, *this);
      }

      inline void operator()(const te_global_fn_call & f) noexcept {
        visit(f.args, *this);
      }
    };

    struct J_TYPE_HIDDEN extent_calc final {
      pair<i32_t> extent{0, 0};

      void operator()(const te_term_construct & f) {
        if (f.term_type_expr) {
          visit(f.term_type_expr, *this);
        }
        visit(f.args, *this);
      }

      void operator()(const te_binop & f) {
        visit(f.lhs, *this);
        visit(f.rhs, *this);
      }

      void operator()(const te_ternary & f) {
        visit(f.condition, *this);
        visit(f.true_branch, *this);
        visit(f.false_branch, *this);
      }

      void operator()(const te_prop_read & f) {
        visit(f.val, *this);
      }

      void operator()(const te_constant) noexcept {
        return;
      }

      void operator()(const reduction_arg & f) {
        using enum arg_type;
        switch (f.type) {
        case none:
        case reference:
        case capture_term_field:
          J_FAIL("Unsupported arg");
        case select_term_ahead:
          extent.second = j::min(extent.second, f.stack_offset);
          break;
        case capture_term:
        case capture_reduction_field:
        case capture_reduction_copy:
        case capture_reduction_copy_nt:
        case capture_this_reduction_nt:
        case capture_reduction_copy_sz:
          extent.first = j::max(extent.first, f.stack_offset + 1);
          break;
        }
      }

      void operator()(const te_member_fn_call & f) {
        visit(f.val, *this);
        visit(f.args, *this);
      }

      void operator()(const te_global_fn_call & f) {
        visit(f.args, *this);
      }
    };

    struct J_TYPE_HIDDEN compile_opts final {
      const te_type * type;
      bool for_write = false;

      J_A(AI,NI) inline compile_opts(const te_type * type = nullptr) noexcept
        : type(type)
      {
      }

      J_A(AI,NI) inline compile_opts(const te_type * type, bool for_write) noexcept
        : type(type),
        for_write(for_write)
      {
      }
    };

    struct J_TYPE_HIDDEN compiler final {
      module * mod;
      precomputes & precs;
      trs_config * trs;
      expr_scope & scope;
      te_type bool_type;

      compiler(expr_scope & scope, precomputes & precs)
        : mod(scope.root->mod),
          precs(precs),
          trs(&mod->trs_config),
          scope(scope),
          bool_type(mod->trs_bool_type)
      { }

      [[nodiscard]] attr_value args_to_attr_value(vector<te_value_source> & args) {
        attr_value result[args.size()];
        u32_t i = 0U;
        for (auto & a : args) {
          evaluate_value(a);
          result[i] = a.value;
          J_ASSERT(result[i]);
          ++i;
        }
        return attr_value(span(result, i));
      }

      [[nodiscard]] attr_value to_struct(te_value_source && src) {
        evaluate_value(src);
        pair<const char*, attr_value> values[]{
          {"Source", attr_value(trs->te_nt_data_source_enum, (u32_t)src.source_type)},
          {"Value", static_cast<attr_value &&>(src.value)},
          {"Type", (src.type.kind == term_expr_type_kind::term_or_terms || src.type.kind == term_expr_type_kind::blob) ? attr_value() :
              src.type.is_val_type() ? attr_value(src.type.value_type) : attr_value(trs->nt_data_type(src.type.get_nt_type()))},
          {}, {},
        };
        u32_t sz = 3U;
        using enum te_source_type;
        switch (src.source_type) {
        case none:
          J_FAIL("None value");
        case opaque: case constant:
          break;
        case converted_constant:
          values[3] = {"Original", src.original};
          ++sz;
          break;
        case stack_nt:
          values[4] = {"Size", attr_value(src.type.get_nt_size())};
          values[3] = {"StackOffset", attr_value(src.stack_value.stack_offset)};
          sz += 2;
          break;
        case stack_nt_field:
          values[4] = {"FieldOffset", attr_value(src.stack_field.field_offset)};
          sz++;
          [[fallthrough]];
        case stack_term:
          values[3] = {"StackOffset", attr_value(src.stack_value.stack_offset)};
          sz++;
          break;
        case select_term_ahead:
          values[3] = {"InputOffset", attr_value(src.term_ahead.offset)};
          sz++;
          break;
        case construct_term:
          if (src.construct_term.term) {
            values[3] = {"Term", attr_value(src.construct_term.term)};
          } else {
            evaluate_value(*src.construct_term.dyn_term_type);
            values[3] = {"Term", src.construct_term.dyn_term_type->value};
          }
          sz++;
          break;
        }
        return attr_value(trs->te_value_struct, span<pair<const char *, attr_value>>((pair<const char *, attr_value>*)values, sz));
      }

      void evaluate_value(te_value_source & src) {
        if (src.value) {
          return;
        }
        using enum te_source_type;
        expr_scope sub(&scope);
        const codegen_template * tpl = nullptr;
        switch (src.source_type) {
        case none:
        case opaque:
        case converted_constant:
        case constant: J_FAIL("Const/none in eval");
        case stack_term:
          sub.set("StackOffset", attr_value(src.stack_value.stack_offset));
          tpl = &trs->get_stack_term;
          break;
        case select_term_ahead:
          sub.set("InputOffset", attr_value(src.term_ahead.offset));
          tpl = &trs->get_input_term;
          break;
        case stack_nt:
          sub.set("StackOffset", attr_value(src.stack_value.stack_offset));
          if (src.type.kind == term_expr_type_kind::blob) {
            sub.set("Size", attr_value(src.type.size));
            return;
          } else {
            sub.set("NtType", trs->nt_data_type(src.type.get_nt_type()));
            sub.set("Size", attr_value(src.type.get_nt_type()->size));
          }
          tpl = &trs->get_stack_nt;
          break;
        case stack_nt_field:
          sub.set("StackOffset", attr_value(src.stack_field.stack_offset));
          sub.set("FieldOffset", attr_value(src.stack_field.field_offset));
          sub.set("ValType", attr_value((node*)src.type.value_type));
          tpl = &trs->get_stack_nt_field;
          break;
        case construct_term: {
          if (src.construct_term.term) {
            sub.set("Term", attr_value(src.construct_term.term));
          } else {
            evaluate_value(*src.construct_term.dyn_term_type);
            sub.set("Term", attr_value(src.construct_term.dyn_term_type->value));
          }
          const u32_t const_sz = src.construct_term.const_args.size();
          const u32_t in_sz = src.construct_term.in_args.size();
          attr_value const_args[const_sz];
          attr_value raw_const_args[const_sz];
          attr_value in_args[in_sz];
          for (u32_t i = 0U; i != const_sz; ++i) {
            evaluate_value(src.construct_term.const_args[i]);
            const_args[i] = src.construct_term.const_args[i].value;
            raw_const_args[i] = to_struct(static_cast<te_value_source &&>(src.construct_term.const_args[i]));
          }
          for (u32_t i = 0U; i != in_sz; ++i) {
            evaluate_value(src.construct_term.in_args[i]);
            in_args[i] = src.construct_term.in_args[i].value;
          }
          sub.set("InArgs", attr_value(span(in_args, in_sz)));
          sub.set("ConstArgs", attr_value(span(const_args, const_sz)));
          sub.set("RawConstArgs", attr_value(span(raw_const_args, const_sz)));
          tpl = &trs->construct_term;
          break;
        }
        }
        src.value = attr_value(mod->expand(*tpl, sub));
        J_ASSERT(src.value);
      }

      [[noreturn]] void throw_convert_error(const char * J_NOT_NULL msg, const te_value_source & in, const te_type & to) {
        throw_error(msg, dump_str(mod, in.type), dump_str(mod, to));
      }

      void check_convert(bool ok, const char * J_NOT_NULL msg, const te_type & a, const te_type & b) {
        if (J_UNLIKELY(!ok)) {
          throw_error(msg, dump_str(mod, a), dump_str(mod, b));
        }
      }

      [[nodiscard]] te_value_source convert_value(te_value_source && in, te_type to) {

        {
          using enum term_expr_type_kind;

          if (to.kind == term_or_terms) {
            check_convert(in.type.is_val_type() && (in.type.value_type->is_trs_terms_type || in.type.value_type->is_trs_term_type),
                          "Cannot convert to term(s). From: {}, To: {}", in.type, to);
            if (in.type.value_type == mod->trs_term_type || in.type.value_type == mod->trs_terms_type) {
              evaluate_value(in);
              return static_cast<te_value_source &&>(in);
            }
            to = in.type.value_type->is_trs_terms_type ? mod->trs_terms_type : mod->trs_term_type;
          }
          if (in.type == to || to.is_any()) {
            evaluate_value(in);
            return static_cast<te_value_source &&>(in);
          }

          check_convert(!to.is_attr_value(), "Cannot convert to attr value. From: {}, To: {}", in.type, to);

          const bool in_is_nt = in.type.kind == nt || in.type.kind == nt_type;
          const bool out_is_nt = to.kind == nt || to.kind == nt_type;
          if (out_is_nt) {
            check(in_is_nt, "Cannot convert to nonterminal type {}.", to.get_nt_type()->name);
            if (in.type.get_nt_type() == to.get_nt_type()) {
              in.type = to;
              return static_cast<te_value_source &&>(in);
            }
            throw_convert_error("Cannot convert between distinct nonterminal types, from {} to {}.",
                                in, to);
          }

          if (to.kind == blob && !to.size && in.type.kind == blob) {
            in.type = to;
            return static_cast<te_value_source &&>(in);
          }
          if (to.kind == blob && in_is_nt) {
            check(!to.size || to.size == in.type.get_nt_size(), "Cannot convert {}, to blob of size {}.", in.type.get_nt_type()->name, to.size);
            in.type = to;
            return static_cast<te_value_source &&>(in);
          }

          check_convert(!in_is_nt, "Cannot convert to {}, from nonterminal type {}.", to, in.type);
        }

        val_type * vt = to.value_type;
        check_convert(to.is_val_type(), "Cannot convert to {} from {}.", to, in.type);
        expr_scope sub(&scope);
        evaluate_value(in);
        sub.set("It", attr_value(in.value));
        if (in.type.is_attr_value()) {
          auto conv = vt->maybe_get_attr_conversion(in.type.attr_value_type);
          if (conv) {
            return make_conv_const(vt, mod->expand(*conv, sub), 100, static_cast<attr_value&&>(in.value));
          }
          throw_error("No conversion from attr_value to {}.", vt->name);
        } else {
          check_convert(in.type.is_val_type(), "Cannot convert to {} from {}.", to, in.type);
          auto conv = vt->maybe_get_conversion(in.type.value_type);
          if (conv) {
            return make_opaque(vt, mod->expand(*conv, sub), 100);
          }
          throw_error("No conversion from {} to {}.", in.type.value_type->name, vt->name);
        }
      }

      template<typename... Ts>
      [[noreturn]] void throw_error(const char * J_NOT_NULL msg, Ts && ... args) const {
        scope.throw_expr_error(s::format(msg, static_cast<Ts&&>(args)...));
      }

      [[noreturn]] void throw_error(const char * J_NOT_NULL msg) const {
        scope.throw_expr_error(msg);
      }

      template<typename... Ts>
      J_A(AI) inline void check(bool ok, const char * J_NOT_NULL msg, Ts && ... args) const {
        if (!ok) {
          scope.throw_expr_error(s::format(msg, static_cast<Ts&&>(args)...));
        }
      }

      J_A(AI) inline void check(bool ok, const char * J_NOT_NULL msg) const {
        if (!ok) {
          scope.throw_expr_error(msg);
        }
      }

      te_value_source operator()(te_term_construct & f, compile_opts) {
        struct term * term = f.term;
        te_value_source * term_type = nullptr;
        if (!term) {
          te_type tt{mod->trs_term_type_type};
          J_ASSERT(mod->trs_term_type_type);
          term_type = ::new te_value_source(compile(f.term_type_expr, &tt));
        }
        te_value_source result = term
          ? make_construct_term(mod->trs_term_type, term)
          : make_construct_term_dynamic(mod->trs_term_type, term_type);
        auto it = term ? term->operands().begin() : nullptr;
        auto end = term ? term->operands().end() : nullptr;
        for (term_expr * arg_expr : f.args) {
          term_member * op = it != end ? *it : nullptr;
          te_type type;
          bool is_variadic = false;
          if (op) {
            if (op->is_input()) {
              is_variadic = op->type->is_trs_terms_type;
              if (!is_variadic || it + 1 == end) {
                type = te_type::term_or_terms;
              }
            } else {
              type = te_type(op->type);
            }
          }
          te_value_source arg = compile(arg_expr, type ? &type : nullptr);
          if (arg.type.is_val_type() && (arg.type.value_type->is_trs_term_type || arg.type.value_type->is_trs_terms_type)) {
            result.construct_term.in_args.emplace_back(static_cast<te_value_source &&>(arg));
            if (it != end && !is_variadic) {
              ++it;
            }
          } else {
            result.construct_term.const_args.emplace_back(convert_value(static_cast<te_value_source &&>(arg), it ? (*it)->type : nullptr));
            if (it != end) {
              it += is_variadic ? 2 : 1;
            }
          }
        }
        return result;
      }

      te_value_source operator()(te_binop & f, compile_opts opts) {
        te_value_source args[2] = {
          compile(f.lhs),
          compile(f.rhs)
        };
        te_value_source result;
        using enum operator_type;

        if (args[0].type.is_val_type()) {
          const te_function & fn = args[0].type.value_type->operators.get_operator(f.type, span(args, 2));
          expr_scope sub(&scope);
          args[0] = convert_value(static_cast<te_value_source &&>(args[0]), fn.args[0].type);
          args[1] = convert_value(static_cast<te_value_source &&>(args[1]), fn.args[1].type);
          evaluate_value(args[0]);
          evaluate_value(args[1]);
          sub.set(fn.args.at(0).name, attr_value(args[0].as_str(fn.precedence)));
          sub.set(fn.args.at(1).name, args[1].type.is_val_type() ? attr_value(args[1].as_str(fn.precedence)) : args[1].value);
          result = make_opaque(fn.return_type, mod->expand(fn.tpl, sub), fn.precedence);
        } else {
          J_FAIL("Operator not supported");
        }
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      te_value_source operator()(te_ternary & f, compile_opts opts) {
        te_value_source cond = compile(f.condition, &bool_type);
        te_value_source true_value = compile(f.true_branch, opts);
        te_value_source false_value = compile(f.false_branch, opts);
        evaluate_value(cond);
        evaluate_value(true_value);
        evaluate_value(false_value);
        return make_opaque(true_value.type, attr_value(s::format("({} ? {} : {})", cond.as_str(0), true_value.as_str(-1), false_value.as_str(0))), 0);
      }

      te_value_source operator()(te_prop_read & f, compile_opts opts) {
        te_value_source val = compile(f.val, compile_opts{nullptr, opts.for_write});
        if (val.type.is_val_type()) {
          evaluate_value(val);
          check(val.type.value_type->props.contains(f.prop_name), "No property {} in {}.",
                                                    f.prop_name, val.type.value_type->name);
          auto & prop = val.type.value_type->props.at(f.prop_name);
          expr_scope sub(&scope);
          sub.set("It", attr_value(val.value));
          te_value_source result = make_opaque(prop.result, attr_value(mod->expand(prop.tpl, sub)), prop.precedence);
          return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
        }
        check(val.type.is_any_nt(), "Expected nonterminal type.", dump_str(mod, val.type));
        te_value_source result;
        const nt_data_field * field = val.type.is_nt() ? val.type.nt->field_maybe_at(f.prop_name)
          : val.type.nt_type->field_maybe_at(f.prop_name);
        check(field, "No field {} in {}", f.prop_name, val.type.nt ? val.type.nt->name : val.type.nt_type->name);
        result = make_stack_nt_field(field->type, val.stack_value.stack_offset, field->offset);
        reduction_arg a{(i8_t)val.stack_value.stack_offset, arg_type::capture_reduction_field, field->type, (u8_t)field->offset};
        if (!opts.for_write) {
          if (precompute * prec = precs.find_precompute(a)) {
            if (!prec->value) {
              evaluate_value(result);
              prec->value = result.value.as_str();
            }
            result.value = attr_value(prec->name);
          }
        }
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      te_value_source operator()(te_constant & f, compile_opts opts) {
        J_ASSERT(f.value && !f.init_expr);
        te_value_source result = f.type ? make_constant(f.type, attr_value(f.value)) : make_constant(attr_value(f.value));
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      te_value_source operator()(reduction_arg & f, compile_opts opts) {
        using enum arg_type;
        te_value_source result;
        switch (f.type) {
        case none: case reference: case capture_term_field: J_FAIL("Unsupported arg");
        case select_term_ahead:
          result = make_select_term_ahead(mod->trs_term_type, f.stack_offset);
          break;
        case capture_term:
          result = make_stack_term(mod->trs_term_type, f.stack_offset);
          break;
        case capture_reduction_field:
          result = make_stack_nt_field(f.value_type, f.stack_offset, f.field_offset);
          break;
        case capture_reduction_copy:
          result = make_stack_nt(f.data_type, f.stack_offset);
          break;
        case capture_reduction_copy_sz:
          result = make_stack_blob(f.size, f.stack_offset);
          break;
        case capture_reduction_copy_nt:
        case capture_this_reduction_nt:
          result = make_stack_nt(f.nt, f.stack_offset);
          break;
        }
        if (!opts.for_write) {
          if (precompute * prec = precs.find_precompute(f)) {
            if (!prec->value) {
              evaluate_value(result);
              prec->value = result.value.as_str();
            }
            result.value = attr_value(prec->name);
          }
        }
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      te_value_source operator()(te_global_fn_call & f, compile_opts opts) {
        const u32_t sz = f.args.size();

        te_type arg_types[sz];
        for (u32_t i = 0U; i < sz; ++i) {
          arg_types[i] = f.args[i]->get_type(scope);
        }

        const te_function * overload = mod->te_functions.at(f.fn_name).maybe_find_overload(span(arg_types, sz));

        if (!overload) {
          scope.throw_expr_error(s::format("No overload found for {}", f.fn_name));
        }
        auto it = overload->args.begin();
        te_value_source arg_vals[sz];
        expr_scope sub{&this->scope};
        for (u32_t i = 0U; i < sz; ++i, ++it) {
          arg_vals[i] = compile(f.args[i], compile_opts(&it->type, it->is_written()));
          sub.set(it->name, convert_value(static_cast<te_value_source &&>(arg_vals[i]), it->type).value);
        }
        te_value_source result = make_opaque(overload->return_type, attr_value(mod->expand(overload->tpl, sub)), overload->precedence);
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      te_value_source operator()(te_member_fn_call & f, compile_opts opts) {
        expr_scope sub{&this->scope};
        const i32_t sz = f.args.size() + 1;
        te_type arg_types[sz];
        i32_t i = 0;
        arg_types[i++] = f.val->get_type(scope);
        for (auto & arg : f.args) {
          arg_types[i++] = arg->get_type(scope);
        }
        check(arg_types[0].is_val_type(), "Expected val type for `this` argument.");
        const te_function * overload = arg_types[0].value_type->fns.at(f.fn_name).maybe_find_overload(span(arg_types, i));
        if (!overload) {
          sub.throw_expr_error(s::format("No overload for ", f.fn_name)) ;
        }

        const te_function_arg * fn_arg = overload->args.begin();
        te_value_source args[sz];
        i = 0;
        args[i++] = compile(f.val, compile_opts(arg_types[i] ? &arg_types[i] : nullptr, fn_arg[i].is_written()));
        for (auto & arg : f.args) {
          args[i++] = compile(arg, compile_opts(arg_types[i] ? &arg_types[min(i, sz - 1)] : nullptr, fn_arg[min(i, sz - 1)].is_written()));
        }

        const bool is_variadic = overload->is_variadic();
        const i32_t num_variadic = is_variadic ? f.args.size() - overload->args.size() + 2 : 0;
        attr_value varargs[num_variadic];
        for (i32_t i = 0, j = 0; i != sz; ++i) {
          evaluate_value(args[i]);
          if (fn_arg->is_variadic()) {
            J_ASSERT(is_variadic && num_variadic && j < num_variadic);
            varargs[j++] = convert_value(static_cast<te_value_source &&>(args[i]), fn_arg->type).value;
          } else {
            sub.set(fn_arg->name, convert_value(static_cast<te_value_source &&>(args[i]), fn_arg->type).value);
            ++fn_arg;
          }
        }
        if (is_variadic) {
          J_ASSERT(fn_arg->is_variadic());
          sub.set(fn_arg->name, attr_value(span(varargs, num_variadic)));
        }

        te_value_source result = make_opaque(overload->return_type, attr_value(mod->expand(overload->tpl, sub)), overload->precedence);
        return opts.type ? convert_value(static_cast<te_value_source &&>(result), *opts.type) : static_cast<te_value_source &&>(result);
      }

      [[nodiscard]] te_value_source compile(term_expr * J_NOT_NULL te, compile_opts opts = {}) {
        auto g = scope.enter_term_expr(te);
        te_value_source result = visit(te, *this, opts);
        if (auto prec = precs.find_precompute(te)) {
          if (!prec->value) {
            evaluate_value(result);
            prec->value = result.value.as_str();
          }
          result.value = attr_value(prec->name);
        }
        return result;
      }
    };
  }


  void term_expr::resolve(const pat_captures & caps, expr_scope & scope, id_resolver * J_NOT_NULL id_resolver, term_expr_use use) {
    J_ASSERT(type != term_expr_type::none);
    resolver r{caps, scope, id_resolver};
    visit_with_expr(this, r, use);
  }

  void term_expr::resolve(expr_scope & scope, id_resolver * J_NOT_NULL id_resolver, term_expr_use use) {
    resolve(pat_captures::empty, scope, id_resolver, use);
  }

  void term_expr::compute_hash(expr_scope & scope) {
    hasher h{scope};
    visit_with_expr(this, h);
  }

  const te_function & te_member_fn_call::find_overload(expr_scope & scope) const {
    te_type types[args.size() + 1];
    types[0] = val->get_type(scope);
    if (!types[0].is_val_type()) {
      scope.throw_expr_error("Type is not a val type.");
    }
    i32_t i = 1;
    for (term_expr * arg : args) {
      types[i++] = arg->get_type(scope);
    }
    return types[0].value_type->fns.at(fn_name).find_overload(span(types, i));
  }

  [[nodiscard]] const te_function & te_global_fn_call::find_overload(expr_scope & scope) const {
    const u32_t sz = args.size();
    const te_fn_set * fns = scope.root->mod->te_functions.maybe_at(fn_name);
    J_ASSERT_NOT_NULL(fns, "No function '{}'.", fn_name);
    te_type arg_types[sz];
    for (u32_t i = 0U; i < sz; ++i) {
      arg_types[i] = args[i]->get_type(scope);
    }
    return fns->find_overload(span(arg_types, sz));
  }

  te_type term_expr::get_type(expr_scope & scope) {
    type_getter r{scope, scope.root->mod};
    return visit(this, r);
  }

  void term_expr::clear_arg_nt(term_expr_use use) noexcept {
    nt_clearer r{};
    visit_with_expr(this, r, use);
  }

  void term_expr::clear() noexcept {
    if (type == term_expr_type::none) {
      return;
    }
    visit(this, []<typename T>(T & f) { f.~T(); });
    type = term_expr_type::none;
  }

  [[nodiscard]] attr_value term_expr::compile(expr_scope & scope, precomputes & precs, te_type type) {
    auto g = scope.enter_term_expr(this);
    compiler c{scope, precs};
    return c.to_struct(c.compile(this, &type));
  }

  [[nodiscard]] te_value_source term_expr::compile_raw(expr_scope & scope, precomputes & precs) {
    auto g = scope.enter_term_expr(this);
    compiler c{scope, precs};
    te_value_source src = c.compile(this);
    c.evaluate_value(src);
    return static_cast<te_value_source &&>(src);
  }

  [[nodiscard]] te_value_source term_expr::compile_raw(expr_scope & scope, precomputes & precs, te_type type) {
    auto g = scope.enter_term_expr(this);
    compiler c{scope, precs};
    te_value_source src = c.compile(this, &type);
    c.evaluate_value(src);
    return static_cast<te_value_source &&>(src);
  }

  [[nodiscard]] attr_value term_expr::compile(expr_scope & scope, precomputes & precs) {
    auto g = scope.enter_term_expr(this);
    compiler c{scope, precs};
    return c.to_struct(c.compile(this));
  }

  [[nodiscard]] attr_value term_expr::compile(expr_scope & scope, te_type type) {
    precomputes precs;
    return compile(scope, precs, type);
  }

  [[nodiscard]] attr_value term_expr::compile(expr_scope & scope)  {
    precomputes precs;
    return compile(scope, precs);
  }

  [[nodiscard]] strings::string term_expr::compile_str(expr_scope & scope, precomputes & precs, te_type type) {
    return compile_raw(scope, precs, type).as_str(-1);
  }
  [[nodiscard]] strings::string term_expr::compile_str(expr_scope & scope, precomputes & precs) {
    return compile_raw(scope, precs).as_str(-1);
  }

  [[nodiscard]] strings::string term_expr::compile_str(expr_scope & scope, te_type type) {
    precomputes precs;
    return compile_str(scope, precs, type);
  }

  [[nodiscard]] strings::string term_expr::compile_str(expr_scope & scope) {
    precomputes precs;
    return compile_str(scope, precs);
  }

  [[nodiscard]] bool te_term_construct::operator==(const te_term_construct & rhs) const noexcept {
    return term == rhs.term && (term_type_expr ? rhs.term_type_expr && *term_type_expr == *rhs.term_type_expr : !rhs.term_type_expr) && args == rhs.args;
  }

  [[nodiscard]] bool te_prop_read::operator==(const te_prop_read & rhs) const noexcept {
    return prop_name == rhs.prop_name && *val == *rhs.val;
  }

  [[nodiscard]] bool te_ternary::operator==(const te_ternary & rhs) const noexcept {
    return *condition == *rhs.condition && *true_branch == *rhs.true_branch && *false_branch == *rhs.false_branch;
  }
  [[nodiscard]] bool te_binop::operator==(const te_binop & rhs) const noexcept {
    return type == rhs.type && *lhs == *rhs.lhs && *this->rhs == *rhs.rhs;
  }

  [[nodiscard]] bool te_member_fn_call::operator==(const te_member_fn_call & rhs) const noexcept {
    return *val == *rhs.val && fn_name == rhs.fn_name && args == rhs.args;
  }

  [[nodiscard]] bool te_global_fn_call::operator==(const te_global_fn_call & rhs) const noexcept {
    return fn_name == rhs.fn_name && args == rhs.args;
  }

  [[nodiscard]] bool operator==(const term_exprs & lhs, const term_exprs & rhs) noexcept {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    const term_expr * const * l = lhs.begin();
    for (const term_expr * r : rhs) {
      if (**l != *r) {
        return false;
      }
      l++;
    }
    return true;
  }

  [[nodiscard]] bool term_expr::operator==(const term_expr & rhs) const noexcept {
    if (type != rhs.type) {
      return false;
    }
    using enum term_expr_type;
    switch (type) {
    case none: return true;
    case arg: return this->arg == rhs.arg;
    case term_construct: return this->term_construct == rhs.term_construct;
    case constant: return this->constant.value == rhs.constant.value && this->constant.type == rhs.constant.type;
    case prop_read: return this->prop_read == rhs.prop_read;
    case member_fn_call: return this->member_fn_call == rhs.member_fn_call;
    case global_fn_call: return this->global_fn_call == rhs.global_fn_call;
    case ternary: return this->ternary == rhs.ternary;
    case binop: return this->binop == rhs.binop;
    }
  }
  [[nodiscard]] arg_counts term_expr::collect_args() const noexcept {
    arg_collector v{};
    visit(this, v);
    return static_cast<arg_counts &&>(v.args);
  }

  [[nodiscard]] pair<i32_t> term_expr::extent() const noexcept {
    extent_calc d{};
    visit(this, d);
    return d.extent;
  }

  void dump(dump_context & ctx, const term_expr * e) noexcept {
    dumper d{ctx};
    visit(e, d);
  }

  [[nodiscard]] term_expr * copy(const term_expr * J_NOT_NULL te, copy_cb_t cb, void * userdata, source_location loc) noexcept {
    J_ASSERT(te->type != term_expr_type::none);
    copier c{cb, userdata};

    term_expr * res = visit(te, c);
    res->hash = te->hash;
    res->loc = loc ? loc : te->loc;
    if (cb) {
      cb(te, res, userdata);
    }
    return res;
  }

  void arg_counts::add(const reduction_arg & arg, i32_t num) noexcept {
    counts.emplace(arg, 0).first->second += num;
  }
  arg_counts & arg_counts::operator+=(const arg_counts & rhs) noexcept {
    for (auto & c : rhs.counts) {
      add(c.first, c.second);
    }
    return *this;
  }
  void arg_counts::filter_min(i32_t min) noexcept {
    for (auto it = counts.begin(), e = counts.end(); it != e;) {
      if (it->second < min) {
        it = counts.erase(it);
      } else {
        ++it;
      }
    }
  }

  [[nodiscard]] precomputes arg_counts::as_precomputes(const module & mod) const noexcept {
    precomputes result{.args{counts.size()}};
    for (auto it = counts.begin(), e = counts.end(); it != e; ++it) {
      s::string name;
      s::const_string_view type;
      switch (it->first.type) {
      case arg_type::none:
      case arg_type::capture_term_field:
      case arg_type::reference: J_FAIL("Unsupported arg");
      case arg_type::capture_term:
        name = s::format("st{}", it->first.stack_offset);
        type = mod.trs_term_type->type_name;
        break;
      case arg_type::select_term_ahead:
        name = s::format("it{}", -it->first.stack_offset);
        type = mod.trs_term_type->type_name;
        break;
      case arg_type::capture_reduction_copy:
        name = s::format("r{}", it->first.stack_offset);
        type = it->first.data_type->name;
        break;
      case arg_type::capture_this_reduction_nt:
      case arg_type::capture_reduction_copy_nt:
        name = s::format("r{}", it->first.stack_offset);
        type = it->first.nt->data_type->name;
        break;
      case arg_type::capture_reduction_copy_sz:
        continue;
      case arg_type::capture_reduction_field:
        name = s::format("tr{}_{}", it->first.stack_offset, it->first.field_offset);
        type = it->first.value_type->type_name;
        break;
      }
      result.add_precompute(it->first, static_cast<s::string &&>(name), type);
    }
    return result;
  }

  precompute * precomputes::find_precompute(term_expr * J_NOT_NULL te) noexcept {
    auto prec = te->type == term_expr_type::arg ? args.maybe_at(te->arg) : tes.maybe_at(te);
    return prec ? *prec : nullptr;
  }
  precompute * precomputes::find_precompute(const reduction_arg & arg) noexcept {
    auto prec = args.maybe_at(arg);
    return prec ? *prec : nullptr;
  }

  void precomputes::add_precompute(const reduction_arg & arg, strings::string && name, strings::string && type) {
    auto prec = ::new precompute{static_cast<s::string &&>(name), static_cast<s::string &&>(type), {}};
    all.push_back(prec);
    args.emplace(arg, prec);
  }

  void precomputes::add_precompute(const reduction_arg & arg, strings::string && name, strings::string && type, strings::string && value) noexcept {
    auto prec = ::new precompute{static_cast<s::string &&>(name), static_cast<s::string &&>(type), static_cast<s::string &&>(value)};
    all.push_back(prec);
    args.emplace(arg, prec);
  }

  void precomputes::add_precompute(term_expr * J_NOT_NULL te, strings::string && name, strings::string && type) {
    auto prec = ::new precompute{static_cast<s::string &&>(name), static_cast<s::string &&>(type), {}};
    all.push_back(prec);
    tes.emplace(te, prec);
    if (te->type == term_expr_type::arg) {
      args.emplace(te->arg, prec);
    }
  }

  void precomputes::add_precompute(term_expr * J_NOT_NULL te, strings::string && name, strings::string && type, strings::string && value) noexcept {
    auto prec = ::new precompute{static_cast<s::string &&>(name), static_cast<s::string &&>(type), static_cast<s::string &&>(value)};
    all.push_back(prec);
    tes.emplace(te, prec);
    if (te->type == term_expr_type::arg) {
      args.emplace(te->arg, prec);
    }
  }
}
