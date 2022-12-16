#pragma once

#include "strings/ropes/rope_string_fwd.hpp"
#include "hzd/string.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::strings::inline ropes {
  J_RETURNS_NONNULL inline rope_string * rope_string::allocate(const char * const str, u32_t size) {
    rope_string * const ptr = allocate(size);
    if (str) {
      ::j::memcpy(reinterpret_cast<char *>(ptr + 1), str, size);
    }
    return ptr;
  }

  J_RETURNS_NONNULL inline rope_string * rope_string::allocate(u32_t size) {
    J_ASSERT_RANGE(1, size, max_rope_string_size_v + 1);
    const u32_t aligned_size = ::j::align_up(size + rope_string_tail_padding_v,
                                             rope_string_granularity_v);
    rope_string * const ptr = reinterpret_cast<rope_string*>(
      ::j::allocate(sizeof(rope_string) + aligned_size));
    char * const data = reinterpret_cast<char *>(ptr + 1);
    while (size != aligned_size) {
      data[size++] = '\0';
    }
    return ::new (ptr) rope_string(aligned_size - rope_string_tail_padding_v);
  }

  J_ALWAYS_INLINE rope_string::rope_string(u32_t size) noexcept
    : m_size(size)
  { J_ASSERT_NOT_NULL(m_size); }
}
