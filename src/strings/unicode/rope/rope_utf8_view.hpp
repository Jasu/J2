#pragma once

#include "strings/unicode/rope/rope_utf8_code_point_iterator.hpp"
#include "strings/unicode/utf8_string_view.hpp"
#include "strings/unicode/utf8_code_point_iterator.hpp"
#include "strings/string.hpp"
#include "containers/trees/rope_tree_algo.hpp"

namespace j::strings::inline unicode::inline rope {
  namespace unicode_rope_detail {
    string interval_to_string(const_rope_utf8_byte_iterator begin,
                              const_rope_utf8_byte_iterator end);
  }

  template<bool IsConst>
  struct rope_utf8_view_code_points {
    utf8_code_point_iterator begin() const noexcept;
    utf8_code_point_iterator end() const noexcept;
  };

  template<bool IsConst>
  class basic_rope_utf8_view final : rope_utf8_view_code_points<IsConst> {
  public:
    using iterator = basic_rope_utf8_byte_iterator<IsConst>;
    constexpr basic_rope_utf8_view() noexcept = default;

    J_ALWAYS_INLINE basic_rope_utf8_view(iterator begin, iterator end) noexcept
      : m_begin(begin), m_end(end)
    {
      J_ASSERT(begin.position() <= end.position());
    }

    J_INLINE_GETTER iterator begin() const noexcept
    { return m_begin; }

    J_INLINE_GETTER iterator end() const noexcept
    { return m_end; }

    J_INLINE_GETTER u64_t size() const noexcept {
      return containers::trees::rope_value_distance(*m_end.m_it, *m_begin.m_it)
        + (m_end.m_char - m_end.m_it->text.begin())
        - (m_begin.m_char - m_begin.m_it->text.begin());
    }

    J_INLINE_GETTER bool empty() const noexcept
    { return m_begin == m_end; }

    J_INLINE_GETTER explicit operator bool() const noexcept
    { return m_begin != m_end; }

    J_INLINE_GETTER bool operator!() const noexcept
    { return m_begin == m_end; }

    J_ALWAYS_INLINE void remove_prefix(u64_t len) noexcept
    { m_begin += len; }

    J_ALWAYS_INLINE void remove_suffix(u64_t len) noexcept
    { m_end -= len; }

    J_INLINE_GETTER explicit operator string() const
    { return unicode_rope_detail::interval_to_string(m_begin, m_end); }

    [[nodiscard]] J_INLINE_GETTER basic_rope_utf8_view take_prefix(u64_t len) noexcept {
      const iterator old_begin = m_begin;
      remove_prefix(len);
      return {old_begin, m_begin};
    }

    [[nodiscard]] J_INLINE_GETTER basic_rope_utf8_view take_suffix(u64_t len) noexcept {
      const iterator old_end = m_end;
      remove_suffix(len);
      return {m_end, old_end};
    }

    template<typename Fn>
    J_ALWAYS_INLINE void for_each_chunk(Fn && fn) const {
      using sv = conditional_t<IsConst, const_utf8_string_view, utf8_string_view>;
      auto begin = m_begin.m_char, end = m_begin.m_end;
      auto it = m_begin.m_it;
      while (it != m_end.m_it) {
        static_cast<Fn &&>(fn)(sv(begin, end));
        if (!++it) {
          J_ASSERT(it == m_end.m_it, "Out of range");
          return;
        }
        auto txt = it->text;
        begin = txt.begin();
        end = txt.end();
      }
      end = m_end.m_char;
      if (begin != end) {
        static_cast<Fn &&>(fn)(sv(begin, end));
      }
    }

    J_INLINE_GETTER operator const basic_rope_utf8_view<true> & () const noexcept
    { return *reinterpret_cast<const basic_rope_utf8_view<true> *>(this); }

    [[nodiscard]] u64_t begin_position() const noexcept {
      return m_begin.position();
    }
    [[nodiscard]] u64_t end_position() const noexcept {
      return m_end.position();
    }

    [[nodiscard]] bool contains_position(u64_t pos) const noexcept {
      return m_begin && pos >= m_begin.position() && pos < m_end.position();
    }
    [[nodiscard]] bool contains(const_rope_utf8_byte_iterator it) const noexcept {
      J_ASSERT(it);
      return contains_position(it.position());
    }
  private:
    iterator m_begin;
    iterator m_end;

    friend struct rope_utf8_view_code_points<IsConst>;
  };

  using rope_utf8_view = basic_rope_utf8_view<false>;
  using const_rope_utf8_view = basic_rope_utf8_view<true>;

  template<bool IsConst>
  utf8_code_point_iterator rope_utf8_view_code_points<IsConst>::begin() const noexcept {
    return utf8_code_point_iterator(static_cast<const basic_rope_utf8_view<IsConst>*>(this)->begin());
  }

  template<bool IsConst>
  utf8_code_point_iterator rope_utf8_view_code_points<IsConst>::end() const noexcept {
    return utf8_code_point_iterator(static_cast<const basic_rope_utf8_view<IsConst>*>(this)->end());
  }
}
