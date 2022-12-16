#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::descriptors {
  class descriptor_pool;
  struct descriptor_pool_description;
  DECLARE_RESOURCE(descriptor_pool, descriptor_pool_description, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::descriptors::descriptor_pool, j::rendering::vulkan::descriptors::descriptor_pool_description);
