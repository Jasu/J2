#pragma once

#include "lisp/air/exprs/expr_criteria.hpp"

namespace j::lisp::air::exprs {
  enum class expr_path_part_type : u8_t {
    none = 0U,

    parent,
    input,
    first_sibling,
    last_sibling,
    prev_sibling,
    next_sibling,
  };

  struct expr_path_part final {
    expr_path_part_type type = expr_path_part_type::none;
    i8_t child_index = 0;
    expr_criteria criteria{};
  };

  struct expr_path final {
    expr_path_part parts[4] = {};
    expr_criteria self_criteria{};
    u8_t num_parts = 0U;

    template<typename... Ts>
    inline expr_path with(expr_path_part_type type, i8_t idx, Ts && ... criteria) const noexcept {
      expr_path result{*this};
      result.parts[num_parts] = { type, idx };
      ++result.num_parts;
      ((result.parts[num_parts].criteria.set(static_cast<Ts &&>(criteria))), ...);
      return result;
    }

    template<typename... Ts>
    inline expr_path parent(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::parent, 0U, static_cast<Ts &&>(criteria)...);
    }
    template<typename... Ts>
    inline expr_path prev_sibling(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::prev_sibling, 0U, static_cast<Ts &&>(criteria)...);
    }
    template<typename... Ts>
    inline expr_path next_sibling(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::prev_sibling, 0U, static_cast<Ts &&>(criteria)...);
    }
    template<typename... Ts>
    inline expr_path input(i8_t idx, Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::input, idx, static_cast<Ts &&>(criteria)...);
    }

    template<typename... Ts>
    inline expr_path first_sibling(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::first_sibling, 0, static_cast<Ts &&>(criteria)...);
    }

    template<typename... Ts>
    inline expr_path last_sibling(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::last_sibling, 0, static_cast<Ts &&>(criteria)...);
    }

    template<typename... Ts>
    inline expr_path first_input(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::input, 0, static_cast<Ts &&>(criteria)...);
    }

    template<typename... Ts>
    inline expr_path last_input(Ts && ... criteria) const noexcept {
      return with(expr_path_part_type::input, -1, static_cast<Ts &&>(criteria)...);
    }

    template<typename Expr>
    Expr * traverse(Expr * J_NOT_NULL e) const noexcept {
      if (!self_criteria.check(e)) {
        return nullptr;
      }
      for (u8_t i = 0; i < num_parts; ++i) {
        auto & part = parts[i];
        switch (part.type) {
        case expr_path_part_type::none:
          J_FAIL("Empty path part");
        case expr_path_part_type::parent:
          e = e->parent;
          break;
        case expr_path_part_type::input: {
          u8_t idx = part.child_index < 0 ? e->num_inputs + part.child_index : part.child_index;
          e = e->num_inputs <= idx ? nullptr : e->input(idx).expr;
          break;
        }
        case expr_path_part_type::first_sibling:
          e = e->parent_pos == 0 ? nullptr : e->parent->input(0).expr;
          break;
        case expr_path_part_type::last_sibling:
          e = e->parent_pos == e->parent->num_inputs - 1 ? nullptr : e->parent->input(e->parent->num_inputs - 1).expr;
          break;
        case expr_path_part_type::prev_sibling:
          e = e->parent_pos == 0 ? nullptr : e->parent->input(e->parent_pos - 1).expr;
          break;
        case expr_path_part_type::next_sibling:
          e = e->parent_pos == e->parent->num_inputs - 1 ? nullptr : e->parent->input(e->parent_pos + 1).expr;
          break;
  }

        if (!e || !part.criteria.check(e)) {
          return nullptr;
        }
      }
      return e;
    }
  };

  inline expr_path self(expr_type t0,
                        expr_type t1 = expr_type::none,
                        expr_type t2 = expr_type::none,
                        expr_type t3 = expr_type::none) noexcept {
    expr_path result;
    result.self_criteria.set(expr_criterion_expr_type{ t0, t1, t2, t3 });
    return result;
  }

  template<typename... Ts>
  inline expr_path self(Ts... criteria) noexcept {
    expr_path result;
    ((result.self_criteria.set(criteria)), ...);
    return result;
  }

  template<typename... Ts>
  inline expr_path parent(Ts && ... criteria) noexcept {
    return expr_path{}.parent(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path next_sibling(Ts && ... criteria) noexcept {
    return expr_path{}.next_sibling(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path prev_sibling(Ts && ... criteria) noexcept {
    return expr_path{}.prev_sibling(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path first_sibling(Ts && ... criteria) noexcept {
    return expr_path{}.first_sibling(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path last_sibling(Ts && ... criteria) noexcept {
    return expr_path{}.last_sibling(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path first_input(Ts && ... criteria) noexcept {
    return expr_path{}.first_input(static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path second_input(Ts && ... criteria) noexcept {
    return expr_path{}.input(1, static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path third_input(Ts && ... criteria) noexcept {
    return expr_path{}.input(2, static_cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path last_input(Ts && ... criteria) noexcept {
    return expr_path{}.last_input(static_Cast<Ts &&>(criteria)...);
  }

  template<typename... Ts>
  inline expr_path nth_input(i8_t i, Ts && ... criteria) noexcept {
    return expr_path{}.input(i, static_cast<Ts &&>(criteria)...);
  }
}
