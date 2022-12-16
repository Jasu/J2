#include "rendering/vulkan/shaders/shader_manager.hpp"
#include "files/paths/path_map.hpp"

J_DEFINE_EXTERN_PATH_MAP(j::rendering::vulkan::shaders::weak_shader_module_ref);

namespace j::rendering::vulkan::shaders {
  weak_shader_module_ref * shader_manager::try_get_shader(const files::path & path) noexcept {
    return m_shaders.maybe_at(path);
  }

  void shader_manager::set_shader(const files::path & path, shader_module_ref shader) {
    m_shaders.emplace(path, shader);
  }

  shader_manager::~shader_manager() {
  }
}
