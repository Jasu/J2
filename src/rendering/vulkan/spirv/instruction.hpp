#pragma once

#include "exceptions/assert_lite.hpp"

#include <rendering/vulkan/spirv/enums.hpp>

namespace j::rendering::vulkan::inline spirv {
  struct instruction {
    u32_t m_opcode;
    J_INLINE_GETTER u16_t word_count() const noexcept {
      return m_opcode >> 16U;
    }

    J_INLINE_GETTER u32_t size_bytes() const noexcept {
      return (m_opcode >> 14U) & ~3U;
    }

    J_INLINE_GETTER enum opcode opcode() const noexcept {
      return static_cast<enum opcode>(m_opcode & 0xFFFFU);
    }
  };
}
