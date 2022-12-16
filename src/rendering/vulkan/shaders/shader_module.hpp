#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/spirv/shader_metadata.hpp"

namespace j::files::inline paths { class path; }

namespace j::rendering::vulkan::shaders {
  struct shader_module final {
    shader_module(const device_context_base & context, const files::path & path);

    ~shader_module();

    vertex_data::vertex_input_info get_vertex_input_info() const;

    specialization_constant_set specialization_constants() const;

    shader_module(const shader_module &) = delete;
    shader_module & operator=(const shader_module &) = delete;

    VkShaderModule vk_shader_module;
    weak_device_ref device;
    shader_metadata metadata;
  };
}
