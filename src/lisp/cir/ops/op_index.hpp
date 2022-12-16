#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::cir::inline ops {
  struct op_index final {
    i16_t index = -1;

    J_BOILERPLATE(op_index, CTOR_NE_ND)

    J_A(AI,ND) inline explicit op_index(i16_t index) noexcept
      : index(index)
    { }

    J_A(AI,NODISC) inline u16_t instruction_index() const noexcept {
      return (u16_t)index >> 1;
    }

    J_A(AI,NODISC) inline op_index as_pre() const noexcept {
      return op_index(index & ~1);
    }

    J_A(AI,NODISC) inline op_index as_post() const noexcept {
      return op_index(index | 1);
    }

    J_A(AI,NODISC) inline op_index next() const noexcept {
      return op_index((index & ~1) + 2);
    }

    J_A(AI,NODISC) inline op_index next_half() const noexcept {
      return op_index(index + 1);
    }

    J_A(AI,NODISC) inline op_index prev_half() const noexcept {
      return op_index(index - 1);
    }

    J_A(AI,NODISC) inline op_index prev() const noexcept {
      return op_index((index & ~1) - 2);
    }

    J_A(AI) inline void to_next() noexcept {
      index = (index & ~1) + 2;
    }

    J_A(AI) inline void to_prev() noexcept {
      index = (index & ~1) - 2;
    }

    J_A(AI,NODISC) inline bool is_pre() const noexcept {
      return !(index & 1);
    }

    J_A(AI,NODISC) inline bool is_post() const noexcept {
      return index & 1;
    }

    J_A(AI,NODISC) inline bool empty() const noexcept {
      return index < 0;
    }

    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return index >= 0;
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return index < 0;
    }

    J_A(AI,NODISC) inline bool operator==(const op_index &) const noexcept = default;

    J_A(AI,NODISC) inline bool operator<(const op_index & rhs) const noexcept {
      return index < rhs.index;
    }

    J_A(AI,NODISC) inline bool operator<=(const op_index & rhs) const noexcept {
      return index <= rhs.index;
    }

    J_A(AI,NODISC) inline bool operator>(const op_index & rhs) const noexcept {
      return index > rhs.index;
    }

    J_A(AI,NODISC) inline bool operator>=(const op_index & rhs) const noexcept {
      return index >= rhs.index;
    }
  };
}
