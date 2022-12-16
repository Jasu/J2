#pragma once

#include "attributes/attribute_definition.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"

namespace j::rendering::vulkan::attributes {
  namespace a = j::attributes;

  /// Size of a buffer, in bytes.
  inline constexpr a::attribute_definition size{
    a::value_type = type<u32_t>,
    a::tag = type<struct size_tag>
  };

  /// Size of a buffer, in bytes.
  inline constexpr a::attribute_definition image_1d{
    a::tag = type<struct image_1d_tag>,
    a::is_flag,
  };

  /// Width of an image or a region, in pixels.
  inline constexpr a::attribute_definition width{
    a::value_type = type<u16_t>,
    a::tag = type<struct width_tag>
  };

  /// Height of an image or a region, in pixels.
  inline constexpr a::attribute_definition height{
    a::value_type = type<u16_t>,
    a::tag = type<struct height_tag>
  };

  /// Format of an image.
  inline constexpr a::attribute_definition format{
    a::tag = type<struct format_tag>
  };

  /// Size of a buffer, in items.
  inline constexpr a::attribute_definition count{
    a::value_type = type<u32_t>,
    a::tag = type<struct count_tag>
  };

  /// Offset of something in a buffer, in bytes.
  inline constexpr a::attribute_definition offset{
    a::value_type = type<u32_t>,
    a::tag = type<struct offset_tag>
  };

  /// Human-readable debug name for various resources.
  inline constexpr a::attribute_definition name{
    a::value_type = type<const char *>,
    a::tag = type<struct name_tag>
  };

  /// Pipeline layout or uniform buffer layout.
  inline constexpr a::attribute_definition layout{
    a::tag = type<descriptors::descriptor_set_layout_ref>,
    a::is_multiple
  };

  /// For specifying either descriptor bindings or vertex/instance attribute bindings.
  inline constexpr a::attribute_definition descriptor{
    a::tag = type<struct descriptor_tag>,
    a::key_type = type<u32_t>,
    a::is_multiple
  };

  /// Specifies uniform value index in a uniform buffer, or vertex attribute binding index:
  inline constexpr a::attribute_definition index{
    a::tag = type<struct index_tag>,
    a::value_type = type<u32_t>};
}
