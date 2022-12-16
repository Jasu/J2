#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"

namespace j::rendering::vulkan::descriptors {
  struct descriptor_pool_description;

  class descriptor_pool {
  public:
    /// Create a descriptor pool with [num_sets] sets, each according to [description].
    descriptor_pool(const device_context_base & context,
                    descriptor_pool_description && description);

    descriptor_pool(descriptor_pool &&) = delete;
    descriptor_pool & operator=(descriptor_pool &&) = delete;

    ~descriptor_pool();

    VkDescriptorSet allocate(const device_context_base & context, descriptor_set_layout_ref layout);
  private:
    VkDescriptorPool m_vk_descriptor_pool = VK_NULL_HANDLE;
    weak_device_ref m_device;
  };
}
