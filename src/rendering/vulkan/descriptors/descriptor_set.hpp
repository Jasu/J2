#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/descriptors/descriptor_pool_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"

namespace j::rendering::vulkan::descriptors {
  struct descriptor_set_description;

  /// A set of references to resources that can be bound in a command buffer.
  ///
  /// No automatic cleanup - it is assumed that the pool will be destroyed at once.
  class descriptor_set final {
  public:
    descriptor_set(const device_context_base & context,
                   descriptor_set_description && description);

    descriptor_set(descriptor_set &&) = delete;
    descriptor_set & operator=(descriptor_set &&) = delete;

    VkDescriptorSet vk_descriptor_set;
    descriptor_set_layout_ref layout;
    descriptor_pool_ref pool;
  };
}
