#pragma once

#include "rendering/vulkan/attributes/common_attributes.hpp"
#include "rendering/vertex_data/get_vertex_attribute_format.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "attributes/basic_operations.hpp"

namespace j::rendering::vulkan::attributes {
  struct vertex_binding_record {
    vertex_data::vertex_binding_info attributes;
    bool is_instance = false;
    i8_t binding = -1;

    constexpr vertex_binding_record() noexcept = default;

    vertex_binding_record(vertex_data::vertex_binding_info info, i8_t binding = -1) noexcept
      : attributes(info), binding(binding) { }

    vertex_binding_record(const vertex_binding_record & lhs, bool is_instance) noexcept
      : attributes(lhs.attributes), is_instance(is_instance), binding(lhs.binding) { }
  };

  namespace a = j::attributes;

  inline constexpr a::attribute_definition vertex_attribute_binding{
    a::tag = type<struct vertex_attribute_tag>,
    a::value_type = type<vertex_binding_record>,
    a::is_multiple};

  inline constexpr a::attribute_definition instance_attribute_binding{
    a::tag = type<struct instance_attribute_tag>,
    a::value_type = type<vertex_binding_record>,
    a::is_multiple};

  inline constexpr a::attribute_definition stride{
    a::tag = type<struct stride_tag>,
    a::value_type = type<u32_t>};

  inline constexpr a::attribute_definition num_vertices{
    a::tag = type<struct num_vertices_tag>,
    a::value_type = type<u32_t>};

  inline constexpr a::attribute_definition attribute{
    a::tag = type<struct attribute_tag>,
    a::key_type = type<attribute_key>,
    a::value_type = type<vertex_data::vertex_attribute_binding>,
    a::is_multiple
  };

  inline constexpr a::attribute_definition scaling{
    a::tag = type<vertex_data::scaling>,
    a::value_type = type<vertex_data::scaling>,
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::attributes::vertex_binding_record);

namespace j::rendering::vulkan::attributes {
  template<typename Struct, typename... Args>
  J_ALWAYS_INLINE_NO_DEBUG vertex_data::vertex_binding_info binding(Args && ... args) {
    u32_t stride_val = 0U;
    if constexpr (j::attributes::has<Args...>(attributes::stride)) {
      stride_val = attributes::stride.get(static_cast<Args &&>(args)...);
    }
    return {
      attributes::attribute.apply([stride_val](const auto & ... bindings) {
        return vertex_data::vertex_binding_info({ bindings... }, stride_val);
      }, static_cast<Args &&>(args)...),
    };
  }

  struct vertex_input_state final {
    J_BOILERPLATE(vertex_input_state, CTOR_CE, MOVE_NE, COPY_DEL)

    template<typename... Args>
    J_ALWAYS_INLINE explicit vertex_input_state(Args && ... args)
      : bindings(containers::uninitialized, sizeof...(args))
    {
      (
        bindings.initialize_element(args.m_value,
                                    a::is<Args>(attributes::instance_attribute_binding)),
        ...
      );
    }

    trivial_array<attributes::vertex_binding_record> bindings;
  };
}
