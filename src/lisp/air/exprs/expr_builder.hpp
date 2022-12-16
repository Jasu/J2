#pragma once

#include "lisp/common/metadata_init.hpp"
#include "lisp/air/functions/air_function.hpp"
#include "lisp/air/exprs/write_expr.hpp"

namespace j::lisp::air::exprs {
  struct expr_builder {
    j::mem::bump_pool * pool = nullptr;
    air_function & fn;

    template<typename... Args>
    [[nodiscard]] J_NO_DEBUG J_RETURNS_NONNULL inline air::exprs::expr * create(expr_type type, metadata_init md, Args && ... args) const {
      const u32_t sz = md.size();
      auto result = create_pad(sz, type, static_cast<Args &&>(args)...);
      if (sz) {
        write_expr_metadata(result, md);
      }
      return result;
    }

    template<typename... Args>
    J_A(AI,ND,NODISC,RNN) inline air::exprs::expr * create(expr_type type, Args && ... args) const {
      return create_pad(0U, type, static_cast<Args &&>(args)...);
    }

    template<typename... Args>
    [[nodiscard]] J_RETURNS_NONNULL air::exprs::expr * create_pad(u32_t pad, expr_type type, Args && ... args) const {
      expr * result = air::exprs::write_expr(type, *pool, pad, static_cast<Args &&>(args)...);
      result->visit_flag = fn.visit_flag_value;
      fn.counts.add_expr(result->type);
      return result;
    }

    void attach_to(air::exprs::expr * J_NOT_NULL parent,
                   u32_t index,
                   air::exprs::expr * J_NOT_NULL child) const noexcept {
      J_ASSUME(parent != child);
      J_ASSUME(index < parent->num_inputs);
      child->parent_pos = index;
      child->parent = parent;
      child->visit_flag = fn.visit_flag_value;
      parent->input(index).expr = child;
    }

    template<typename... Args>
    J_RETURNS_NONNULL air::exprs::expr * create_late(
      air::exprs::expr * J_NOT_NULL parent,
      u32_t index,
      expr_type type,
      Args && ... args) const {
      auto expr = create(type, static_cast<Args &&>(args)...);
      attach_to(parent, index, expr);
      return expr;
    }

    template<typename... Args>
    J_RETURNS_NONNULL air::exprs::expr * wrap_with(exprs::expr * J_NOT_NULL target, expr_type type, Args && ... args) const {
      auto parent = target->parent;
      J_ASSUME_NOT_NULL(parent);
      u8_t parent_pos = target->parent_pos;
      auto result = create(type, copy_metadata(target->metadata()), target, static_cast<Args &&>(args)...);
      result->parent_pos = parent_pos;
      result->parent = parent;
      parent->input(parent_pos).expr = result;
      return result;
    }

    template<typename... Args>
    J_RETURNS_NONNULL air::exprs::expr * wrap_with_var(exprs::expr * J_NOT_NULL target, expr_type type, Args && ... args) const {
      auto parent = target->parent;
      J_ASSUME_NOT_NULL(parent);
      u8_t parent_pos = target->parent_pos;
      auto result = create(type, copy_metadata(target->metadata()), span(&target, 1), static_cast<Args &&>(args)...);
      result->parent_pos = parent_pos;
      result->parent = parent;
      parent->input(parent_pos).expr = result;
      return result;
    }

    void create_nil(expr & e, u32_t i) {
      J_ASSERT(i < e.num_inputs);
      auto result = create(expr_type::nop, copy_metadata(e.metadata()));
      result->parent_pos = i;
      result->parent = &e;
      e.input(i).expr = result;
    }

    void shift_inputs_right(expr & e, u32_t i) noexcept {
      J_ASSERT(i < e.num_inputs);
      auto beg = e.inputs_begin() + i, end = e.inputs_end();
      if (beg != end) {
        ::j::memmove(beg + 1U, beg, byte_offset(beg, end));
        for (++beg; beg != end; ++beg) {
          beg->expr->parent_pos = i++;
        }
      }
    }

