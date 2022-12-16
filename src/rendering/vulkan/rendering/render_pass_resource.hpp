#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::rendering {
  class render_pass;
  DECLARE_SURFACE_RESOURCE(render_pass, void, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::render_pass, void);
