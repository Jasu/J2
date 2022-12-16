#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::lisp::inline common {
  enum class truthiness : u8_t {
    none          = 0b00,
    always_falsy  = 0b01,
    always_truthy = 0b10,
    unknown       = 0b11,
  };

  enum class truthiness_condition : u8_t {
    none           = 0b1100,
    any            = 0b0011,
    may_be_falsy   = 0b0001,
    may_be_truthy  = 0b0010,
    must_be_falsy  = 0b1001,
    must_be_truthy = 0b0110,
  };

  [[nodiscard]] inline constexpr truthiness invert(truthiness t) noexcept {
    switch(t) {
    case truthiness::none:  J_FAIL("Empty truthiness");
    case truthiness::unknown:  return t;
    case truthiness::always_falsy:  return truthiness::always_truthy;
    case truthiness::always_truthy:  return truthiness::always_falsy;
    }
  }

  J_INLINE_GETTER constexpr truthiness_condition flip_condition(truthiness_condition t) noexcept {
    return (truthiness_condition)((((u8_t)t & 0b0101) << 1U) | (((u8_t)t & 0b1010) >> 1U));
  }

  J_INLINE_GETTER constexpr truthiness operator|(truthiness lhs, truthiness rhs) noexcept {
    return (truthiness)((u8_t)lhs | (u8_t)rhs);
  }

  J_INLINE_GETTER constexpr truthiness operator&(truthiness lhs, truthiness rhs) noexcept {
    return (truthiness)((u8_t)lhs & (u8_t)rhs);
  }

  J_INLINE_GETTER constexpr truthiness operator-(truthiness lhs, truthiness rhs) noexcept {
    return rhs == truthiness::unknown ? lhs : (truthiness)((u8_t)lhs & ~(u8_t)rhs);
  }

  J_INLINE_GETTER constexpr bool operator&(truthiness lhs, truthiness_condition rhs) noexcept {
    return ((u8_t)lhs & (u8_t)rhs) && !((u8_t)lhs & ((u8_t)rhs >> 2U));
  }

  J_INLINE_GETTER constexpr bool operator&(truthiness_condition lhs, truthiness rhs) noexcept {
    return operator&(rhs, lhs);
  }

  J_INLINE_GETTER constexpr bool operator&(bool lhs, truthiness_condition rhs) noexcept {
    return operator&(lhs ? truthiness::always_truthy : truthiness::always_falsy, rhs);
  }

  J_INLINE_GETTER constexpr bool operator&(truthiness_condition lhs, bool rhs) noexcept {
    return operator&(rhs, lhs);
  }
}
