#pragma once

#include "exceptions/assert_lite.hpp"
#include "hzd/utility.hpp"

namespace j::detail {
  class chunk {
    union {
      chunk * m_previous;
      u32_t m_begin_offset;
    };
    u32_t m_end_offset;
    u32_t m_capacity_bytes;

  public:
    J_ALWAYS_INLINE chunk * prev() const noexcept {
      return (m_begin_offset & 1U) ? nullptr : m_previous;
    }

    void set_prev(chunk * next) noexcept;

    void set_next(chunk * next) noexcept;

    u32_t begin_offset() const noexcept {
      return (m_begin_offset & 1U) ? m_begin_offset >> 1U : 0U;
    }

    void set_begin_offset(u32_t offset) noexcept {
      m_begin_offset = (offset << 1UL) + 1UL;
    }

    J_INLINE_GETTER u32_t end_offset() const noexcept {
      return m_end_offset;
    }

    void set_end_offset(u32_t end_offset) noexcept {
      J_ASSERT(end_offset <= m_capacity_bytes);
      m_end_offset = end_offset;
    }

    J_INLINE_GETTER u32_t capacity_bytes() const noexcept {
      return m_capacity_bytes;
    }

    template<typename T>
    [[nodiscard]] T * data(u32_t byte_offset = 0U) noexcept {
      J_ASSERT(m_end_offset <= m_capacity_bytes);
      return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + sizeof(chunk) + byte_offset);
    }
    template<typename T>
    [[nodiscard]] const T * data(u32_t byte_offset = 0U) const noexcept {
      J_ASSERT(m_end_offset <= m_capacity_bytes);
      return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) + sizeof(chunk) + byte_offset);
    }

    [[nodiscard]] inline chunk * next() const noexcept {
      J_ASSERT(m_end_offset <= m_capacity_bytes);
      return *reinterpret_cast<chunk * const *>(reinterpret_cast<const char*>(this) + sizeof(chunk) + m_capacity_bytes);
    }

    static chunk * allocate_chunk(chunk * previous, chunk * next, u32_t capacity_bytes);

    static chunk * release_head(chunk * c) noexcept;
    static chunk * release_tail(chunk * c) noexcept;
    static void release_chunks_forward(chunk * c) noexcept;
    static void copy_chunks_forward(const chunk * from, chunk * & head, chunk * & tail, bool memcpy_contents);
  };

  u32_t chunk_size_forward(const chunk * head, u32_t element_size) noexcept;
}
