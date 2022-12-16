#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/physical_devices/queue_family_set.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
}

namespace j::rendering::vulkan::physical_devices {
  namespace detail {
    struct physical_device_create_info;
  }

  class physical_device {
    VkPhysicalDevice m_vk_physical_device = VK_NULL_HANDLE;
    queue_family_set m_queue_families;
    VkPhysicalDeviceProperties m_vk_properties;
    VkPhysicalDeviceMemoryProperties m_vk_memory_properties;

    explicit physical_device(detail::physical_device_create_info && create_info);
  public:
    explicit physical_device(const device_context_base & context);

    ~physical_device();

    VkPhysicalDevice vk_physical_device() const noexcept {
      return m_vk_physical_device;
    }
    const VkPhysicalDeviceProperties & properties() const noexcept {
      return m_vk_properties;
    }

    const queue_family_set & queue_families() const noexcept {
      return m_queue_families;
    }

    /// Find a memory type by type and property flags.
    ///
    /// If no memory type is found, throws.
    ///
    /// \param type Bitmask of acceptable memory type indices, as returned by
    /// [vkGetBufferMemoryRequirements].
    /// \param property_flags Bitmask of property flags. All set bits must be set in the
    /// memory region.
    u32_t find_memory_type_index(u32_t type, VkMemoryPropertyFlags property_flags) const;

    physical_device(const physical_device &) = delete;
    physical_device & operator=(const physical_device &) = delete;
  };
}
