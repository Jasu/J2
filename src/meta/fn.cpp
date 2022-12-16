#include "fn.hpp"
#include "meta/expr_fn.hpp"
#include "meta/expr.hpp"
#include "meta/expr_scope.hpp"

namespace j::meta {
  namespace {
    attr_value invoke_fn(const expr_fn & efn, expr_scope & scope, attr_value * args, u32_t nargs) {
      expr_scope ch(&scope);
      const fn * f = (const fn *)efn.userdata;
      for (u8_t i = 0U; i < efn.num_args; ++i) {
        J_ASSERT(f->args[i]);
        if (efn.arg_flags[i] & arg_rest) {
          ch.set(f->args[i], attr_value(span<attr_value>(args + i, nargs - i)));
        } else {
          ch.set(f->args[i], attr_value(args[i]));
        }
      }
      return eval_expr(ch, f->e);
    }
  }

  fn::fn(strings::const_string_view name, doc_comment && comment) noexcept
    : node(node_fn, name, static_cast<doc_comment &&>(comment))
  { }

  expr_fn fn::to_expr_fn() const noexcept {
    expr_fn result{
      {attr_mask_none},
      {arg_required},
      num_args,
      num_args,
      invoke_fn,
      (void*)this,
    };
    for (u8_t i = 0U; i < num_args; ++i) {
      result.arg_types[i] = attr_mask_any;
    }
    if (has_rest) {
      result.arg_flags[num_args - 1] = arg_rest;
      result.num_required_args--;
    }
    return result;
  }

  void fn::push_arg(strings::const_string_view name, bool is_rest) {
    J_ASSERT(name);
    J_ASSERT(num_args < 4);
    J_ASSERT(!has_rest);
    has_rest = is_rest;
    args[num_args] = name;
    ++num_args;
  }
}
