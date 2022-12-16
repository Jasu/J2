#pragma once

#include <vulkan/vulkan.h>
#include "rendering/vulkan/attributes/common_attributes.hpp"
#include "rendering/vulkan/attributes/vertex_input_state_attributes.hpp"
#include "rendering/vulkan/shaders/shader_resource.hpp"
#include "rendering/data_types/typed_value.hpp"
#include "rendering/attribute_key.hpp"
#include "attributes/trivial_array.hpp"
#include "files/paths/path.hpp"


namespace j::rendering::vulkan::attributes {
  namespace a = ::j::attributes;

  inline constexpr a::attribute_definition vertex_input{
    a::tag = type<vertex_input_state>,
    a::value_type = type<vertex_input_state>};

  inline constexpr a::attribute_definition primitive_topology{
    a::tag = type<VkPrimitiveTopology>,
    a::value_type = type<VkPrimitiveTopology>};

  namespace detail {
    struct push_constant_value final {
      u8_t offset:8;
      u8_t size:8;
      VkShaderStageFlags stages:16;

      explicit constexpr push_constant_value(u8_t size, VkShaderStageFlags stages) noexcept
        : offset(0),
          size(size),
          stages(stages)
      {
      }

      constexpr push_constant_value(u8_t offset, const push_constant_value & constant) noexcept
        : offset(offset),
          size(constant.size),
          stages(constant.stages)
      {
      }

      constexpr operator VkPushConstantRange() const noexcept {
        return VkPushConstantRange{
          .stageFlags = stages,
          .offset = offset,
          .size = size,
        };
      }
    };
  }

  template<typename T>
  J_INLINE_GETTER constexpr detail::push_constant_value vertex_shader_push_constant() noexcept {
    return detail::push_constant_value(align_up(sizeof(T), 4U), VK_SHADER_STAGE_VERTEX_BIT);
  }

  template<typename T>
  J_INLINE_GETTER constexpr detail::push_constant_value shared_push_constant() noexcept {
    return detail::push_constant_value(align_up(sizeof(T), 4U), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  template<typename T>
  J_INLINE_GETTER constexpr detail::push_constant_value fragment_shader_push_constant() noexcept {
    return detail::push_constant_value(align_up(sizeof(T), 4U), VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  inline constexpr a::attribute_definition push_constant{
    a::tag = type<detail::push_constant_value>,
    a::value_type = type<detail::push_constant_value>,
    a::key_type = type<u8_t>,
    a::is_multiple};

  inline constexpr a::attribute_definition path{
    a::tag = type<struct path_tag>,
    a::value_type = type<files::path>};

  struct spec_constant_value final {
    constexpr spec_constant_value() noexcept = default;

    template<typename T>
    spec_constant_value(attribute_key && k, const T & value)
      : key(static_cast<attribute_key &&>(k)),
        value(value)
    { }

    attribute_key key;
    data_types::typed_value value;
  };

  inline constexpr a::attribute_definition spec_constant{
    a::tag = type<spec_constant_value>,
    a::value_type = type<spec_constant_value>,
    a::key_type = type<attribute_key>,
    a::is_multiple};
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::attributes::spec_constant_value);

namespace j::rendering::vulkan::attributes {
  inline constexpr a::attribute_definition shader{
    a::tag = type<shaders::shader_module_ref>,
    a::value_type = type<shaders::shader_module_ref>};

  struct shader_configuration final {
    J_BOILERPLATE(shader_configuration, CTOR_CE, MOVE_NE, COPY_DEL)

    J_ALWAYS_INLINE shader_configuration(const char * J_NOT_NULL str) noexcept
      : shader(str)
    { }

    J_ALWAYS_INLINE shader_configuration(files::path && p) noexcept
      : shader(static_cast<files::path &&>(p))
    { }

    J_ALWAYS_INLINE shader_configuration(const files::path & p) : shader(p)
    { }

    J_ALWAYS_INLINE shader_configuration(shaders::shader_module_ref && s) noexcept
      : shader(static_cast<shaders::shader_module_ref &&>(s))
    { }

    J_ALWAYS_INLINE shader_configuration(const shaders::shader_module_ref & s) noexcept
      : shader(s)
    { }

    template<typename... Attrs>
    J_ALWAYS_INLINE shader_configuration(Attrs && ... attrs)
      : shader(attributes::shader.get(static_cast<Attrs &&>(attrs)...)),
        specialization_constants(a::as_trivial_array_move<spec_constant_value>(spec_constant, static_cast<Attrs &&>(attrs)...))
    { }

    shaders::shader_module_ref shader;
    trivial_array<spec_constant_value> specialization_constants;
  };


  inline constexpr a::attribute_definition vertex_shader{
    a::tag = type<struct vertex_shader_tag>,
    a::value_type = type<shader_configuration>};

  inline constexpr a::attribute_definition fragment_shader{
    a::tag = type<struct fragment_shader_tag>,
    a::value_type = type<shader_configuration>};
}
