#pragma once

#include <vulkan/vulkan.h>

#include "containers/span.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/shaders/shader_resource.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(VkPushConstantRange);

namespace j::rendering::vulkan::rendering {
  struct pipeline_description;

  /// Pipeline configuration.
  ///
  /// The pipeline is created with dynamic viewport and scissor, so that it would not have to be
  /// recreated when the window resizes.
  class pipeline final {
  public:
    pipeline(const surface_context & context,
             pipeline_description && description);

    const VkPushConstantRange & get_push_constant_range(u8_t offset) const;

    span<const descriptors::descriptor_set_layout_ref> descriptor_set_layouts() const noexcept;

    pipeline(pipeline &&) = delete;
    pipeline & operator=(pipeline &&) = delete;

    ~pipeline();

    void debug_dump_pipeline(const files::path & path);

    VkPipeline vk_pipeline;
    VkPipelineLayout vk_pipeline_layout;
    trivial_array<VkPushConstantRange> push_constants;
    j::rendering::vulkan::descriptors::descriptor_set_layout_ref m_descriptor_set_layouts[4];
    const char * name;
    shaders::shader_module_ref vertex_shader;
    shaders::shader_module_ref fragment_shader;
    weak_device_ref device;
  };
}
