#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"

namespace j::rendering::vulkan::buffers {
  class buffer_description;
  class raw_buffer_description;
  class base_buffer_description;

  /// Base class for all buffers.
  ///
  /// "raw" means that the buffer is not specialized for a specific buffer usage or content type.
  struct raw_buffer {
    /// Constructs an empty, unusable buffer.
    constexpr raw_buffer() noexcept = default;

    ~raw_buffer();

    /// Throw if [index] is greater than or equal to the buffer size.
    void assert_range_in_bounds(VkDeviceSize begin, VkDeviceSize size) const noexcept;

    /// Throw if [size] is greater than buffer size or zero.
    void assert_region_valid(const void * ptr, VkDeviceSize size) const noexcept;

    raw_buffer & operator=(const raw_buffer &) = delete;
    raw_buffer(const raw_buffer &) = delete;

    /// The buffer.
    VkBuffer vk_buffer = VK_NULL_HANDLE;
    /// Size of the memory reserved for the buffer.
    VkDeviceSize size_bytes = 0;

    weak_device_ref device;

    /// Device memory associated with the buffer.
    VkDeviceMemory vk_device_memory = VK_NULL_HANDLE;

    /// Map the buffer to host memory.
    [[nodiscard]] J_RETURNS_NONNULL u8_t * map_memory();

    /// Unmap the buffer from host memory.
    void unmap_memory() noexcept;

    /// Constructor for buffers.
    ///
    /// \param size Size of the buffer in bytes.
    raw_buffer(const device_context_base & context,
               VkBufferUsageFlags usage_flags,
               VkMemoryPropertyFlags memory_properties,
               VkDeviceSize size);

  private:
    void free_resources() noexcept;
  };

  #ifdef NDEBUG
  J_ALWAYS_INLINE void raw_buffer::assert_range_in_bounds(VkDeviceSize, VkDeviceSize) const noexcept
  { }

  J_ALWAYS_INLINE void raw_buffer::assert_region_valid(const void *, VkDeviceSize) const noexcept
  { }
  #endif
}
