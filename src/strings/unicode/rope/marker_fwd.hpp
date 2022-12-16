#pragma once

#include "strings/unicode/rope/common.hpp"
#include "strings/unicode/rope/rope_utf8_byte_iterator_fwd.hpp"
#include "util/flags.hpp"

namespace j::strings::inline unicode::inline rope {

  enum class marker_flag : u8_t {
    binds_to_char_after,
    track_modifications,
    modified,
  };

  using marker_flags_t = util::flags<marker_flag, u8_t>;
  J_FLAG_OPERATORS(marker_flag, u8_t)

  class marker final {
  public:
    constexpr marker() noexcept = default;

    J_ALWAYS_INLINE constexpr explicit marker(marker_flags_t flags) noexcept
      : m_flags(flags)
    { }

    J_ALWAYS_INLINE constexpr explicit marker(rope_utf8_byte_iterator it, marker_flags_t flags = {}) noexcept
      : m_flags(flags)
    {
      move(it);
    }

    J_ALWAYS_INLINE constexpr marker(marker && rhs) noexcept
      : m_chunk(rhs.m_chunk),
        m_flags(rhs.m_flags),
        m_offset(rhs.m_offset),
        m_previous(rhs.m_previous),
        m_next(rhs.m_next)
    {
      reattach(rhs);
    }

    J_ALWAYS_INLINE constexpr marker & operator=(marker && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        detach();
        m_chunk = rhs.m_chunk;
        m_flags = rhs.m_flags;
        m_offset = rhs.m_offset;
        m_previous = rhs.m_previous;
        m_next = rhs.m_next;
        reattach(rhs);
      }
      return *this;
    }

    J_ALWAYS_INLINE ~marker() {
      // detach();
    }



    J_INLINE_GETTER rope_utf8_byte_iterator as_byte_iterator() const noexcept {
      J_ASSUME_NOT_NULL(m_chunk);
      auto it = m_chunk->ref.as_iterator();
      rope_utf8_byte_iterator result = {
        m_chunk->text.begin() + m_offset,
        m_chunk->text.end(),
        it
      };
      if (result.m_char == result.m_end && it.has_next()) {
        --result.m_char;
        ++result;
      }
      return result;
    }

    J_INLINE_GETTER bool is_valid() const noexcept
    { return m_chunk; }

    J_ALWAYS_INLINE void invalidate() noexcept {
      detach();
      m_offset = 0U;
    }

    J_INLINE_GETTER constexpr bool binds_to_char_after() const noexcept
    { return m_flags.has(marker_flag::binds_to_char_after); }

    J_INLINE_GETTER constexpr bool binds_to_char_before() const noexcept
    { return !m_flags.has(marker_flag::binds_to_char_after); }

    J_INLINE_GETTER constexpr bool tracks_modifications() const noexcept
    { return m_flags.has(marker_flag::track_modifications); }

    J_INLINE_GETTER constexpr bool is_modified() const noexcept
    { return m_flags.has(marker_flag::modified); }

    J_ALWAYS_INLINE constexpr void set_modified() noexcept
    { m_flags.set(marker_flag::modified); }

    J_ALWAYS_INLINE constexpr void clear_modified() noexcept
    { m_flags.clear(marker_flag::modified); }

    J_INLINE_GETTER marker * previous() noexcept
    { return m_previous; }

    J_INLINE_GETTER const marker * previous() const noexcept
    { return m_previous; }

    J_INLINE_GETTER marker * next() noexcept
    { return m_next; }

    J_INLINE_GETTER const marker * next() const noexcept
    { return m_next; }

    J_ALWAYS_INLINE void set_previous(marker * previous) const noexcept
    { m_previous = previous; }

    J_ALWAYS_INLINE void set_next(marker * next) const noexcept
    { m_next = next; }

    J_ALWAYS_INLINE void move(rope_utf8_byte_iterator it) noexcept {
      move(&*it.m_it, it.m_char - it.m_it->text.begin());
    }

    void move(rope_utf8_value * J_NOT_NULL chunk, u16_t offset) noexcept {
      if (J_UNLIKELY(offset == 0 && binds_to_char_before() && chunk->ref.previous())) {
        chunk = chunk->ref.previous();
        offset = chunk->size_bytes();
      }
      if (m_chunk != chunk) {
        detach();
        add_to_chunk(chunk);
      }
      m_offset = offset;
    }

    inline void move_different(rope_utf8_value * J_NOT_NULL chunk, u16_t offset) noexcept;

    void add_to_chunk(rope_utf8_value * J_NOT_NULL chunk) noexcept {
      m_chunk = chunk;
      m_next = m_chunk->m_markers;
      m_chunk->m_markers = this;
      m_previous = nullptr;
      if (m_next) {
        J_ASSUME(m_next->m_previous == nullptr);
        m_next->set_previous(this);
      }
    }

    J_INLINE_GETTER rope_utf8_value * chunk() const noexcept
    { return m_chunk; }

    J_ALWAYS_INLINE void set_chunk(rope_utf8_value * J_NOT_NULL chunk) noexcept
    { m_chunk = chunk; }

    J_INLINE_GETTER u16_t offset() const noexcept
    { return m_offset; }

    J_ALWAYS_INLINE void set_offset(u16_t offset) noexcept
    { m_offset = offset; }

    bool position_equals(const marker & other) const noexcept;
  private:
    void reattach(marker & rhs) noexcept {
      if (!m_chunk) {
        J_ASSUME(m_previous == nullptr);
        J_ASSUME(m_next == nullptr);
        return;
      }
      if (m_next) {
        m_next->m_previous = this;
        rhs.m_next = nullptr;
      }
      if (m_previous) {
        m_previous->m_next = this;
        rhs.m_previous = nullptr;
      } else {
        m_chunk->m_markers = this;
      }
    }

    void detach() noexcept {
      if (m_previous) {
        m_previous->set_next(m_next);
        if (m_next) {
          m_next->set_previous(m_previous);
          m_next = nullptr;
        }
        m_previous = nullptr;
      } else if (m_chunk) {
        J_ASSUME(m_chunk->m_markers == this);
        m_chunk->m_markers = m_next;
        if (m_next) {
          m_next->set_previous(nullptr);
          m_next = nullptr;
        }
      }
      m_chunk = nullptr;
    }

    rope_utf8_value * m_chunk = nullptr;
    marker_flags_t m_flags;
    u16_t m_offset = 0U;
    mutable marker * m_previous = nullptr;
    mutable marker * m_next = nullptr;
  };
}
