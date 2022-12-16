#include "rendering/vulkan/buffers/raw_buffer.hpp"

#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/physical_devices/physical_device.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"

namespace j::rendering::vulkan::buffers {

  raw_buffer::raw_buffer(const device_context_base & context,
                         VkBufferUsageFlags usage_flags,
                         VkMemoryPropertyFlags memory_properties,
                         VkDeviceSize size)
    : size_bytes(size),
      device(context.device_ref())
  {
    VkDevice vk_device = context.device().vk_device;
    J_ASSERT_NOT_NULL(usage_flags, size, vk_device);

    const VkBufferCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = size,
      .usage = usage_flags,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices = nullptr,
    };
    J_VK_CALL_CHECKED(vkCreateBuffer, vk_device, &create_info, nullptr, &vk_buffer);
    J_ASSERT_NOT_NULL(vk_buffer);

    VkMemoryRequirements requirements = { .size = 0 };
    vkGetBufferMemoryRequirements(vk_device, vk_buffer, &requirements);
    J_ASSERT(requirements.size >= size, "Requirement size mismatch.");
    try {
      u32_t index = context
        .physical_device()
        .find_memory_type_index(requirements.memoryTypeBits, memory_properties);
      const VkMemoryAllocateInfo allocate_info{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        // Note that requirements may have a larger size than requested.
        .allocationSize = requirements.size,
        .memoryTypeIndex = index,
      };
      J_VK_CALL_CHECKED(vkAllocateMemory, vk_device, &allocate_info, nullptr, &vk_device_memory);
      J_ASSERT_NOT_NULL(vk_device_memory);
      J_VK_CALL_CHECKED(vkBindBufferMemory, vk_device, vk_buffer, vk_device_memory, 0);
    } catch (...) {
      if (vk_device_memory != VK_NULL_HANDLE) {
        vkFreeMemory(vk_device, vk_device_memory, nullptr);
      }
      vkDestroyBuffer(vk_device, vk_buffer, nullptr);
      throw;
    }
  }

  void raw_buffer::free_resources() noexcept {
    // Move constructor and assignment operator sets vk_buffer to VK_NULL_HANDLE.
    if (J_LIKELY(vk_buffer != VK_NULL_HANDLE)) {
      if (auto dev = device.lock()) {
        const VkDevice d = dev.get().vk_device;
        vkDestroyBuffer(d, vk_buffer, nullptr);
        vkFreeMemory(d, vk_device_memory, nullptr);
        vk_device_memory = nullptr, vk_buffer = nullptr;
      }
    }
  }

  u8_t * raw_buffer::map_memory() {
    J_ASSERT_NOT_NULL(vk_device_memory);
    void * result = nullptr;
    auto dev = device.lock();
    J_VK_CALL_CHECKED(vkMapMemory,
                      dev.get().vk_device,
                      vk_device_memory,
                      0, // Offset
                      VK_WHOLE_SIZE,
                      0, // Flags (reserved)
                      &result);
    J_ASSERT_NOT_NULL(result);
    return reinterpret_cast<u8_t *>(result);
  }

  void raw_buffer::unmap_memory() noexcept {
    if (J_LIKELY(vk_buffer != VK_NULL_HANDLE)) {
      if (auto dev = device.lock()) {
        vkUnmapMemory(dev.get().vk_device, vk_device_memory);
      }
    }
  }

  raw_buffer::~raw_buffer() {
    free_resources();
  }

#ifndef NDEBUG
  void raw_buffer::assert_range_in_bounds(VkDeviceSize begin, VkDeviceSize size) const noexcept {
    J_VK_ASSERT(begin < size, "Buffer begin index was out of bounds.");
    J_VK_ASSERT(size <= size, "Buffer end index was out of bounds.");
  }

  void raw_buffer::assert_region_valid(const void * ptr, VkDeviceSize size) const noexcept {
    J_ASSERT_NOT_NULL(ptr, size);
    J_VK_ASSERT(size <= size, "Buffer size was out of bounds.");
  }
#endif
}