    void erase_operand(expr & e, u32_t i, bool branch_check = true) noexcept {
      J_ASSERT(i < e.num_inputs);
      fn.counts.remove_expr(e.input(i).expr->type);
      if (branch_check && has_fixed_branches(e.type)) {
        create_nil(e, i);
        return;
      }
      --e.num_inputs;
      auto beg = e.inputs_begin() + i, end = e.inputs_end();
      if (beg != end) {
        ::j::memmove(beg, beg + 1U, byte_offset(beg, end));
      }
      u32_t index = 0;
      for (auto & i : e.inputs()) {
        i.expr->parent_pos = index++;
      }
    }

    J_A(AI,ND) void erase(expr & e) noexcept {
      erase_operand(*e.parent, e.parent_pos);
    }

    void convert_operand_to_progn(expr & e, u32_t i) noexcept {
      auto * victim = e.input(i).expr;
      if (victim->num_inputs == 0U) {
        erase_operand(e, i);
        return;
      }

      fn.counts.remove_expr(victim->type);
      if (victim->num_inputs == 1U) {
        auto child = victim->input(0).expr;
        e.input(i).expr = child;
        child->parent = &e;
        child->parent_pos = victim->parent_pos;
      } else {
        victim->type = expr_type::progn;
        fn.counts.add_expr(expr_type::progn);
      }
    }

    expr * create_for_resize(expr * J_NOT_NULL e, u8_t size, u32_t num_extra_vars = 0U) {
      fn.counts.remove_expr(e->type);
      switch (e->type) {
      case expr_type::vec:
      case expr_type::vec_append:
      case expr_type::progn:
      case expr_type::iadd:
      case expr_type::isub:
      case expr_type::imul:
      case expr_type::idiv:
      case expr_type::lor:
      case expr_type::land:
        return create(e->type, copy_metadata(e->metadata()), exprs::allocate_inputs(size));
      case expr_type::let: {
        auto own_vars = reinterpret_cast<air_var_defs_t*>(e->static_begin());
        expr * result = create(expr_type::let, copy_metadata(e->metadata()), exprs::allocate_inputs(size), exprs::alloc_vars(num_extra_vars + own_vars->size()));
        auto res_vars = reinterpret_cast<air_var_defs_t*>(result->static_begin());
        ::j::memcpy(res_vars->begin(), own_vars->begin(), sizeof(air_var*) * own_vars->size());
        return result;
      }
      default:
        return nullptr;
      }
    }

    expr * create_empty(expr_type e, u8_t capacity) {
      expr * result = create(e, reserve_inputs(capacity));
      J_ASSERT(result && result->num_inputs == 0U && result->max_inputs == capacity);
      return result;
    }

    expr * try_append_input_to(expr * J_NOT_NULL parent, expr * J_NOT_NULL input) noexcept {
      if (input->parent == parent) {
        erase_operand(*parent, input->parent_pos, false);
        J_ASSUME(parent->max_inputs > parent->num_inputs);
      } else if (input->parent) {
        erase(*input);
      }
      if (parent->max_inputs > parent->num_inputs) {
        attach_to(parent, parent->num_inputs++, input);
        return parent;
      }

      exprs::expr * new_parent = create_for_resize(parent, parent->num_inputs + 1U);
      if (new_parent) {
        for (u8_t i = 0; i < parent->num_inputs; ++i) {
          attach_to(new_parent, i, parent->input(i).expr);
        }
        attach_to(new_parent, parent->num_inputs, input);
        attach_to(parent->parent, parent->parent_pos, new_parent);
      }
      return new_parent;
    }

