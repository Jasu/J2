#pragma once

#include "containers/trivial_array_fwd.hpp"

namespace j::inline containers {
  template<typename T>
  trivial_array<T>::trivial_array(u32_t size)
    : trivial_array(uninitialized, size)
  {
    m_size = size;
    if constexpr (is_zero_initializable_v<T> || is_trivially_default_constructible_v<T>) {
      if (size) {
        j::memzero(m_data, m_size * sizeof(T));
      }
    } else {
      for (u32_t i = 0; i < size; ++i) {
        ::new (m_data + i) T;
      }
    }
  }

  template<typename T>
  T & trivial_array<T>::initialize_element(T && element) noexcept {
    return *::new (m_data + m_size++) T(static_cast<T &&>(element));
  }

  template<typename T>
  trivial_array<T> & trivial_array<T>::operator=(trivial_array && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      m_data = rhs.m_data, m_size = rhs.m_size;
      rhs.m_data = nullptr, rhs.m_size = 0;
    }
    return *this;
  }

  template<typename T>
  void trivial_array<T>::clear() noexcept {
    if (m_data) {
      if constexpr (!is_trivially_destructible_v<T>) {
        while (m_size) {
          m_data[--m_size].~T();
        }
      } else {
        m_size = 0U;
      }
      ::j::free(m_data);
      m_data = nullptr;
    }
  }
}

#define J_DEFINE_EXTERN_TRIVIAL_ARRAY(T) template class j::trivial_array<T>
#define J_DEFINE_EXTERN_TRIVIAL_ARRAY_COPYABLE(T) template class j::trivial_array<T>; template class j::trivial_array_copyable<T>
