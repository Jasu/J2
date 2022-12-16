#pragma once

#include "strings/unicode/rope/common.hpp"

J_DECLARE_EXTERN_ROPE_TREE(j::strings::rope_utf8_description);

namespace j::strings::inline unicode::inline rope {
  template<bool IsConst>
  struct basic_rope_utf8_byte_iterator final {
    using chunk_iterator_t = conditional_t<
      IsConst,
      const_rope_utf8_chunk_iterator,
      rope_utf8_chunk_iterator>;

    using char_t = const_if_t<IsConst, char>;

    char_t * m_char = nullptr;
    char_t * m_end = nullptr;
    chunk_iterator_t m_it;

    J_ALWAYS_INLINE void reset() noexcept {
      m_char = m_end = nullptr;
      m_it = {};
    }

    [[nodiscard]] bool is_at_begin() const noexcept
    { return m_it.is_at_begin() && m_char == m_it->text.begin(); }

    J_INLINE_GETTER bool is_before_begin() const noexcept
    { return m_char == m_it->text.begin() - 1; }

    J_INLINE_GETTER bool is_at_end() const noexcept
    { return m_char == m_end; }

    [[nodiscard]] u64_t position() const noexcept;

    J_INLINE_GETTER bool is_at_chunk_begin() const noexcept {
      return m_char != m_end && m_it->text.begin() == m_char;
    }

    J_INLINE_GETTER bool is_at_chunk_end() const noexcept {
      const char * begin = m_it->text.begin();
      return (m_char == m_end && m_char != begin) || (m_char == begin && m_it.has_previous());
    }

    J_INLINE_GETTER char_t & operator*() const noexcept {
      J_ASSERT_NOT_NULL(m_char);
      return *m_char;
    }

    void rewind(u64_t num) noexcept;

    void forward(u64_t num) noexcept;

    inline basic_rope_utf8_byte_iterator & operator+=(i64_t num) noexcept {
      J_ASSUME(m_char);
      if (J_LIKELY(num)) {
        num > 0 ? forward(num) : rewind(-num);
      }
      return *this;
    }


    inline basic_rope_utf8_byte_iterator & operator-=(i64_t num) noexcept {
      J_ASSERT_NOT_NULL(m_char);
      if (J_LIKELY(num)) {
        num > 0 ? rewind(num) : forward(-num);
      }
      return *this;
    }

    inline basic_rope_utf8_byte_iterator operator+(i64_t num) const noexcept {
      auto result{*this};
      result += num;
      return result;
    }

    inline basic_rope_utf8_byte_iterator operator-(i64_t num) const noexcept {
      auto result{*this};
      result -= num;
      return result;
    }

    basic_rope_utf8_byte_iterator & operator++() noexcept {
      J_ASSUME(m_char && m_char < m_end);
      if (++m_char == m_end) {
        if (J_LIKELY(m_it.has_next())) {
          const auto text = (++m_it)->text;
          m_char = text.begin();
          m_end = text.end();
        }
      }
      return *this;
    }

    basic_rope_utf8_byte_iterator & operator--() noexcept {
      J_ASSUME(m_char && m_char <= m_end);
      if (m_char-- == m_it->text.begin() && m_it.has_previous()) {
        const auto text = (--m_it)->text;
        m_end = m_char = text.end();
        --m_char;
      }
      return *this;
    }

    J_ALWAYS_INLINE basic_rope_utf8_byte_iterator operator++(int) noexcept {
      const basic_rope_utf8_byte_iterator result{*this};
      return operator++(), result;
    }

    J_ALWAYS_INLINE basic_rope_utf8_byte_iterator operator--(int) noexcept {
      const basic_rope_utf8_byte_iterator result{*this};
      return operator--(), result;
    }

    J_INLINE_GETTER basic_rope_utf8_byte_iterator operator+(i64_t i) noexcept {
      basic_rope_utf8_byte_iterator result{*this};
      result += i;
      return result;
    }

    J_INLINE_GETTER basic_rope_utf8_byte_iterator operator-(i64_t i) noexcept {
      basic_rope_utf8_byte_iterator result{*this};
      result -= i;
      return result;
    }

    J_INLINE_GETTER bool operator==(const basic_rope_utf8_byte_iterator & rhs) const noexcept
    { return m_char == rhs.m_char; }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return m_char != m_end; }

    J_INLINE_GETTER bool operator!() const noexcept
    { return m_char == m_end; }

    J_INLINE_GETTER operator const basic_rope_utf8_byte_iterator<true> &() const noexcept
    { return *reinterpret_cast<const basic_rope_utf8_byte_iterator<true> *>(this); }
  };

  using rope_utf8_byte_iterator = basic_rope_utf8_byte_iterator<false>;
  using const_rope_utf8_byte_iterator = basic_rope_utf8_byte_iterator<true>;

  template<bool IsConst>
  i64_t operator-(const basic_rope_utf8_byte_iterator<IsConst> & lhs,
                  const basic_rope_utf8_byte_iterator<IsConst> & rhs) noexcept;

  extern template struct basic_rope_utf8_byte_iterator<false>;
  extern template struct basic_rope_utf8_byte_iterator<true>;
  extern template i64_t operator-<true>(
    const basic_rope_utf8_byte_iterator<true> & lhs,
    const basic_rope_utf8_byte_iterator<true> & rhs) noexcept;
  extern template i64_t operator-<false>(
    const basic_rope_utf8_byte_iterator<false> & lhs,
    const basic_rope_utf8_byte_iterator<false> & rhs) noexcept;
}
