#include "rendering/vulkan/descriptors/descriptor_set.hpp"

#include "rendering/vulkan/descriptors/descriptor_pool.hpp"
#include "rendering/vulkan/descriptors/descriptor_pool_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"

namespace j::rendering::vulkan::descriptors {
  descriptor_set::descriptor_set(const device_context_base & context,
                                 descriptor_set_description && description)
    : layout(static_cast<descriptor_set_layout_ref &&>(description.layout)),
      pool(attributes::layout = this->layout)
  {
    J_ASSERT_NOT_NULL(this->layout);
    vk_descriptor_set = context.get(pool).allocate(context, this->layout);
  }
  DEFINE_RESOURCE_DEFINITION(descriptor_set, descriptor_set_description, descriptor_set_state)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_set, j::rendering::vulkan::descriptors::descriptor_set_description);
