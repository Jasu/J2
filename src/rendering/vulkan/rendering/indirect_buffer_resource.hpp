#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::rendering {
  class indirect_buffer;
  class indirect_count_buffer;
  class indirect_buffer_description;
  DECLARE_RESOURCE(indirect_buffer, indirect_buffer_description, void);
  DECLARE_RESOURCE(indirect_count_buffer, indirect_buffer_description, void);
}

DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::indirect_buffer, j::rendering::vulkan::rendering::indirect_buffer_description);
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::indirect_count_buffer, j::rendering::vulkan::rendering::indirect_buffer_description);
