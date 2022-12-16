#pragma once

#include "strings/tables/border_style.hpp"
#include "attributes/attribute_definition.hpp"
#include "strings/formatting/pad.hpp"

namespace j::strings::inline tables {
  namespace a = j::attributes;

  inline constexpr a::attribute_definition border_left{
    a::value_type = type<border_style>,
    a::tag = type<struct border_left_tag>,
  };

  inline constexpr a::attribute_definition border_right{
    a::value_type = type<border_style>,
    a::tag = type<struct border_right_tag>,
  };

  inline constexpr a::attribute_definition border_top{
    a::value_type = type<border_style>,
    a::tag = type<struct border_top_tag>,
  };

  inline constexpr a::attribute_definition border_bottom{
    a::value_type = type<border_style>,
    a::tag = type<struct border_bottom_tag>,
  };

  inline constexpr a::attribute_definition border_horizontal{
    a::value_type = type<border_style>,
    a::tag = type<struct border_horizontal_tag>,
  };

  inline constexpr a::attribute_definition border_vertical{
    a::value_type = type<border_style>,
    a::tag = type<struct border_vertical_tag>,
  };

  inline constexpr a::attribute_definition border{
    a::value_type = type<border_style>,
    a::tag = type<border_style>,
  };

  inline constexpr a::attribute_definition pad{
    a::value_type = type<pad_settings>,
    a::tag = type<struct pad_horizontal_tag>,
  };

  inline constexpr a::attribute_definition pad_top{
    a::value_type = type<u16_t>,
    a::tag = type<struct pad_top_tag>,
  };

  inline constexpr a::attribute_definition pad_bottom{
    a::value_type = type<u16_t>,
    a::tag = type<struct pad_bottom_tag>,
  };

  inline constexpr a::attribute_definition pad_height_to{
    a::value_type = type<u16_t>,
    a::tag = type<struct pad_height_tag>,
  };
}
