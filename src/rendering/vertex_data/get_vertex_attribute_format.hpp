#pragma once

#include "rendering/vertex_data/vertex_binding_info.hpp"
#include "rendering/data_types/get_data_type.hpp"

namespace j::rendering::vertex_data {
  J_INLINE_GETTER constexpr vertex_attribute_target get_vertex_attribute(const vertex_attribute_target & a) noexcept {
    return a;
  }

  J_INLINE_GETTER constexpr vertex_attribute_target get_vertex_attribute(const vertex_attribute_target & a, scaling s) noexcept {
    return vertex_attribute_target(a.offset, a.type, s);
  }

  template<typename T, typename U>
  J_INLINE_GETTER constexpr vertex_attribute_target get_vertex_attribute(T U::*ptr, scaling s = scaling::normalized) noexcept {
    return vertex_attribute_target{
      (u16_t)__builtin_bit_cast(ssz_t, &(((U*)nullptr)->*ptr)),
      data_types::data_type_v<T>,
      s
    };
  }

  template<typename T>
  J_INLINE_GETTER constexpr vertex_attribute_target scaled(T attr) noexcept {
    return get_vertex_attribute(attr, scaling::scaled);
  }

  template<typename T>
  J_INLINE_GETTER constexpr vertex_attribute_target integer(T attr) noexcept {
    return get_vertex_attribute(attr, scaling::integer);
  }

  template<typename T>
  J_INLINE_GETTER constexpr vertex_attribute_target normalized(T attr) noexcept {
    return get_vertex_attribute(attr, scaling::normalized);
  }
}
