#pragma once

#include "lisp/common/truthiness.hpp"

namespace j::lisp::value_info {
  struct bool_info final {
    truthiness truthiness = truthiness::unknown;

    J_ALWAYS_INLINE bool_info & operator|=(const bool_info & rhs) noexcept {
      truthiness = truthiness | rhs.truthiness;
      return *this;
    }

    J_ALWAYS_INLINE bool_info & operator&=(const bool_info & rhs) noexcept {
      truthiness = truthiness & rhs.truthiness;
      return *this;
    }

    J_ALWAYS_INLINE bool_info & operator-=(const bool_info & rhs) noexcept {
      truthiness = truthiness - rhs.truthiness;
      return *this;
    }

    J_INLINE_GETTER bool_info operator|(const bool_info & rhs) const noexcept {
      return { truthiness | rhs.truthiness };
    }

    J_ALWAYS_INLINE bool_info operator&(const bool_info & rhs) const noexcept {
      return { truthiness & rhs.truthiness };
    }

    J_INLINE_GETTER bool_info operator-(const bool_info & rhs) const noexcept {
      return { truthiness - rhs.truthiness };
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return truthiness == truthiness::none;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return truthiness != truthiness::none;
    }
  };

  struct i64_info final {
    i64_t min = I64_MIN;
    i64_t max = I64_MAX;
    u64_t possible_bits = U64_MAX;
    u64_t known_set_bits = 0U;

    J_ALWAYS_INLINE i64_info & operator|=(const i64_info & rhs) noexcept {
      min = j::min(min, rhs.min);
      max = j::max(max, rhs.max);
      possible_bits |= rhs.possible_bits;
      known_set_bits |= rhs.known_set_bits;
      return *this;
    }

    J_INLINE_GETTER i64_info operator|(const i64_info & rhs) const noexcept {
      return {
        j::min(min, rhs.min),
        j::max(max, rhs.max),
        possible_bits | rhs.possible_bits,
        known_set_bits | rhs.known_set_bits,
      };
    }

    J_ALWAYS_INLINE i64_info & operator&=(const i64_info & rhs) noexcept {
      min = j::max(min, rhs.min);
      max = j::min(max, rhs.max);
      possible_bits &= rhs.possible_bits;
      known_set_bits |= rhs.known_set_bits;
      normalize();
      return *this;
    }

    J_INLINE_GETTER i64_info operator&(const i64_info & rhs) const noexcept {
      i64_info result{
        j::max(min, rhs.min),
        j::min(max, rhs.max),
        possible_bits & rhs.possible_bits,
        known_set_bits | rhs.known_set_bits,
      };
      result.normalize();
      return result;
    }

    J_ALWAYS_INLINE void normalize() noexcept {
      if (min > max || (known_set_bits & ~possible_bits)) {
        clear();
      }
    }

    void clear() noexcept {
      min = I64_MAX;
      max = I64_MIN;
      possible_bits = 0U;
      known_set_bits = 0U;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return min <= max;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return min > max;
    }
  };
}