    expr * try_prepend_input_to(expr * J_NOT_NULL parent, expr * J_NOT_NULL input) noexcept {
      if (input->parent) {
        erase(*input);
      }
      if (parent->max_inputs > parent->num_inputs) {
        shift_inputs_right(*parent, 0);
        ++parent->num_inputs;
        attach_to(parent, 0, input);
        return parent;
      }

      exprs::expr * new_parent = create_for_resize(parent, parent->num_inputs + 1U);
      if (new_parent) {
        attach_to(new_parent, 0, input);
        for (u8_t i = 0; i < parent->num_inputs; ++i) {
          attach_to(new_parent, i + 1, parent->input(i).expr);
        }
        if (parent->parent) {
          attach_to(parent->parent, parent->parent_pos, new_parent);
        }
      }
      return new_parent;
    }

    expr * try_append_inputs_to(expr * to, expr * from) noexcept {
      u8_t num_self = from->num_inputs;
      u8_t num_to = to->num_inputs;
      u8_t new_size = num_self + num_to;

      if (new_size <= to->max_inputs) {
        to->num_inputs = new_size;
        for (u8_t i = 0; i < num_self; ++i) {
          attach_to(to, i + num_to, from->input(i).expr);
        }
        from->num_inputs = 0;
        return to;
      }

      exprs::expr * new_e = create_for_resize(to, new_size);
      if (!new_e) {
        return nullptr;
      }
      for (u8_t i = 0; i < num_to; ++i) {
        attach_to(new_e, i, to->input(i).expr);
      }
      for (u8_t i = 0; i < num_self; ++i) {
        attach_to(new_e, i + num_to, from->input(i).expr);
      }
      new_e->visit_flag = to->visit_flag;
      new_e->result = to->result;
      attach_to(to->parent, to->parent_pos, new_e);
      from->num_inputs = 0;
      return new_e;
    }

    expr * try_merge_inputs_to_parent(expr * e) noexcept {
      expr * p = e->parent;
      u8_t num_self = e->num_inputs;
      if (!num_self) {
        erase(*e);
        return p;
      }
      u8_t num_parent = p->num_inputs - 1U;
      u8_t new_size = num_self + num_parent;
      if (new_size <= p->max_inputs) {
        p->num_inputs = new_size;
        auto insert_begin = p->inputs_begin() + e->parent_pos;
        ::j::memmove(insert_begin + num_self,
                     insert_begin + 1,
                     (num_parent - e->parent_pos) * sizeof(input));
        for (u8_t i = 0; i < num_self; ++i) {
          attach_to(p, i + e->parent_pos, e->input(i).expr);
        }
        for (u8_t i = num_self + e->parent_pos; i < new_size; ++i) {
          p->input(i).expr->parent_pos = i;
        }
        fn.counts.remove_expr(e->type);
        return p;
      }
      u32_t extra_vars = 0U;
      if (e->type == expr_type::let) {
        J_ASSERT(p->type == expr_type::let);
        extra_vars = reinterpret_cast<air_var_defs_t*>(e->static_begin())->size();
      }
      exprs::expr * new_e = create_for_resize(p, new_size, extra_vars);
      if (!new_e) {
        return nullptr;
      }
      if (extra_vars) {
        auto p_vars = reinterpret_cast<air_var_defs_t*>(p->static_begin());
        auto res_vars = reinterpret_cast<air_var_defs_t*>(new_e->static_begin());
        auto own_vars = reinterpret_cast<air_var_defs_t*>(e->static_begin());
        ::j::memcpy(res_vars->begin() + p_vars->size(), own_vars->begin(), sizeof(air_var*) * own_vars->size());
      }
      for (u8_t i = 0; i < e->parent_pos; ++i) {
        attach_to(new_e, i, p->input(i).expr);
      }
      for (u8_t i = 0; i < num_self; ++i) {
        attach_to(new_e, i + e->parent_pos, e->input(i).expr);
      }
      for (u8_t i = 0; i < num_parent - e->parent_pos; ++i) {
        attach_to(new_e, i + e->parent_pos + num_self, p->input(i + e->parent_pos + 1).expr);
      }
      new_e->visit_flag = p->visit_flag;
      new_e->result = p->result;
      attach_to(p->parent, p->parent_pos, new_e);
      return new_e;
    }
  };
}
