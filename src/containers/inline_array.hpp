#pragma once

#include "containers/inline_array_fwd.hpp"

namespace j::inline containers {
  template<typename T>
  inline_array<T>::inline_array(u32_t size) noexcept(is_nothrow_constructible_v<T>)
    : m_size(size)
  {
    if (size) {
      if constexpr (is_zero_initializable_v<T>) {
        j::memzero(this + 1, m_size * sizeof(T));
      } else {
        T *cur = begin(), * const e = end();
        while (cur != e) {
          ::new (cur++) T;
        }
      }
    }
  }

  template<typename T>
  T & inline_array<T>::at(u32_t i) {
    if (J_UNLIKELY(i >= m_size)) {
      exceptions::throw_out_of_range(0, i);
    }
    return begin()[i];
  }

  template<typename T>
  void inline_array<T>::clear() noexcept {
    if (m_size) {
      if constexpr (!is_trivially_destructible_v<T>) {
        for (T *e = begin() - 1U, *cur = e + m_size; cur != e; --cur) {
          cur->~T();
        }
      }
      m_size = 0U;
    }
  }
}

#define J_DEFINE_EXTERN_INLINE_ARRAY(T) template class j::containers::inline_array<T>
