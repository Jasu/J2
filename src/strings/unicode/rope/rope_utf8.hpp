#pragma once

#include "strings/unicode/rope/common.hpp"
#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"
#include "strings/unicode/rope/rope_utf8_code_point_iterator.hpp"
#include "strings/unicode/rope/rope_utf8_view.hpp"
#include "strings/unicode/rope/marker_fwd.hpp"

J_DECLARE_EXTERN_ROPE_TREE(j::strings::rope_utf8_description);

namespace j::strings::inline unicode::inline rope {
  struct rope_utf8_chunks {
    rope_utf8_chunk_iterator begin() noexcept;
    J_INLINE_GETTER const_rope_utf8_chunk_iterator begin() const noexcept
    { return const_cast<rope_utf8_chunks *>(this)->begin(); }

    rope_utf8_chunk_iterator end() noexcept;

    J_INLINE_GETTER const_rope_utf8_chunk_iterator end() const noexcept
    { return const_cast<rope_utf8_chunks *>(this)->end(); }
  };

  struct rope_utf8_bytes {
    rope_utf8_byte_iterator begin() noexcept;
    J_INLINE_GETTER const_rope_utf8_byte_iterator begin() const noexcept
    { return const_cast<rope_utf8_bytes *>(this)->begin(); }

    rope_utf8_byte_iterator end() noexcept;
    J_INLINE_GETTER const_rope_utf8_byte_iterator end() const noexcept
    { return const_cast<rope_utf8_bytes *>(this)->end(); }

    rope_utf8_byte_iterator iterate_at(u64_t i);
    J_INLINE_GETTER const_rope_utf8_byte_iterator iterate_at(u64_t i) const noexcept
    { return const_cast<rope_utf8_bytes *>(this)->iterate_at(i); }
  };

  struct rope_utf8_code_points {
    rope_utf8_code_point_iterator begin() const noexcept;
    rope_utf8_code_point_iterator end() const noexcept;
  };

  class rope_utf8 final : rope_utf8_chunks, rope_utf8_bytes, rope_utf8_code_points {
  public:
    J_ALWAYS_INLINE rope_utf8() noexcept = default;
    J_ALWAYS_INLINE rope_utf8(const_string_view str) noexcept {
      prepend(str);
    }

    void prepend(const_string_view str);
    void append(const_string_view str);
    rope_utf8_byte_iterator insert(const_rope_utf8_byte_iterator at, const_string_view str);

    void insert(u64_t at, const_string_view str) {
      if (J_LIKELY(str)) {
        insert(bytes().iterate_at(at), str);
      }
    }

    void erase(u64_t at, u64_t size);

    rope_utf8_byte_iterator erase(const const_rope_utf8_byte_iterator at, u64_t size);

    rope_utf8_byte_iterator erase(const const_rope_utf8_byte_iterator start,
                                  const const_rope_utf8_byte_iterator end);

    rope_utf8_code_point_iterator erase(const rope_utf8_code_point_iterator at, u64_t size);

    rope_utf8_code_point_iterator erase(const rope_utf8_code_point_iterator start,
                                        const rope_utf8_code_point_iterator end);

    rope_utf8_byte_iterator split(const_rope_utf8_byte_iterator at);

    rope_utf8_byte_iterator split(u64_t at);

    rope_utf8_view view_at(const_rope_utf8_byte_iterator begin,
                           const_rope_utf8_byte_iterator end) noexcept;

    const const_rope_utf8_view view_at(const_rope_utf8_byte_iterator begin,
                                       const_rope_utf8_byte_iterator end) const noexcept;

    J_INLINE_GETTER u64_t size_bytes() const noexcept
    { return m_tree.metrics().size_bytes; }

    J_INLINE_GETTER u64_t num_hard_breaks() const noexcept
    { return m_tree.metrics().num_hard_breaks; }

    rope_utf8_byte_iterator find_line(u64_t line_number);
    J_INLINE_GETTER const_rope_utf8_byte_iterator find_line(u64_t line_number) const
    { return const_cast<rope_utf8 *>(this)->find_line(line_number); }

    J_INLINE_GETTER bool empty() const noexcept { return m_tree.empty(); }

    J_INLINE_GETTER rope_utf8_chunks & chunks() noexcept { return *this; }
    J_INLINE_GETTER const rope_utf8_chunks & chunks() const noexcept { return *this; }

    J_INLINE_GETTER rope_utf8_bytes & bytes() noexcept { return *this; }
    J_INLINE_GETTER const rope_utf8_bytes & bytes() const noexcept { return *this; }

    J_INLINE_GETTER const rope_utf8_code_points & code_points() const noexcept { return *this; }

    operator rope_utf8_view() noexcept;
    operator const_rope_utf8_view() const noexcept;
    rope_utf8_tree m_tree;
  private:
    friend struct rope_utf8_chunks;
    friend struct rope_utf8_bytes;
    friend struct rope_utf8_code_points;
  };

  inline rope_utf8_chunk_iterator rope_utf8_chunks::begin() noexcept
  { return static_cast<rope_utf8*>(this)->m_tree.begin(); }

  inline rope_utf8_chunk_iterator rope_utf8_chunks::end() noexcept
  { return static_cast<rope_utf8*>(this)->m_tree.end(); }

  inline rope_utf8_byte_iterator rope_utf8_bytes::begin() noexcept {
    const auto it = static_cast<rope_utf8*>(this)->m_tree.begin();
    const auto text = it->text;
    return rope_utf8_byte_iterator{text.begin(), text.end(), it};
  }

  inline rope_utf8_byte_iterator rope_utf8_bytes::end() noexcept {
    const auto it = static_cast<rope_utf8*>(this)->m_tree.before_end();
    const auto ptr = it->text.end();
    return rope_utf8_byte_iterator{ptr, ptr, it};
  }

  inline rope_utf8_byte_iterator rope_utf8_bytes::iterate_at(u64_t i) {
    if (i == 0ULL) {
      return begin();
    }
    char *begin = nullptr, *end = nullptr;
    rope_utf8_chunk_iterator it;
    const auto p = static_cast<rope_utf8*>(this)->m_tree.find(i);
    it = p.first;
    if (J_UNLIKELY(!it)) {
      --it;
      begin = end = it->text.end();
    } else {
      const auto text = it->text;
      begin = text.begin() + p.second;
      end = text.end();
      if (begin == end && it.has_next()) {
        ++it;
        begin = it->text.begin();
        end = it->text.end();
      }
    }
    return rope_utf8_byte_iterator{begin, end, it};
  }

  inline rope_utf8_code_point_iterator rope_utf8_code_points::begin() const noexcept {
    return rope_utf8_code_point_iterator(
      static_cast<const rope_utf8*>(this)->bytes().begin());
  }

  inline rope_utf8_code_point_iterator rope_utf8_code_points::end() const noexcept {
    return rope_utf8_code_point_iterator(
      static_cast<const rope_utf8*>(this)->bytes().end());
  }
}
