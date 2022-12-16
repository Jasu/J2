#include "rendering/vulkan/transfer/transfer_source_buffer.hpp"

#include "rendering/vulkan/resources/resource_definition.hpp"

namespace j::rendering::vulkan::transfer {
  transfer_source_buffer::transfer_source_buffer(const device_context_base & context,
                                                 u32_t size)
    : buffers::suballocated_buffer<transfer_source_buffer_ref>(context, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size)
  {
  }
  DEFINE_RESOURCE_DEFINITION(transfer_source_buffer, u32_t, void)
}

DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::transfer::transfer_source_buffer, u32_t);
