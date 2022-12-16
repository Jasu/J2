#pragma once

#include "strings/unicode/rope/common.hpp"
#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"
#include "strings/unicode/utf8.hpp"

namespace j::strings::inline unicode::inline rope {
  struct rope_utf8_code_point_iterator final {
    const_rope_utf8_byte_iterator m_it;

    constexpr rope_utf8_code_point_iterator() noexcept = default;

    J_ALWAYS_INLINE explicit rope_utf8_code_point_iterator(const_rope_utf8_byte_iterator it) noexcept
      : m_it(it)
    { }

    J_INLINE_GETTER u32_t operator*() const noexcept {
      J_ASSUME_NOT_NULL(m_it.m_char);
      return utf8_get_code_point(m_it.m_char);
    }

    J_ALWAYS_INLINE rope_utf8_code_point_iterator & operator++() noexcept {
      J_ASSUME_NOT_NULL(m_it.m_char);
      m_it += utf8_code_point_bytes(m_it.m_char);
      return *this;
    }

    J_ALWAYS_INLINE rope_utf8_code_point_iterator & operator--() noexcept {
      --m_it;
      m_it.m_char -= utf8_code_point_bytes_backwards(m_it.m_char) - 1;
      return *this;
    }

    J_ALWAYS_INLINE rope_utf8_code_point_iterator operator++(int) noexcept {
      const auto result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE rope_utf8_code_point_iterator operator--(int) noexcept {
      const auto result{*this};
      return operator--(), result;
    }

    J_INLINE_GETTER bool operator==(const rope_utf8_code_point_iterator & rhs) const noexcept
    { return m_it == rhs.m_it; }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return m_it.operator bool(); }

    J_INLINE_GETTER bool operator!() const noexcept
    { return m_it.operator!(); }

    J_ALWAYS_INLINE void reset() noexcept
    { m_it.reset(); }

    J_INLINE_GETTER u64_t position() const noexcept
    { return m_it.position(); }

    J_INLINE_GETTER operator const const_rope_utf8_byte_iterator & () const noexcept
    { return m_it; }
  };
}
