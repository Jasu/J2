#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::physical_devices {
  class physical_device;
  DECLARE_RESOURCE(physical_device, void, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::physical_devices::physical_device, void);
