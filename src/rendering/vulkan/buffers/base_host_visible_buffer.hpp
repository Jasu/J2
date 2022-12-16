#pragma once

#include "rendering/vulkan/buffers/raw_buffer.hpp"

namespace j::rendering::vulkan::buffers {
  /// Base class for buffers that are mapped to host memory.
  ///
  /// The buffers are always mapped. "raw" means that the buffer is not specialized for a specific
  /// buffer usage or content type.
  class base_host_visible_buffer : public raw_buffer {
  public:
    void zero() noexcept;

    J_INLINE_GETTER_NONNULL u8_t * buffer_begin() noexcept {
      return m_mapping;
    }

    J_INLINE_GETTER_NONNULL const u8_t * buffer_begin() const noexcept {
      return m_mapping;
    }
    J_INLINE_GETTER_NONNULL u8_t * buffer_end() noexcept {
      return m_mapping + size_bytes;
    }

    J_INLINE_GETTER_NONNULL const u8_t * buffer_end() const noexcept {
      return m_mapping + size_bytes;
    }

    J_INLINE_GETTER_NONNULL u8_t * buffer_data() noexcept {
      return m_mapping;
    }

    J_INLINE_GETTER_NONNULL const u8_t * buffer_data() const noexcept {
      return m_mapping;
    }

    void copy_to_buffer(VkDeviceSize offset, const void * source, VkDeviceSize size);
    void copy_to_buffer(const void * source, VkDeviceSize size);

    ~base_host_visible_buffer();

  protected:
    u8_t * m_mapping = nullptr;

    base_host_visible_buffer(const device_context_base & context,
                             VkBufferUsageFlags usage,
                             VkDeviceSize size)
      : raw_buffer(context,
                   usage,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                   size),
        m_mapping(map_memory())
    { }
  };
}
