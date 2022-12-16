#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::descriptors {
  class descriptor_set;
  struct descriptor_set_description;
  struct descriptor_set_state;
  DECLARE_RESOURCE(descriptor_set, descriptor_set_description, descriptor_set_state);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_set, j::rendering::vulkan::descriptors::descriptor_set_description);
