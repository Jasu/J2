#pragma once

#include "hzd/concepts.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::algo {
  template<typename T>
  void destroy(T * J_NOT_NULL begin, const T * const J_NOT_NULL end) noexcept {
    J_ASSUME(begin <= end);
    for (; begin != end; ++begin) {
      begin->~T();
    }
  }

  template<TriviallyDestructible T>
  void destroy(T * J_NOT_NULL, const T * const J_NOT_NULL) noexcept {}

  template<typename T>
  void destroy(T * J_NOT_NULL begin, u32_t sz) noexcept {
    T * const end = begin + sz;
    for (; begin != end; ++begin) {
      begin->~T();
    }
  }

  template<TriviallyDestructible T>
  void destroy(T * J_NOT_NULL, u32_t) noexcept { }
}
