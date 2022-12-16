#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::samplers {
  struct sampler;
  DECLARE_RESOURCE(sampler, void, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::samplers::sampler, void);
