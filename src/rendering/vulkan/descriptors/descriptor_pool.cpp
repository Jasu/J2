#include "rendering/vulkan/descriptors/descriptor_pool.hpp"

#include "rendering/vulkan/descriptors/descriptor_set_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"
#include "rendering/vulkan/utils.hpp"
#include "rendering/vulkan/device.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/descriptors/descriptor_pool_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_pool_resource.hpp"

namespace j::rendering::vulkan::descriptors {
  descriptor_pool::descriptor_pool(const device_context_base & context,
                                   descriptor_pool_description && description)
    : m_device(context.device_ref())
  {
    u32_t num_uniforms = description.num_uniforms;
    u32_t num_dynamic_uniforms = 0;
    u32_t num_combined = description.num_combined_image_samplers;
    u32_t num_sets = description.max_sets;
    for (auto & rec : description.layouts) {
      J_ASSERT_NOT_NULL(rec.count);
      num_sets += rec.count;

      // TODO: Don't instantiate the layout - check description.
      auto & layout = context.get(rec.layout);
      for (auto & binding : layout.bindings) {
        switch (binding.binding.type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
          num_uniforms += rec.count;
        break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
          num_combined += rec.count;
        break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
          num_dynamic_uniforms += rec.count;
        break;
        default:
          J_THROW("Unknown binding type");
        }
      }
    }
    u32_t num_descriptor_types = 0;
    VkDescriptorPoolSize pool_sizes[3];
    if (num_uniforms) {
      pool_sizes[num_descriptor_types++] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = num_uniforms,
      };
    }
    if (num_combined) {
      pool_sizes[num_descriptor_types++] = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = num_combined,
      };
    }
    if (num_dynamic_uniforms) {
      pool_sizes[num_descriptor_types++] = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        .descriptorCount = num_dynamic_uniforms,
      };
    }
    VkDescriptorPoolCreateInfo info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = num_sets,
      .poolSizeCount = num_descriptor_types,
      .pPoolSizes = pool_sizes,
    };
    J_VK_CALL_CHECKED(vkCreateDescriptorPool, context.device().vk_device, &info, nullptr, &m_vk_descriptor_pool);
  }

  descriptor_pool::~descriptor_pool() {
    if (auto device = m_device.lock()) {
      vkDestroyDescriptorPool(device.get().vk_device, m_vk_descriptor_pool, nullptr);
    }
  }

  VkDescriptorSet descriptor_pool::allocate(
    const device_context_base & context,
    descriptor_set_layout_ref layout
  ) {
    J_ASSERT_NOT_NULL(layout);
    const VkDescriptorSetAllocateInfo info{
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = m_vk_descriptor_pool,
      .descriptorSetCount = 1,
      .pSetLayouts = &context.get(layout).vk_descriptor_set_layout,
    };
    VkDescriptorSet result = VK_NULL_HANDLE;
    J_VK_CALL_CHECKED(vkAllocateDescriptorSets, context.device().vk_device, &info, &result);
    J_ASSERT_NOT_NULL(result);
    return result;
  }
  DEFINE_RESOURCE_DEFINITION(descriptor_pool, descriptor_pool_description, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_pool, j::rendering::vulkan::descriptors::descriptor_pool_description);
