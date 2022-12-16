#pragma once

#include "rendering/vulkan/spirv/type_info.hpp"
#include "rendering/vulkan/spirv/variable_info.hpp"

namespace j::rendering::vertex_data {
  class vertex_input_info;
}

namespace j::rendering::vulkan::inline spirv {
  struct specialization_constant_set;

  class shader_metadata final {
  public:
    static shader_metadata introspect(const u8_t * spirv, u32_t size);

    vertex_data::vertex_input_info get_vertex_input_info() const;

    specialization_constant_set get_specialization_constant_set() const;

    J_ALWAYS_INLINE shader_metadata() noexcept = default;

    u8_t vulkan_version_major = 0U;
    u8_t vulkan_version_minor = 0U;
    type_table types;
    variable_table vars;

  private:
    J_ALWAYS_INLINE shader_metadata(u8_t vulkan_version_major,
                                    u8_t vulkan_version_minor) noexcept
      : vulkan_version_major(vulkan_version_major),
        vulkan_version_minor(vulkan_version_minor)
      {
      }
  };
}
