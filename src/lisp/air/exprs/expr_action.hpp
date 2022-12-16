#pragma once

#include "logging/global.hpp"

#include "lisp/air/exprs/expr_source.hpp"

namespace j::lisp::air::exprs {
  enum class expr_action_type : u8_t {
    /// Empty action, with no criteria.
    none = 0U,

    /// Check the criterion with no action.
    check,

    /// Remove the expression.
    remove,
    /// Filter inputs by condition.
    filter_inputs,
    /// Replace the expression with target expression.
    replace,
    /// Merge expression with target
    merge,
    /// Append source expression to target
    push_back,
    /// Prepend source expression to target
    push_front,
    /// Change expression type without touching inputs or statics.
    change_expr_type,
    /// Wrap in another expression.
    wrap_in_expr,
    wrap_inputs_in_expr,
    merge_inputs_in_expr,
  };

  struct resolved_action final {
    expr * target_expr = nullptr;
    expr * source_expr = nullptr;
    expr * parent_expr = nullptr;
    const struct expr_action * action = nullptr;

    explicit operator bool() const noexcept {
      return target_expr;
    }
    bool operator!() const noexcept {
      return !target_expr;
    }

    expr * J_NOT_NULL run(expr_builder & builder);
    void do_wrap_single(expr_builder & builder);
    void do_wrap_inputs(expr_builder & builder);

    u8_t get_merged_exprs(expr ** J_NOT_NULL exprs, bool flatten_target, bool flatten_source) {
      u8_t num_exprs = 0U;
      u8_t num_target_before = flatten_target ? target_expr->num_inputs : 1U;
      u8_t num_source = source_expr ? (flatten_source ? source_expr->num_inputs : 1U) : 0U;
      u8_t num_target_after = 0U;
      if (source_expr && flatten_source && source_expr->parent == target_expr) {
        J_ASSERT(flatten_target);
        num_target_after = num_target_before - source_expr->parent_pos - 1U;
        num_target_before = source_expr->parent_pos;
      }
      if (flatten_target) {
        for (; num_exprs < num_target_before; ++num_exprs) {
          exprs[num_exprs] = target_expr->input(num_exprs).expr;
        }
      } else {
        exprs[num_exprs++] = target_expr;
      }
      if (source_expr && flatten_source) {
        for (u8_t i = 0; i < num_source; ++i) {
          exprs[num_exprs++] = source_expr->input(i).expr;
        }
      } else if (source_expr) {
        exprs[num_exprs++] = source_expr;
      }
      for (u8_t i = 0; i < num_target_after; ++i) {
        exprs[num_exprs++] = target_expr->input(i + num_target_before + 1U).expr;
      }
      return num_exprs;
    }
  };

  struct resolved_action_set final {
    resolved_action actions[4] = {};
    u8_t num_actions = 0U;
    exprs::expr * parent = nullptr;
    u8_t parent_depth = 0U;

    static u8_t get_expr_depth(expr * J_NOT_NULL e) noexcept {
      u8_t result = 0;
      for (; e->parent; e = e->parent, ++result) { }
      return result;
    }

    bool add_action(const resolved_action & action) noexcept;

    J_RETURNS_NONNULL expr * run(expr_builder & builder) {
      J_ASSUME(num_actions);
      for (u8_t i = 0U; i < num_actions; ++i) {
        actions[i].run(builder);
      }
      num_actions = 0U;
      return parent;
    }

    explicit operator bool() const noexcept {
      return num_actions;
    }
    bool operator!() const noexcept {
      return !num_actions;
    }
  };

  struct expr_action_set;

  struct expr_action final {
    expr_action_type type = expr_action_type::none;
    expr_type target_type = expr_type::none;
    expr_path target{};
    expr_source source{};

    template<typename T>
    inline expr_action_set then(T && act) const noexcept;

