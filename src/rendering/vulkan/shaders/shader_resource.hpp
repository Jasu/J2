#pragma once

#include "rendering/vulkan/resources/resource_declaration.hpp"
#include "rendering/vulkan/resources/weak_resource_ref.hpp"

namespace j::files::inline paths { class path; }
namespace j::rendering::vulkan::shaders {
  struct shader_module;
  DECLARE_RESOURCE(shader_module, files::path, void);
  DECLARE_WEAK_RESOURCE_REF(shader_module, files::path);
}
DECLARE_RESOURCE_TEMPLATES(j::rendering::vulkan::shaders::shader_module, j::files::path);
DECLARE_WEAK_RESOURCE_TEMPLATES(j::rendering::vulkan::shaders::shader_module, j::files::path);
