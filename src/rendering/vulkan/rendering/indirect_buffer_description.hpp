#pragma once

#include "hzd/types.hpp"

namespace j::rendering::vulkan::rendering {
  class indirect_buffer_description final {
    u32_t m_max_count;
  public:
    constexpr indirect_buffer_description(u32_t max_count) noexcept
      : m_max_count(max_count)
    {
    }

    u32_t max_count() const noexcept {
      return m_max_count;
    }
  };
}