    resolved_action try_resolve(expr * J_NOT_NULL cur) const noexcept {
      if (type == expr_action_type::none) {
        return {};
      }
      expr * action_target = target.traverse(cur);
      if (!action_target) {
        return {};
      }
      expr * action_parent = action_target->parent ? action_target->parent : action_target;
      expr * action_source = nullptr;
      J_ASSUME_NOT_NULL(action_parent);
      switch (type) {
      case expr_action_type::check:
      case expr_action_type::remove:
      case expr_action_type::change_expr_type:
      case expr_action_type::wrap_in_expr:
      case expr_action_type::wrap_inputs_in_expr:
        break;
      case expr_action_type::filter_inputs: {
        J_ASSUME(source.type == expr_source_type::path);
        bool had = false;
        for (auto & in : action_target->inputs()) {
          if (source.path.traverse(in.expr)) {
            had = true;
            break;
          }
        }
        if (!had) {
          return {};
        }
        break;
      }
      case expr_action_type::push_back:
      case expr_action_type::push_front:
      case expr_action_type::replace:
        if (source.type == expr_source_type::path) {
          action_source = source.path.traverse(cur);
          if (!action_source) {
            return {};
          }
        }
        break;
      case expr_action_type::merge:
      case expr_action_type::merge_inputs_in_expr:
        J_ASSUME(source.type == expr_source_type::path);
        action_source = source.path.traverse(cur);
        if (!action_source) {
          return {};
        }
        break;
      case expr_action_type::none:
        J_UNREACHABLE();
      }

      return {
        action_target,
        action_source,
        action_parent,
        this,
      };
    }
  };

  inline void resolved_action::do_wrap_single(expr_builder & builder) {
    switch (action->target_type) {
    case expr_type::ineg:
      builder.wrap_with(target_expr, expr_type::ineg);
      break;
    case expr_type::lnot:
      builder.wrap_with(target_expr, expr_type::lnot);
      break;
    case expr_type::idiv:
      builder.wrap_with_var(target_expr, expr_type::idiv);
      break;
    case expr_type::imul:
      builder.wrap_with_var(target_expr, expr_type::imul);
      break;
    default:
      J_FAIL("Unsupported wrapper");
    }
  }

  inline void resolved_action::do_wrap_inputs(expr_builder & builder) {
    expr * exprs[max(target_expr->num_inputs, 1) + (source_expr ? max(source_expr->num_inputs, 1) : 0U)];
    bool flatten_source = false, flatten_target = false;
    if (action->type == expr_action_type::merge) {
      J_ASSUME_NOT_NULL(source_expr);
      flatten_source = true;
      flatten_target = true;
    } else if (action->type == expr_action_type::merge_inputs_in_expr) {
      J_ASSUME_NOT_NULL(source_expr);
      flatten_source = source_expr && source_expr->type == action->target_type;
      flatten_target = target_expr->type == action->target_type;
    } else if (action->type == expr_action_type::wrap_in_expr) {
      J_ASSERT(!source_expr);
    }
    u8_t num_exprs = get_merged_exprs(exprs, flatten_target, flatten_source);
    auto parent = target_expr->parent;
    auto parent_pos = target_expr->parent_pos;
    auto inputs = span(exprs, num_exprs);
    // It is important to erase the node before adding it. Otherwise, stale data would be left in the tree.
    // Target need not be deleted, since it is replaced in the tree.
    if (source_expr) {
      builder.erase(*source_expr);
    }
    switch (action->type == expr_action_type::merge ? target_expr->type : action->target_type) {
    case expr_type::vec:
      builder.create_late(parent, parent_pos, expr_type::vec, inputs);
      break;
    case expr_type::progn:
      builder.create_late(parent, parent_pos, expr_type::progn, inputs);
      break;
    case expr_type::iadd:
      builder.create_late(parent, parent_pos, expr_type::iadd, inputs);
      break;
    case expr_type::imul:
      builder.create_late(parent, parent_pos, expr_type::imul, inputs);
      break;
    case expr_type::vec_append:
      builder.create_late(parent, parent_pos, expr_type::vec_append, inputs);
      break;
    case expr_type::let: {
      auto target_vars = reinterpret_cast<air_var_defs_t*>(target_expr->static_begin());
      J_ASSUME_NOT_NULL(source_expr);
      auto source_vars = reinterpret_cast<air_var_defs_t*>(source_expr->static_begin());
      auto let = builder.create_late(parent, parent_pos, expr_type::let, inputs, exprs::alloc_vars(target_vars->size() + source_vars->size()));
      auto cur = reinterpret_cast<air_var_defs_t*>(let->static_begin())->begin();
      for (auto v : *target_vars) {
        *cur++ = v;
      }
      for (auto v : *source_vars) {
        *cur++ = v;
      }
      J_ASSERT(let->num_inputs == inputs.size());
      break;
    }
    default:
      J_FAIL("Unsupported wrapper");
    }
  }

