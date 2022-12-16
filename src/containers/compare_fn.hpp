#pragma once

#include "hzd/utility.hpp"

namespace j::inline containers {
  template<typename K>
  using compare_fn_t = i32_t (*)(const K &, const K &) noexcept;

  template<typename K>
  J_INLINE_GETTER_NO_DEBUG i32_t compare(const K & lhs, const K & rhs) noexcept {
    if (lhs < rhs) return -1;
    return lhs == rhs ? 0 : 1;
  }

  template<typename K>
  struct comparer final {
    J_INLINE_GETTER i32_t operator()(const K & lhs, const K & rhs) const noexcept {
      if (lhs < rhs) return -1;
      return lhs == rhs ? 0 : 1;
    }
  };

  template<>
  struct comparer<u8_t> final {
    J_INLINE_GETTER i32_t operator()(u8_t lhs, const u8_t rhs) const noexcept {
      return (i32_t)lhs - rhs;
    }
  };

  template<>
  struct comparer<u16_t> final {
    J_INLINE_GETTER i32_t operator()(u16_t lhs, const u16_t rhs) const noexcept {
      return (i32_t)lhs - rhs;
    }
  };

  template<>
  struct comparer<i8_t> final {
    J_INLINE_GETTER i32_t operator()(i8_t lhs, const i8_t rhs) const noexcept {
      return (i32_t)lhs - rhs;
    }
  };

  template<>
  struct comparer<i16_t> final {
    J_INLINE_GETTER i32_t operator()(i16_t lhs, const i16_t rhs) const noexcept {
      return (i32_t)lhs - rhs;
    }
  };

  template<>
  struct comparer<u32_t> final {
    J_INLINE_GETTER i64_t operator()(u32_t lhs, const u32_t rhs) const noexcept {
      return (i64_t)lhs - rhs;
    }
  };

  template<>
  struct comparer<i32_t> final {
    J_INLINE_GETTER i64_t operator()(i32_t lhs, const i32_t rhs) const noexcept {
      return (i64_t)lhs - rhs;
    }
  };
}
