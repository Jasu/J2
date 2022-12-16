#include "rendering/vulkan/rendering/indirect_buffer.hpp"

#include "rendering/vulkan/rendering/indirect_buffer_description.hpp"
#include "rendering/vulkan/rendering/indirect_buffer_resource.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"

template class j::rendering::vulkan::buffers::detail::buffer_access<
  VkDrawIndirectCommand,
  sizeof(VkDrawIndirectCommand),
  j::rendering::vulkan::buffers::base_host_visible_buffer
>;

namespace j::rendering::vulkan::rendering {
  indirect_buffer::indirect_buffer(const device_context_base & context,
                                   const indirect_buffer_description & desc)
    : buffers::detail::buffer_access<
      VkDrawIndirectCommand,
      sizeof(VkDrawIndirectCommand),
      buffers::base_host_visible_buffer
    >(context, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, desc.max_count() * sizeof(VkDrawIndirectCommand))
  {
    zero();
  }

  indirect_count_buffer::indirect_count_buffer(const device_context_base & context,
                                               const indirect_buffer_description & desc)
    : buffers::base_host_visible_buffer(
      context,
      VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
      offset_v + sizeof(VkDrawIndirectCommand) * desc.max_count()
  ) {
    zero();
  }
  DEFINE_RESOURCE_DEFINITION(indirect_buffer, indirect_buffer_description, void)
  DEFINE_RESOURCE_DEFINITION(indirect_count_buffer, indirect_buffer_description, void)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::indirect_buffer, j::rendering::vulkan::rendering::indirect_buffer_description);
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::indirect_count_buffer, j::rendering::vulkan::rendering::indirect_buffer_description);
