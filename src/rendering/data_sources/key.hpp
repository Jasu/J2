#pragma once

#include "exceptions/assert_lite.hpp"

namespace j::rendering::data_sources {
  template<typename Tag>
  class key {
  public:
    J_ALWAYS_INLINE constexpr key() noexcept = default;

    J_ALWAYS_INLINE explicit key(uptr_t uptr) noexcept
      : m_uptr(uptr)
    { }

    J_ALWAYS_INLINE key(u32_t index, const void * ptr) noexcept
      : m_uptr((reinterpret_cast<uptr_t>(ptr) << 4) | index)
    { J_ASSUME(index < 16); }

    J_ALWAYS_INLINE constexpr key(u32_t index, uptr_t ptr) noexcept
      : m_uptr((ptr << 4) | index)
    { J_ASSUME(index < 16); }

    J_INLINE_GETTER constexpr u8_t handler_index() const noexcept
    { return m_uptr & 15U; }

    J_INLINE_GETTER void * ptr() const noexcept
    { return reinterpret_cast<void*>(m_uptr >> 4); }

    J_INLINE_GETTER constexpr uptr_t uptr() const noexcept
    { return m_uptr >> 4; }

    J_INLINE_GETTER constexpr explicit operator uptr_t() const noexcept
    { return m_uptr; }

    J_INLINE_GETTER constexpr explicit operator bool() const noexcept
    { return m_uptr; }

    J_INLINE_GETTER constexpr bool empty() const noexcept
    { return !m_uptr; }

    J_INLINE_GETTER bool operator!() const noexcept
    { return !m_uptr; }

    J_INLINE_GETTER constexpr bool operator==(const key & rhs) const noexcept = default;

    J_INLINE_GETTER constexpr bool operator<(const key & rhs) const noexcept
    { return m_uptr < rhs.m_uptr; }

    J_INLINE_GETTER constexpr bool operator<=(const key & rhs) const noexcept
    { return m_uptr <= rhs.m_uptr; }

    J_INLINE_GETTER constexpr bool operator>(const key & rhs) const noexcept
    { return m_uptr > rhs.m_uptr; }

    J_INLINE_GETTER constexpr bool operator>=(const key & rhs) const noexcept
    { return m_uptr >= rhs.m_uptr; }

  private:
    uptr_t m_uptr = 0ULL;
  };
}
