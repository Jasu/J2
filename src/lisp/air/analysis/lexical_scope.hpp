#pragma once

#include "lisp/air/air_var.hpp"
#include "lisp/air/exprs/expr.hpp"

namespace j::lisp::air::analysis {
  struct lexical_scope_manager;

  struct lexical_scope final {
    air_var_ptrs_t vars;
    lexical_scope_manager & manager;
    lexical_scope * parent = nullptr;
    exprs::expr * expr = nullptr;
    u32_t depth = 0U;
    u8_t loop_depth = 0U;
    u8_t closure_depth = 0U;

    lexical_scope(lexical_scope_manager & manager, exprs::expr * J_NOT_NULL expr) noexcept;

    ~lexical_scope();
  };

  struct var_with_scope final {
    air_var * var = nullptr;
    exprs::expr * scope = nullptr;
    u32_t depth = 0U;
    u8_t loop_depth = 0U;
    u8_t closure_depth = 0U;
    bool is_bound = false;

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return var;
    }
    J_INLINE_GETTER bool operator!() const noexcept {
      return !var;
    }
    J_INLINE_GETTER bool empty() const noexcept {
      return !var;
    }
    J_INLINE_GETTER bool operator==(const var_with_scope & rhs) const noexcept {
      return rhs.var == var;
    }
  };

  struct lexical_scope_manager final {
    lexical_scope * cur_scope = nullptr;
    u32_t depth = 0U;
    u8_t loop_depth = 0U;
    u8_t closure_depth = 0U;

    air_var * try_find_var(id name) const noexcept {
      for (lexical_scope * s = cur_scope; s; s = s->parent) {
        for (auto v : s->vars) {
          if (v->name == name) {
            return v;
          }
        }
      }
      return nullptr;
    }

    var_with_scope try_find_var_with_scope(id name) const noexcept {
      for (lexical_scope * s = cur_scope; s; s = s->parent) {
        for (auto & v : s->vars) {
          if (v->name == name) {
            return {
              .var = v,
              .scope = s->expr,
              .depth = s->depth,
              .loop_depth = s->loop_depth,
              .closure_depth = s->closure_depth,
              .is_bound = s->closure_depth != closure_depth,
            };
          }
        }
      }
      return {};
    }
    J_ALWAYS_INLINE void enter_closure() noexcept {
      ++closure_depth;
    }

    J_ALWAYS_INLINE void exit_closure() noexcept {
      J_ASSUME(closure_depth);
      --closure_depth;
    }

    J_ALWAYS_INLINE void enter_loop() noexcept {
      ++loop_depth;
    }

    J_ALWAYS_INLINE void exit_loop() noexcept {
      J_ASSUME(loop_depth);
      --loop_depth;
    }

    lexical_scope enter_scope(exprs::expr * J_NOT_NULL expr) noexcept {
      return { *this, expr };
    }
  };

  inline lexical_scope::lexical_scope(lexical_scope_manager & manager, exprs::expr * J_NOT_NULL expr) noexcept
    : manager(manager),
      parent(manager.cur_scope),
      expr(expr),
      depth(manager.depth++),
      loop_depth(manager.loop_depth),
      closure_depth(manager.closure_depth)
  {
    manager.cur_scope = this;
    J_ASSUME(expr->type == expr_type::let || expr->type == expr_type::fn_body);

    auto * const defs = reinterpret_cast<air_var_defs_t*>(expr->static_begin());
    vars = {
      defs->begin(),
      (i32_t)defs->size(),
    };
  }

  inline lexical_scope::~lexical_scope() {
    J_ASSERT(manager.cur_scope == this);
    --manager.depth;
    manager.cur_scope = parent;
  }
}
