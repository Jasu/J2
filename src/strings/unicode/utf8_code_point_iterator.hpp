#pragma once

#include "strings/unicode/utf8.hpp"

namespace j::strings::inline unicode {
  struct utf8_code_point_iterator final {
    const char * m_it = nullptr;
    const char * m_end = nullptr;

    inline utf8_code_point_iterator & operator++() noexcept {
      m_it += utf8_code_point_bytes(m_it, m_end);
      return *this;
    }

    J_ALWAYS_INLINE utf8_code_point_iterator operator++(int) noexcept {
      utf8_code_point_iterator result{*this};
      return operator++(), result;
    }

    inline utf8_code_point_iterator & operator--() noexcept {
      m_it -= utf8_code_point_bytes_backwards(m_it - 1);
      return *this;
    }

    J_ALWAYS_INLINE utf8_code_point_iterator operator--(int) noexcept {
      utf8_code_point_iterator result{*this};
      return operator--(), result;
    }

    J_ALWAYS_INLINE void reset() noexcept {
      m_it = m_end = nullptr;
    }

    J_INLINE_GETTER u32_t operator*() const noexcept {
      return utf8_get_code_point(m_it, m_end);
    }

    [[nodiscard]] inline bool operator==(const utf8_code_point_iterator & rhs) const noexcept {
      J_ASSERT(m_end == rhs.m_end);
      return m_it == rhs.m_it;
    }

    [[nodiscard]] inline bool operator<(const utf8_code_point_iterator & rhs) const noexcept {
      J_ASSUME(m_end == rhs.m_end);
      return m_it < rhs.m_it;
    }

    J_INLINE_GETTER bool operator<=(const utf8_code_point_iterator & rhs) const noexcept
    { return !rhs.operator<(*this); }

    J_INLINE_GETTER bool operator>(const utf8_code_point_iterator & rhs) const noexcept
    { return rhs.operator<(*this); }

    J_INLINE_GETTER bool operator>=(const utf8_code_point_iterator & rhs) const noexcept
    { return !operator<(rhs); }

    J_INLINE_GETTER bool operator==(const char * const rhs) const noexcept
    { return m_it == rhs; }

    J_INLINE_GETTER bool operator<(const char * const rhs) const noexcept
    { return m_it < rhs; }

    J_INLINE_GETTER bool operator<=(const char * const rhs) const noexcept
    { return m_it <= rhs; }

    J_INLINE_GETTER bool operator>(const char * const rhs) const noexcept
    { return m_it > rhs; }

    J_INLINE_GETTER bool operator>=(const char * const rhs) const noexcept
    { return m_it >= rhs; }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !m_it;
    }
    J_INLINE_GETTER explicit operator bool() const noexcept {
      return m_it;
    }

    J_INLINE_GETTER const char * ptr() const noexcept
    { return m_it; }
  };
}
