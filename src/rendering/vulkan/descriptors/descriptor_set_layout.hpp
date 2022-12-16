#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/device_resource.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::rendering::vulkan::descriptors {
  class bound_descriptor_binding_description;
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::descriptors::bound_descriptor_binding_description);

namespace j::rendering::vulkan::descriptors {
  struct descriptor_binding_description;
  struct descriptor_set_layout_description;

  /// Wraps [VkDescriptorSetLayout].
  ///
  /// Created by [descriptor_set_description]().
  class descriptor_set_layout final {
  public:
    descriptor_set_layout(const device_context_base & context,
                          descriptor_set_layout_description && description);

    const descriptor_binding_description & find_binding(u32_t binding) const;

    descriptor_set_layout(descriptor_set_layout &&) = delete;
    descriptor_set_layout & operator=(descriptor_set_layout &&) = delete;

    ~descriptor_set_layout();

    VkDescriptorSetLayout vk_descriptor_set_layout;
    trivial_array<bound_descriptor_binding_description> bindings;
    weak_device_ref device;
  };
}
