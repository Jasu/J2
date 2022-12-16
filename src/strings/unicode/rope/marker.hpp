#pragma once

#include "strings/unicode/rope/marker_fwd.hpp"
#include "containers/trees/rope_tree_algo.hpp"

namespace j::strings::inline unicode::inline rope {
  inline bool operator<(const marker & lhs, const marker & rhs) noexcept {
    auto lhs_chunk = lhs.chunk(), rhs_chunk = rhs.chunk();
    u8_t lhs_offset = lhs.offset(), rhs_offset = rhs.offset();
    if (lhs_chunk == rhs_chunk) {
      return lhs_offset < rhs_offset;
    }
    // Handle case where markers are bound to the last char of the string
    // and should actually be at the next position. This can make two
    // markers compare as non-equal when they are not.
    if ((lhs_offset == 0 && rhs.binds_to_char_before()
         && rhs_offset == rhs_chunk->text.size()
         && rhs_chunk->ref.next() == lhs_chunk)) {
      return false;
    }

    if ((rhs_offset == 0 && lhs.binds_to_char_before()
         && lhs_offset == lhs_chunk->text.size()
         && lhs_chunk->ref.next() == rhs_chunk)) {
      return false;
    }

    return containers::trees::rope_value_before(*lhs_chunk, *rhs_chunk);
  }

  J_INLINE_GETTER bool operator<=(const marker & lhs, const marker & rhs) noexcept
  { return !operator<(rhs, lhs); }

  J_INLINE_GETTER bool operator>(const marker & lhs, const marker & rhs) noexcept
  { return operator<(rhs, lhs); }

  J_INLINE_GETTER bool operator>=(const marker & lhs, const marker & rhs) noexcept
  { return !operator<(lhs, rhs); }

  inline i64_t operator-(const marker & lhs, const marker & rhs) noexcept {
    return lhs.offset() - rhs.offset()
         + containers::trees::rope_value_distance(*rhs.chunk(), *lhs.chunk());
  }

  inline void marker::move_different(rope_utf8_value * J_NOT_NULL chunk, u16_t offset) noexcept {
    detach();
    add_to_chunk(chunk);
    m_offset = offset;
  }

  inline bool marker::position_equals(const marker & other) const noexcept {
    if (m_chunk == other.m_chunk) {
      return m_offset == other.m_offset;
    }
    if (m_offset == 0
        && other.binds_to_char_before()
        && other.m_offset == other.m_chunk->size_bytes()
        && other.m_chunk->next() == m_chunk)
    {
      return true;
    }

    if (other.m_offset == 0
        && binds_to_char_before()
        && m_offset == m_chunk->size_bytes()
        && m_chunk->next() == other.m_chunk->next())
    {
      return true;
    }

    return false;
  }

  template<typename Chunk, typename Fn>
  J_ALWAYS_INLINE_NO_DEBUG void for_each_marker(
    Chunk & value,
    Fn && callback
  ) noexcept {
    auto * J_RESTRICT cur = value.markers();
    while (cur) {
      auto * J_RESTRICT next = cur->next();
      static_cast<Fn &&>(callback)(*cur);
      cur = next;
    }
  }

  enum class binding_to_previous_t { v };
  enum class binding_to_next_t { v };

  constexpr inline binding_to_previous_t binding_to_previous{binding_to_previous_t::v};
  constexpr inline binding_to_next_t binding_to_next{binding_to_next_t::v};

  template<typename Chunk, typename Fn, typename Binding = int>
  J_ALWAYS_INLINE_NO_DEBUG void for_each_marker_at(
    Chunk & value,
    u32_t offset,
    Fn && callback,
    Binding = 0
  ) noexcept {
    J_ASSUME(offset <= U16_MAX);
    for_each_marker(value, [&](auto & m) noexcept {
      if (J_UNLIKELY(m.offset() == offset)) {
        if constexpr (is_same_v<Binding, binding_to_previous_t>) {
          if (!m.binds_to_char_before()) {
            return;
          }
        } else if constexpr (is_same_v<Binding, binding_to_next_t>) {
          if (!m.binds_to_char_after()) {
            return;
          }
        }
        static_cast<Fn &&>(callback)(m);
      }
    });
  }

  template<typename Chunk, typename Fn, typename Binding = int>
  J_ALWAYS_INLINE_NO_DEBUG void for_each_marker_after(
    Chunk & value,
    u32_t offset,
    Fn && callback
  ) noexcept {
    J_ASSUME(offset <= U16_MAX);
    for_each_marker(value, [&](auto & m) noexcept {
      if (m.offset() >= offset) {
        if (J_UNLIKELY(m.offset() == offset) && m.binds_to_char_before()) {
          return;
        }
        static_cast<Fn &&>(callback)(m);
      }
    });
  }

  template<typename Chunk, typename Fn, typename Binding = int>
  J_ALWAYS_INLINE_NO_DEBUG void for_each_marker_before(
    Chunk & value,
    u32_t offset,
    Fn && callback
  ) noexcept {
    J_ASSUME(offset <= U16_MAX);
    for_each_marker(value, [&](auto & m) noexcept {
      if (m.offset() <= offset) {
        if (J_UNLIKELY(m.offset() == offset) && m.binds_to_char_after()) {
          return;
        }
        static_cast<Fn &&>(callback)(m);
      }
    });
  }

}
