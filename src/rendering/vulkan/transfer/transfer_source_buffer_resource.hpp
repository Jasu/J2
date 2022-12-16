#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::buffers {
  class buffer_description;
}

namespace j::rendering::vulkan::transfer {
  class transfer_source_buffer;
  DECLARE_RESOURCE(transfer_source_buffer, u32_t, void);
}

DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::transfer::transfer_source_buffer, u32_t);
