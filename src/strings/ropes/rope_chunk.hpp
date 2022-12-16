#pragma once

#include "strings/ropes/rope_chunk_fwd.hpp"
#include "strings/ropes/rope_string.hpp"

namespace j::strings::inline ropes {
  template<typename ChunkInfo>
  rope_chunk<ChunkInfo>::rope_chunk(sz_t at,
                                    rope_string * J_NOT_NULL const string,
                                    string_view data) noexcept
    : key(at),
      m_view(data),
      m_string(string)
  {
    J_ASSERT_RANGE(1U, m_view.size(), m_string->size() + 1U);
    m_string->add_reference();
  }

  template<typename ChunkInfo>
  J_ALWAYS_INLINE void rope_chunk<ChunkInfo>::initialize() noexcept {
    m_info.initialize(*this);
  }

  template<typename ChunkInfo>
  J_ALWAYS_INLINE void rope_chunk<ChunkInfo>::erase() noexcept {
    m_info.erase(*this);
  }

  template<typename ChunkInfo>
  void rope_chunk<ChunkInfo>::remove_prefix(u32_t sz) noexcept {
    J_ASSERT_RANGE(1, sz, m_view.size());
    m_info.remove_prefix(*this, sz);
    m_view.remove_prefix(sz);
  }

  template<typename ChunkInfo>
  void rope_chunk<ChunkInfo>::remove_suffix(u32_t sz) noexcept {
    J_ASSERT_RANGE(1, sz, m_view.size());
    m_info.remove_suffix(*this, sz);
    m_view.remove_suffix(sz);
  }

  template<typename ChunkInfo>
  [[nodiscard]] rope_chunk<ChunkInfo> rope_chunk<ChunkInfo>::split(u32_t it1, u32_t it2) noexcept {
    J_ASSERT_NOT_NULL(it1);
    J_ASSERT_RANGE(it1, it2, m_view.size());
    auto result = rope_chunk(key + (sz_t)it1, m_string, m_view.without_prefix(it2));
    result.m_info = m_info.split(*this, it1, it2);
    m_view = m_view.prefix(it1);
    return static_cast<rope_chunk &&>(result);
  }

  template<typename ChunkInfo>
  rope_chunk<ChunkInfo>::~rope_chunk() {
    if (m_string) {
      m_string->remove_reference();
    }
  }
}
