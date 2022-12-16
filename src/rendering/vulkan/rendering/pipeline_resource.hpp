#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"

namespace j::rendering::vulkan::rendering {
  class pipeline;
  struct pipeline_description;
  DECLARE_SURFACE_RESOURCE(pipeline, pipeline_description, void);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::rendering::pipeline, j::rendering::vulkan::rendering::pipeline_description);
