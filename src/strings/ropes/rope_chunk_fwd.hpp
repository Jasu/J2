#pragma once

#include "containers/trees/binary_tree_node.hpp"
#include "strings/string_view.hpp"

namespace j::strings::inline ropes {
  class rope_string;

  template<typename ChunkInfo>
  class rope_chunk final : public containers::trees::threaded_binary_tree_node_base {
  public:
    rope_chunk(sz_t at, rope_string * J_NOT_NULL const string, string_view str) noexcept;

    J_INLINE_GETTER_NONNULL const char * data() const noexcept
    { return m_view.data(); }

    J_INLINE_GETTER_NONNULL char * data() noexcept
    { return m_view.data(); }

    J_INLINE_GETTER bool is_first() const noexcept
    { return !previous; }

    J_INLINE_GETTER bool is_last() const noexcept
    { return !next; }

    J_INLINE_GETTER rope_chunk * previous_chunk() noexcept
    { return static_cast<rope_chunk*>(previous); }

    J_INLINE_GETTER const rope_chunk * previous_chunk() const noexcept
    { return static_cast<const rope_chunk*>(previous); }

    J_INLINE_GETTER rope_chunk * next_chunk() noexcept
    { return static_cast<rope_chunk*>(next); }

    J_INLINE_GETTER const rope_chunk * next_chunk() const noexcept
    { return static_cast<rope_chunk*>(next); }

    J_INLINE_GETTER_NONNULL char * begin() noexcept {
      J_ASSERT_NOT_NULL(m_view);
      return m_view.begin();
    }

    J_INLINE_GETTER_NONNULL const char * begin() const noexcept
    { return const_cast<rope_chunk*>(this)->begin(); }

    J_INLINE_GETTER_NONNULL char * end() noexcept {
      J_ASSERT_NOT_NULL(m_view);
      return m_view.end();
    }
    J_INLINE_GETTER_NONNULL const char * end() const noexcept
    { return const_cast<rope_chunk*>(this)->end(); }

    J_INLINE_GETTER u32_t size() const noexcept {
      J_ASSERT_NOT_NULL(m_view);
      return m_view.size();
    }

    J_INLINE_GETTER sz_t end_index() const noexcept
    { return key + m_view.size(); }

    void remove_prefix(u32_t sz) noexcept;

    void remove_suffix(u32_t sz) noexcept;

    void initialize() noexcept;

    void erase() noexcept;

    [[nodiscard]] rope_chunk split(u32_t it1, u32_t it2) noexcept;

    J_INLINE_GETTER ChunkInfo & info() noexcept
    { return m_info; }

    J_INLINE_GETTER const ChunkInfo & info() const noexcept
    { return m_info; }

    J_INLINE_GETTER_NONNULL rope_string * string() const noexcept {
      J_ASSERT_NOT_NULL(m_string);
      return m_string;
    }

    J_ALWAYS_INLINE rope_chunk(rope_chunk && rhs) noexcept
      : key(rhs.key),
        m_view(rhs.m_view),
        m_info(static_cast<ChunkInfo &&>(rhs.m_info)),
        m_string(rhs.m_string)
    {
      rhs.m_view.clear();
      rhs.m_string = nullptr;
    }

    J_ALWAYS_INLINE rope_chunk(sz_t key, rope_chunk && rhs) noexcept
      : rope_chunk(static_cast<rope_chunk &&>(rhs))
    { this->key = key; }

    J_INLINE_GETTER operator string_view() noexcept {
      J_ASSERT_NOT_NULL(m_view);
      return m_view;
    }

    J_INLINE_GETTER operator const_string_view() const noexcept {
      J_ASSERT_NOT_NULL(m_view);
      return m_view;
    }

    ~rope_chunk();

    rope_chunk(const rope_chunk &) = delete;
    rope_chunk & operator=(const rope_chunk &) = delete;
  public:
    /// Index of the first character in the chunk.
    sz_t key;
  private:
    /// Pointer to the start of the string in the rope_string.
    J_PACKED string_view m_view;
    /// Auxiliary info about the chunk.
    ChunkInfo m_info;
    /// The rope string containing the string of this chunk.
    ///
    /// \note Since rope chunks may be split (e.g. when inserting a chunk
    ///       in the middle of another), the string may be owned by multiple
    ///       chunks.
    rope_string * m_string;
  };
}
