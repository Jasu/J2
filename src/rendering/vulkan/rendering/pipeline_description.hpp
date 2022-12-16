#pragma once

#include "rendering/vulkan/attributes/pipeline_attributes.hpp"
#include "rendering/vulkan/attributes/descriptor_set_layout_record.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::attributes::descriptor_set_layout_record);
J_DECLARE_EXTERN_TRIVIAL_ARRAY(VkPushConstantRange);

namespace j::rendering::vulkan::rendering {
  namespace a = vulkan::attributes;
  namespace ja = j::attributes;

  /// Used to create a graphics pipeline.
  ///
  /// A [pipeline_description] can be obtained from [renderer].
  struct pipeline_description final {
    J_BOILERPLATE(pipeline_description, CTOR_CE, MOVE_NE)

    a::shader_configuration vertex_shader;
    a::shader_configuration fragment_shader;
    a::vertex_input_state vertex_input_state;
    trivial_array<a::descriptor_set_layout_record> descriptor_set_layouts;
    trivial_array<VkPushConstantRange> push_constants;
    VkPrimitiveTopology primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    const char * name = nullptr;

    template<typename... Args>
    J_ALWAYS_INLINE explicit pipeline_description(Args && ... args)
      : vertex_shader(a::vertex_shader.get(static_cast<Args &&>(args)...)),
        fragment_shader(a::fragment_shader.get(static_cast<Args &&>(args)...)),
        vertex_input_state(a::vertex_input.get(static_cast<Args &&>(args)...)),
        descriptor_set_layouts(ja::as_trivial_array_move<a::descriptor_set_layout_record>(
                                 a::descriptor_set_layout, static_cast<Args &&>(args)...)),
        push_constants(ja::as_trivial_array_move<VkPushConstantRange>(
                         a::push_constant, static_cast<Args &&>(args)...))
    {
      if constexpr (ja::has<Args...>(a::primitive_topology)) {
        primitive_topology = a::primitive_topology.get(static_cast<Args &&>(args)...);
      }
      if constexpr (ja::has<Args...>(a::name)) {
        name = attributes::name.get(static_cast<Args &&>(args)...);
      }
    }
  };
}
