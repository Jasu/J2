#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::sources {
  struct source;

  struct source_location final {
    J_BOILERPLATE(source_location, CTOR_ND, EQ)

    u64_t is_valid:1 = 0U;
    u64_t source_index:15 = 0U;
    u64_t line:24 = 0U;
    u64_t column:24 = 0U;

    J_A(AI,ND) inline source_location(u32_t source_index, u32_t line, u32_t column) noexcept
      : is_valid(1U),
        source_index(source_index),
        line(line),
        column(column)
    { }

    source_location(const source & src, u32_t line, u32_t column) noexcept;

    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return is_valid; }

    J_A(AI) inline bool operator!() const noexcept { return !is_valid; }
    inline void clear() noexcept {
      is_valid = false;
      source_index = 0U;
      line = 0U;
      column = 0U;
    }
  };
}
