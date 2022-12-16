#pragma once

#include "hzd/type_traits.hpp"

namespace j::inline containers {
  template<typename T, u32_t N>
  struct inline_wrapper final {
    J_INLINE_GETTER_NONNULL T * data() const noexcept
    { return (T*)&m_data[0]; }

    J_INLINE_GETTER_NONNULL T * ptr(u32_t i) const noexcept {
      return data() + i;
    }
    J_INLINE_GETTER T & operator[](u32_t i) const noexcept
    { return *ptr(i); }
  private:
    alignas(T) u8_t m_data[N * sizeof(T)];
  };

  template<typename T>
  struct inline_wrapper<T, 0U> final { };
}
