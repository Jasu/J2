#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::descriptors {
  class descriptor_set_layout;
  struct descriptor_set_layout_description;
  DECLARE_RESOURCE(descriptor_set_layout, descriptor_set_layout_description, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_set_layout, j::rendering::vulkan::descriptors::descriptor_set_layout_description);
