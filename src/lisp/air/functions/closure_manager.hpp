#pragma once

#include "mem/bump_pool.hpp"
#include "lisp/air/functions/air_closure.hpp"
#include "lisp/air/functions/air_function.hpp"

namespace j::lisp::air::inline functions {
  struct closure_manager final {
    j::mem::bump_pool & info_pool;
    analysis::lexical_scope_manager & scope;
    air_function * air_fn = nullptr;
    air_closure * current_closure = nullptr;

    J_RETURNS_NONNULL air_closure * enter_closure(lisp::functions::func_info * J_NOT_NULL func_info,
                                                  symbols::symbol * J_NOT_NULL sym) {
      J_ASSUME_NOT_NULL(air_fn);
      scope.enter_closure();
      air_fn->max_closure_depth = max(air_fn->max_closure_depth, scope.depth);
      info_pool.align(8);
      current_closure = &info_pool.emplace_with_padding<air_closure>(
        sizeof(closure_scope) * scope.depth,
        scope.depth,
        func_info,
        sym,
        current_closure);

      air_fn->num_closures++;
      if (air_fn->last_closure) {
        air_fn->last_closure->next_closure = current_closure;
      } else {
        air_fn->first_closure = current_closure;
      }
      air_fn->last_closure = current_closure;

      closure_scope * const first = reinterpret_cast<closure_scope*>(current_closure + 1);
      analysis::lexical_scope * cur_lex = scope.cur_scope;

      for (closure_scope * cur = first + scope.depth - 1; cur >= first;
           --cur,
           cur_lex = cur_lex->parent)
      {
        J_ASSUME_NOT_NULL(cur_lex);
        ::new (cur) closure_scope{
          cur_lex->expr,
          nullptr,
          cur_lex->loop_depth,
          cur_lex->closure_depth,
        };
      }

      return current_closure;
    }

    void handle_bound_wr(const analysis::var_with_scope & v) {
      J_ASSUME_NOT_NULL(current_closure);
      auto & scope = current_closure->scopes()[v.depth];
      if (scope.contains(v.var)) {
        return;
      }
      v.var->is_bound = 1U;
      scope.first_binding = &info_pool.emplace<closure_binding>(v.var, scope.first_binding);
      current_closure->num_bindings++;
      current_closure->max_binding_depth = max(current_closure->max_binding_depth, v.depth);
    }

    void handle_bound_rd(const analysis::var_with_scope & v) {
      handle_bound_wr(v);
    }

    void exit_closure() noexcept {
      J_ASSUME_NOT_NULL(current_closure);
      current_closure = current_closure->parent_closure;
      scope.exit_closure();
    }
  };
}
