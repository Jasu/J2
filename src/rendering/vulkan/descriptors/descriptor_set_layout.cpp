#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"

#include "rendering/vulkan/descriptors/descriptor_set_layout_description.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::descriptors::bound_descriptor_binding_description);

namespace j::rendering::vulkan::descriptors {
  descriptor_set_layout::descriptor_set_layout(const device_context_base & context,
                                               descriptor_set_layout_description && description)
    : bindings(static_cast<trivial_array<bound_descriptor_binding_description> &&>(description.bindings)),
      device(context.device_ref())
  {
    const u32_t size = bindings.size();
    J_ASSERT_NOT_NULL(size);
    VkDescriptorSetLayoutBinding vk_bindings[size];
    for (sz_t i = 0; i < size; ++i) {
      bound_descriptor_binding_description & binding = bindings[i];
      vk_bindings[i] = {
        .binding            = binding.index,
        .descriptorType     = binding.binding.type,
        .descriptorCount    = 1,
        .stageFlags         = binding.binding.stages,
        .pImmutableSamplers = nullptr,
      };
      J_ASSERT_NOT_NULL(vk_bindings[i].stageFlags);
    }
    const VkDescriptorSetLayoutCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = size,
      .pBindings = vk_bindings,
    };
    J_VK_CALL_CHECKED(vkCreateDescriptorSetLayout, context.device().vk_device, &create_info,
                      nullptr, &vk_descriptor_set_layout);
  }

  const descriptor_binding_description & descriptor_set_layout::find_binding(u32_t binding) const {
    for (auto & b : bindings) {
      if (b.index == binding) {
        return b.binding;
      }
    }
    J_THROW("Binding not found");
  }

  descriptor_set_layout::~descriptor_set_layout() {
    if (auto dev = device.lock()) {
      vkDestroyDescriptorSetLayout(dev.get().vk_device, vk_descriptor_set_layout, nullptr);
    }
  }
  DEFINE_RESOURCE_DEFINITION(descriptor_set_layout, descriptor_set_layout_description, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_set_layout, j::rendering::vulkan::descriptors::descriptor_set_layout_description);
