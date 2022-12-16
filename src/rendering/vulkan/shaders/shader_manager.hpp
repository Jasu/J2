#pragma once

#include "files/paths/path_map_fwd.hpp"
#include "rendering/vulkan/shaders/shader_resource.hpp"

J_DECLARE_EXTERN_PATH_MAP(j::rendering::vulkan::shaders::weak_shader_module_ref);

namespace j::rendering::vulkan::shaders {
  /// Handles compiling shaders for a specific device.
  ///
  /// Caches compilation results.
  class shader_manager {
  public:
    /// Loads and compiles shader from [path].
    ///
    /// Caches the shaders (as a weak pointer), if it is already lodaed, returns the previous
    /// instance.
    weak_shader_module_ref * try_get_shader(const files::path & path) noexcept;

    void set_shader(const files::path & path, shader_module_ref shader);

    ~shader_manager();
  private:
    files::path_map<weak_shader_module_ref> m_shaders;
  };
}
