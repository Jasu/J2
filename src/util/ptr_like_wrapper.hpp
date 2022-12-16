#pragma once

#include "hzd/type_traits.hpp"

namespace j::util {
  template<typename T>
  struct ptr_like_wrapper final {
    J_INLINE_GETTER_NONNULL T * operator->() noexcept { return &m_value; }
    J_INLINE_GETTER_NONNULL const T * operator->() const noexcept { return &m_value; }
    J_INLINE_GETTER T & operator*() noexcept { return m_value; }
    J_INLINE_GETTER const T & operator*() const noexcept { return m_value; }

    T m_value;
  };
}
