#pragma once

#include "lisp/cir/locs/loc.hpp"

namespace j::lisp::cir::inline ops {
  struct op;
  struct input;

  enum class use_type : u8_t {
    none,

    op_result,
    op_input,
    mem_input_base,
    mem_input_index,
    move,
    callee_saved,
  };

  struct use final {
    use_type type = use_type::none;
    u8_t input_index = 0U;
    loc loc_in{};
    loc loc_out{};
    use * next_use = nullptr;
    op * def = nullptr;

    J_ALWAYS_INLINE void set_loc(loc in, loc out) noexcept {
      loc_in = in;
      loc_out = out;
    }

    J_ALWAYS_INLINE void set_loc(loc l) noexcept {
      loc_in = l;
      loc_out = l;
    }

    void reset() noexcept {
      input_index = 0U;
      type = use_type::none;
      loc_in = {};
      loc_out = {};
      next_use = nullptr;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return type != use_type::none;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return type == use_type::none;
    }

    [[nodiscard]] J_RETURNS_NONNULL op * op() const noexcept;
    [[nodiscard]] J_RETURNS_NONNULL input * input() const noexcept;
    void attach(use * J_NOT_NULL target, u8_t index, use_type type = use_type::op_input) noexcept;
    void attach(u8_t index, use_type type = use_type::op_input) noexcept;
    void detach() noexcept;
    [[nodiscard]] use * previous_use() const noexcept;

    void set_next_use(struct use * J_NOT_NULL u) noexcept;
  };
}