  expr * J_NOT_NULL resolved_action::run(expr_builder & builder) {
    switch (action->type) {
    case expr_action_type::check:
      break;
    case expr_action_type::remove:
      builder.erase(*target_expr);
      break;
    case expr_action_type::filter_inputs: {
      for (u8_t i = 0U; i < target_expr->num_inputs;) {
        expr * in = target_expr->input(i).expr;
        if (action->source.path.traverse(in)) {
          builder.erase(*in);
        } else {
          ++i;
        }
      }
      break;
    }
    case expr_action_type::change_expr_type:
      target_expr->type = action->target_type;
      break;
    case expr_action_type::wrap_inputs_in_expr: {
      J_ASSERT(!source_expr);
      auto e = builder.create_empty(action->target_type, target_expr->num_inputs + 1U);
      J_ASSERT(e);
      e = builder.try_append_inputs_to(e, target_expr);
      J_ASSERT(e->type == action->target_type);
      J_ASSERT(e);
      target_expr->num_inputs = 1U;
      builder.attach_to(target_expr, 0, e);
      break;
    }
    case expr_action_type::wrap_in_expr:
      do_wrap_single(builder);
      break;
    case expr_action_type::push_back:
      if (!source_expr) {
        source_expr = action->source.get(builder, target_expr);
      }
      J_REQUIRE(builder.try_append_input_to(target_expr, source_expr), "Could not append");
      break;
    case expr_action_type::push_front:
      if (!source_expr) {
        source_expr = action->source.get(builder, target_expr);
      }
      J_DEBUG("Prepend {} to {}", source_expr->type, target_expr->type);
      J_REQUIRE(builder.try_prepend_input_to(target_expr, source_expr), "Could not prepend");
      break;
    case expr_action_type::merge:
    case expr_action_type::merge_inputs_in_expr:
      do_wrap_inputs(builder);
      break;
    case expr_action_type::replace:
      if (!source_expr) {
        source_expr = action->source.get(builder, parent_expr);
      } else if (source_expr->parent) {
        builder.erase(*source_expr);
      }
      builder.attach_to(target_expr->parent, target_expr->parent_pos, source_expr);
      break;
    case expr_action_type::none:
      J_UNREACHABLE();
    }
    return parent_expr;
  }

  inline bool resolved_action_set::add_action(const resolved_action & action) noexcept {
      if (!action) {
        num_actions = 0U;
        return false;
      }
      if (action.action->type == expr_action_type::check) {
        return true;
      }
      actions[num_actions++] = action;
      u8_t depth = get_expr_depth(action.parent_expr);
      if (!parent || parent_depth < depth) {
        parent = action.parent_expr;
        parent_depth = depth;
      }
      return true;
    }

  struct expr_action_set final {
    expr_action actions[4] = {};
    u8_t num_actions = 0U;
    const char * name = nullptr;

    template<typename... Ts>
    inline expr_action_set(const Ts & ... acts) noexcept
      : actions{acts...},
        num_actions(sizeof...(Ts))
    {
    }

    template<typename T>
    inline expr_action_set then(T && act) const noexcept {
      expr_action_set result{*this};
      result.actions[result.num_actions++] = static_cast<T &&>(act);
      return result;
    }

