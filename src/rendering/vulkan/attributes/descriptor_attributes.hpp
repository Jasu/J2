#pragma once

#include "rendering/vulkan/attributes/common_attributes.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_description.hpp"
#include "attributes/basic_operations.hpp"

namespace j::rendering::vulkan::descriptors {
  struct layout_count_rec;
}

namespace j::rendering::vulkan::attributes {
  namespace a = j::attributes;

  /// For specifying a laoyout in a descriptor pool, and the number of copies of
  /// that layout required.
  inline constexpr a::attribute_definition layout_count{
    a::value_type = type<descriptors::layout_count_rec>,
    a::tag = type<descriptors::layout_count_rec>,
    a::key_type = type<descriptors::descriptor_set_layout_ref>,
    a::is_multiple};

  /// For specifying the (extra) number of uniform bindings required in a descriptor pool.
  inline constexpr a::attribute_definition num_uniforms{
    a::value_type = type<u32_t>,
    a::tag = type<struct num_uniforms_tag>};

  /// For specifying the (extra) number of combined image sampler bindings required in a descriptor pool.
  inline constexpr a::attribute_definition num_combined_image_samplers{
    a::value_type = type<u32_t>,
    a::tag = type<struct num_combined_image_samplers_tag>};

  /// Maximum number of descriptor sets allocated from a descriptor pool.
  inline constexpr a::attribute_definition max_sets{
    a::value_type = type<u32_t>,
    a::tag = type<struct max_sets_tag>};

  /// Specifies that the descriptor is only accessible during fragment shader evaluation.
  inline constexpr a::attribute_definition fragment_stage_only{
    a::tag = type<struct fragment_stage_only_tag>,
    a::is_flag};

  /// Specifies that the descriptor is only accessible during vertex shader evaluation.
  inline constexpr a::attribute_definition vertex_stage_only{
    a::tag = type<struct vertex_stage_only_tag>,
    a::is_flag};

  inline constexpr a::attribute_definition is_dynamic{a::tag = type<struct is_dynamic_tag>, a::is_flag};

  template<typename... Args>
  J_INLINE_GETTER_NO_DEBUG descriptors::descriptor_binding_description constexpr uniform(Args && ... attributes) noexcept {
    VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    if constexpr (a::has<Args...>(vertex_stage_only)) {
      stages = VK_SHADER_STAGE_VERTEX_BIT;
    } else if constexpr (a::has<Args...>(fragment_stage_only)) {
      stages = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return descriptors::descriptor_binding_description{
      a::has<Args...>(is_dynamic)
        ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
        : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      stages,
      layout.get_first(static_cast<Args &&>(attributes)...),
    };
  }

  template<typename... Args>
  J_INLINE_GETTER_NO_DEBUG descriptors::descriptor_binding_description constexpr combined_image_sampler(const Args & ...) noexcept {
    VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    if constexpr (a::has<Args...>(vertex_stage_only)) {
      stages = VK_SHADER_STAGE_VERTEX_BIT;
    } else if constexpr (a::has<Args...>(fragment_stage_only)) {
      stages = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return descriptors::descriptor_binding_description{
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      stages,
      {}
    };
  }
}
