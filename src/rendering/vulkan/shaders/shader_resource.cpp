#include "rendering/vulkan/shaders/shader_resource.hpp"
#include "rendering/vulkan/resources/resource_definition.hpp"
#include "rendering/vulkan/shaders/shader_manager.hpp"
#include "rendering/vulkan/shaders/shader_module.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"

namespace j::rendering::vulkan::shaders {
  namespace f = files;
  namespace {
    using namespace j::rendering::vulkan::resources;
    bool get_shader(const device_context_base & context, const f::path & path, shader_module_ref & ref) {
      weak_shader_module_ref * weak = context.shader_manager().try_get_shader(path);
      if (!weak) {
        return false;
      }
      if (shader_module_ref prev = weak->lock()) {
        ref = static_cast<shader_module_ref &&>(prev);
        return true;
      }
      return false;
    }

    J_ALWAYS_INLINE void set_shader(const device_context_base & context,
                                 const f::path & path,
                                 shader_module_ref & ref) {
      context.shader_manager().set_shader(path, ref);
    }
  }

  DEFINE_RESOURCE_DEFINITION(shader_module, f::path, void, &get_shader, &set_shader)
}
DEFINE_RESOURCE_TEMPLATES(j::rendering::vulkan::shaders::shader_module, j::files::path);
DEFINE_WEAK_RESOURCE_TEMPLATES(j::rendering::vulkan::shaders::shader_module, j::files::path);
