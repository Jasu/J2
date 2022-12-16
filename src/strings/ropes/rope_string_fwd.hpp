#pragma once

#include "hzd/mem.hpp"

namespace j::strings::inline ropes {
  inline constexpr u32_t rope_string_granularity_v = 32U;
  /// Number of extra bytes to allocate.
  ///
  /// This is required to make the last character accessible as a 32-bit
  /// integer (i.e. code point parsing logic can safely read the maximum
  /// length UTF-8 code sequence, i.e. 4 bytes.
  inline constexpr u32_t rope_string_tail_padding_v = 3U;
  inline constexpr u32_t max_rope_string_size_v = rope_string_granularity_v * 64U
                                                  - rope_string_tail_padding_v;

  /// Reference-counted byte string, with the buffer allocated
  /// directly after the object.
  class rope_string final {
  public:
    J_ALWAYS_INLINE void add_reference() noexcept
    { ++m_refcount; }

    void remove_reference() noexcept {
      if (!--m_refcount) {
        ::j::free(this);
      }
    }

    /// Return whether the rope string has more than one reference.
    J_INLINE_GETTER bool is_shared() const noexcept { return m_refcount != 1; }

    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }

    J_INLINE_GETTER_NONNULL const char * data() const noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<const char *>(this + 1); }

    J_INLINE_GETTER_NONNULL char * data() noexcept J_RETURNS_ALIGNED(8)
    { return reinterpret_cast<char *>(this + 1); }

    J_INLINE_GETTER_NONNULL const char * begin() const noexcept J_RETURNS_ALIGNED(8)
    { return data(); }

    J_INLINE_GETTER_NONNULL char * begin() noexcept J_RETURNS_ALIGNED(8)
    { return data(); }

    J_INLINE_GETTER_NONNULL const char * end() const noexcept J_RETURNS_ALIGNED(8)
    { return data() + m_size; }

    J_INLINE_GETTER_NONNULL char * end() noexcept J_RETURNS_ALIGNED(8)
    { return data() + m_size; }

    /// Allocate a rope_string initialized with str.
    static rope_string * allocate(
      const char * const str, u32_t size) J_RETURNS_ALIGNED(16) J_RETURNS_NONNULL;

    /// Allocate an uninitialized rope_string.
    static rope_string * allocate(u32_t size) J_RETURNS_ALIGNED(16) J_RETURNS_NONNULL;

    rope_string(const rope_string &) = delete;
    rope_string & operator=(const rope_string &) = delete;

  private:
    explicit rope_string(u32_t size) noexcept;

    u32_t m_size;
    u32_t m_refcount = 0U;
    // When adding data here, make sure to update J_RETURNS_ALIGNED in data()
  };
}