    resolved_action_set try_resolve(expr * J_NOT_NULL cur) const noexcept {
      J_ASSUME(num_actions);
      resolved_action_set result;
      for (u8_t i = 0U; i < num_actions; ++i) {
        if (!result.add_action(actions[i].try_resolve(cur))) {
          break;
        }
      }
      return result;
    }
  };

  struct substitution final {
    const char * name;
    expr_action_set set;
  };

  template<typename T>
  inline expr_action_set expr_action::then(T && act) const noexcept {
    return {*this, static_cast<T &&>(act)};
  }

  inline expr_action replace_with(const expr_path & path, const expr_source & src) noexcept {
    return expr_action{expr_action_type::replace, expr_type::none, path, src};
  }

  inline expr_action replace_with(const expr_source & src) noexcept {
    return expr_action{expr_action_type::replace, expr_type::none, self(), src};
  }

  template<typename T>
  inline expr_action push_back(const expr_path & path, T && src) noexcept {
    return expr_action{expr_action_type::push_back, expr_type::none, path, static_cast<T &&>(src)};
  }

  template<typename T>
  inline expr_action push_back(T && src) noexcept {
    return expr_action{expr_action_type::push_back, expr_type::none, self(), static_cast<T &&>(src)};
  }

  template<typename T>
  inline expr_action push_front(const expr_path & path, T && src) noexcept {
    return expr_action{expr_action_type::push_front, expr_type::none, path, static_cast<T &&>(src)};
  }

  template<typename T>
  inline expr_action push_front(T && src) noexcept {
    return expr_action{expr_action_type::push_front, expr_type::none, self(), static_cast<T &&>(src)};
  }

  inline expr_action merge_with(const expr_path & path, const expr_path & src) noexcept {
    return expr_action{expr_action_type::merge, expr_type::none, path, src};
  }

  inline expr_action erase() noexcept {
    return expr_action{expr_action_type::remove, expr_type::none, self()};
  }

  inline expr_action erase(const expr_path & path) noexcept {
    return expr_action{expr_action_type::remove, expr_type::none, path};
  }

  inline expr_action filter_inputs(const expr_path & input_path) noexcept {
    return expr_action{expr_action_type::filter_inputs, expr_type::none, self(), input_path};
  }

  inline expr_action check(const expr_path & path) noexcept {
    return expr_action{expr_action_type::check, expr_type::none, path};
  }

  inline expr_action change_expr_type(const expr_path & path, expr_type new_type) noexcept {
    return expr_action{expr_action_type::change_expr_type, new_type, path, {}};
  }

  inline expr_action change_expr_type(expr_type new_type) noexcept {
    return expr_action{expr_action_type::change_expr_type, new_type, self(), {}};
  }

  inline expr_action wrap_in_expr(const expr_path & path, expr_type new_type) noexcept {
    return expr_action{expr_action_type::wrap_in_expr, new_type, path, {}};
  }

  inline expr_action wrap_in_expr(expr_type new_type) noexcept {
    return expr_action{expr_action_type::wrap_in_expr, new_type, self(), {}};
  }

  inline expr_action wrap_inputs_in_expr(const expr_path & path, expr_type new_type) noexcept {
    return expr_action{expr_action_type::wrap_inputs_in_expr, new_type, path, {}};
  }

  inline expr_action wrap_inputs_in_expr(expr_type new_type) noexcept {
    return expr_action{expr_action_type::wrap_inputs_in_expr, new_type, self(), {}};
  }

  inline expr_action wrap_in_associative_expr(const expr_path & path, const expr_path & src, expr_type new_type) noexcept {
    return expr_action{expr_action_type::merge_inputs_in_expr, new_type, path, src};
  }

  inline expr_action wrap_in_associative_expr(const expr_path & src, expr_type new_type) noexcept {
    return expr_action{expr_action_type::merge_inputs_in_expr, new_type, self(), src};
  }
}
