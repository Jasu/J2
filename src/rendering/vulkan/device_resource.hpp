#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"
#include "rendering/vulkan/resources/weak_resource_ref.hpp"

namespace j::rendering::vulkan {
  struct device;
  DECLARE_RESOURCE(device, void, void);
  DECLARE_WEAK_RESOURCE_REF(device, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::device, void);
DECLARE_WEAK_RESOURCE_TEMPLATES(j::rendering::vulkan::device, void);
