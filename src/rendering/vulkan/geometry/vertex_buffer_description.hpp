#pragma once

#include "rendering/vertex_data/vertex_binding_info.hpp"
#include "rendering/vertex_data/vertex_buffer_info.hpp"
#include "rendering/vulkan/attributes/vertex_input_state_attributes.hpp"
#include "rendering/vertex_data/get_vertex_attribute_format.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/enable_if_attributes.hpp"

namespace j::rendering::vulkan::geometry {

  struct vertex_buffer_description final {
    vertex_data::vertex_binding_info binding_info;
    u32_t num_vertices = 0U;

    explicit vertex_buffer_description(const vertex_data::vertex_buffer_info & info) noexcept
      : binding_info(info.binding_info()),
        num_vertices(info.num_vertices())
    {
    }

    template<typename... Args, typename = ::j::attributes::enable_if_attributes_t<Args...>>
    J_ALWAYS_INLINE vertex_buffer_description(Args && ... args) {
      u16_t stride = 0U;
      if constexpr (j::attributes::has<Args...>(attributes::stride)) {
        stride = attributes::stride.get(static_cast<Args &&>(args)...);
      }
      binding_info = attributes::attribute.apply([stride](auto... ptrs) {
        return vertex_data::vertex_binding_info({ptrs...}, stride);
      }, static_cast<Args &&>(args)...);
      num_vertices = attributes::num_vertices.get(static_cast<Args &&>(args)...);
    }
  };
}
