#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_buffer_resource.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    class uniform_buffer_allocation {
    public:
      constexpr uniform_buffer_allocation() noexcept = default;

      uniform_buffer_allocation(uniform_buffer_ref buffer,
                                u32_t offset,
                                u16_t allocation_index,
                                u16_t layout_index) noexcept;

      /// Get a pointer to the memory region of the buffer (on host).
      u8_t * target() noexcept;

      /// Free the transfer operation source in [m_buffer].
      void release() noexcept;

      J_INLINE_GETTER bool empty() const noexcept {
        return !(bool)buffer;
      }

      J_INLINE_GETTER explicit operator bool() const noexcept {
        return (bool)buffer;
      }

      J_INLINE_GETTER bool operator!() const noexcept {
        return !(bool)buffer;
      }

      uniform_buffer_ref buffer;
      u32_t offset = 0U;
      u16_t allocation_index = 0U;
      u16_t layout_index = 0U;
    };
  }
}
