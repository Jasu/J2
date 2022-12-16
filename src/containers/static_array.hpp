#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::inline containers {
  template<typename T, u32_t Size>
  class fixed_array final {
  public:
    J_BOILERPLATE(fixed_array, CTOR_CE)

    template<typename... Ts>
    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr fixed_array(Ts && ... vals) noexcept
      : m_data{static_cast<Ts &&>(vals)...}
    { }

    J_INLINE_GETTER constexpr T & operator[](u32_t i) noexcept {
      J_ASSERT(i < Size);
      return m_data[i];
    }

    J_INLINE_GETTER constexpr const T & operator[](u32_t i) const noexcept {
      J_ASSERT(i < Size);
      return m_data[i];
    }

    J_INLINE_GETTER_NONNULL constexpr T * begin() noexcept { return m_data; }
    J_INLINE_GETTER_NONNULL constexpr T * end() noexcept { return m_data + Size; }
    J_INLINE_GETTER_NONNULL constexpr const T * begin() const noexcept { return m_data; }
    J_INLINE_GETTER_NONNULL constexpr const T * end() const noexcept { return m_data + Size; }

    J_INLINE_GETTER constexpr u32_t size() const noexcept { return Size; }

    J_INLINE_GETTER constexpr bool empty() const noexcept { return !Size; }
    J_INLINE_GETTER constexpr bool operator!() const noexcept { return !Size; }
    J_INLINE_GETTER constexpr explicit operator bool() const noexcept { return Size; }

  private:
    T m_data[Size];
  };

  template<typename T, typename... Ts>
  fixed_array(T, Ts...) -> fixed_array<T, sizeof...(Ts) + 1U>;

  template<typename T, u32_t Capacity>
  class static_array final {
  public:
    J_BOILERPLATE(static_array, CTOR_CE)

    template<typename... Ts>
    J_ALWAYS_INLINE_NO_DEBUG explicit constexpr static_array(Ts && ... vals) noexcept
      : m_data{static_cast<Ts &&>(vals)...},
        m_size(sizeof...(Ts))
    { }

    J_INLINE_GETTER constexpr T & operator[](u32_t i) noexcept {
      return m_data[i];
    }

    J_INLINE_GETTER constexpr const T & operator[](u32_t i) const noexcept {
      return m_data[i];
    }

    J_INLINE_GETTER constexpr T * begin() noexcept { return m_data; }
    J_INLINE_GETTER constexpr T * end() noexcept { return m_data + m_size; }
    J_INLINE_GETTER constexpr const T * begin() const noexcept { return m_data; }
    J_INLINE_GETTER constexpr const T * end() const noexcept { return m_data + m_size; }

    J_INLINE_GETTER constexpr u32_t size() const noexcept { return m_size; }

    J_INLINE_GETTER constexpr bool empty() const noexcept { return !m_size; }
    J_INLINE_GETTER constexpr bool operator!() const noexcept { return !m_size; }
    J_INLINE_GETTER constexpr explicit operator bool() const noexcept { return m_size; }
  private:
    T m_data[Capacity];
    u32_t m_size = 0;
  };
}
