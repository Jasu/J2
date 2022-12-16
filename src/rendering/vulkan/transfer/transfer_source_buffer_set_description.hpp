#pragma once

#include "hzd/types.hpp"

namespace j::rendering::vulkan::transfer {
  class transfer_source_buffer_set_description final {
    u32_t m_count = 0;
    u32_t m_size = 0;
  public:
    constexpr transfer_source_buffer_set_description() noexcept = default;
    constexpr transfer_source_buffer_set_description(u32_t count, u32_t size) noexcept
      : m_count(count),
        m_size(size)
    {
    }

    u32_t count() const noexcept {
      return m_count;
    }

    u32_t size() const noexcept {
      return m_size;
    }
  };
}
