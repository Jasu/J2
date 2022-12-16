#pragma once

#include "rendering/vulkan/spirv/instruction.hpp"

namespace j::rendering::vulkan::inline spirv {
  struct instruction_iterator final {
    const u8_t * m_it;

    J_INLINE_GETTER_NONNULL const instruction * operator->() const noexcept {
      J_ASSUME_NOT_NULL(m_it);
      return reinterpret_cast<const instruction*>(m_it);
    }

    J_INLINE_GETTER const instruction & operator*() const noexcept {
      return *reinterpret_cast<const instruction*>(m_it);
    }

    J_INLINE_GETTER bool operator==(const instruction_iterator & rhs) const noexcept = default;

    J_INLINE_GETTER bool operator<(instruction_iterator rhs) const noexcept {
      return m_it < rhs.m_it;
    }

    J_ALWAYS_INLINE instruction_iterator & operator++() noexcept {
      m_it += (*this)->size_bytes();
      return *this;
    }

    J_ALWAYS_INLINE instruction_iterator operator++(int) noexcept {
      const auto result{*this};
      return operator++(), result;
    }
  };
}
