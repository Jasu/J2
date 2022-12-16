#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::transfer {
  class transfer_source_buffer_set;
  class transfer_source_buffer_set_description;
  DECLARE_RESOURCE(transfer_source_buffer_set, transfer_source_buffer_set_description, void);
}

DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::transfer::transfer_source_buffer_set,
                           j::rendering::vulkan::transfer::transfer_source_buffer_set_description);
