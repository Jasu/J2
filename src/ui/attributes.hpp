#pragma once

#include "ui/layout/flags.hpp"
#include "attributes/attribute_definition.hpp"
#include "geometry/perimeter.hpp"
#include "geometry/vec2.hpp"
#include "geometry/side.hpp"
#include "colors/rgb.hpp"

namespace j::ui::attributes {
  namespace a = j::attributes;
  namespace g = j::geometry;

  inline constexpr a::attribute_definition collapse_margins{
    a::value_type = type<g::sides>,
    a::tag = type<struct collapse_margin_tag>,
  };

  inline constexpr a::attribute_definition border_size{
    a::value_type = type<g::perimeter_u8>,
    a::tag = type<struct border_type_tag>,
  };
  inline constexpr a::attribute_definition corner_radius{
    a::value_type = type<g::perimeter_u8>,
    a::tag = type<struct corner_radius_tag>,
  };

  inline constexpr a::attribute_definition border_color{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct border_color_tag>,
  };

  inline constexpr a::attribute_definition border_color_top{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct border_color_top_tag>,
  };
  inline constexpr a::attribute_definition border_color_bottom{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct border_color_bottom_tag>,
  };
  inline constexpr a::attribute_definition border_color_left{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct border_color_left_tag>,
  };
  inline constexpr a::attribute_definition border_color_right{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct border_color_right_tag>,
  };

  inline constexpr a::attribute_definition background_color{
    a::value_type = type<colors::rgba8>,
    a::tag = type<struct background_color_tag>,
  };

  inline constexpr a::attribute_definition padding{
    a::value_type = type<g::perimeter_u8>,
    a::tag = type<struct padding_tag>,
  };
  inline constexpr a::attribute_definition margin{
    a::value_type = type<g::perimeter_u8>,
    a::tag = type<struct margin_tag>,
  };
  inline constexpr a::attribute_definition min_size{
    a::value_type = type<g::vec2u16>,
    a::tag = type<struct min_size_tag>,
  };
  inline constexpr a::attribute_definition max_size{
    a::value_type = type<g::vec2u16>,
    a::tag = type<struct max_size_tag>,
  };
  inline constexpr a::attribute_definition orientation{
    a::value_type = type<layout::orientation>,
    a::tag = type<layout::orientation>,
  };
  inline constexpr a::attribute_definition vertical_gravity{
    a::value_type = type<gravity>,
    a::tag = type<struct vertical_gravity_tag>,
  };

  inline constexpr a::attribute_definition horizontal_gravity{
    a::value_type = type<gravity>,
    a::tag = type<struct horizontal_gravity_tag>,
  };
}
