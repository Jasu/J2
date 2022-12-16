#pragma once

#include "attributes/attribute_definition.hpp"
#include "fonts/selection/flags.hpp"
#include "fonts/font_size.hpp"
#include "strings/string_view.hpp"

namespace j::fonts::selection {
  using namespace attributes;

  inline constexpr attribute_definition family(
    value_type = type<strings::const_string_view>,
    tag = type<struct family_tag>,
    is_multiple);

  inline constexpr attribute_definition weight(
    value_type = type<font_weight_flags>,
    tag = type<font_weight_flags>);

  inline constexpr attribute_definition slant(
    value_type = type<font_slant_flags>,
    tag = type<font_slant_flags>);

  inline constexpr attribute_definition width(
    value_type = type<font_width_flags>,
    tag = type<font_width_flags>);

  inline constexpr attribute_definition spacing(
    value_type = type<font_spacing_flags>,
    tag = type<font_spacing_flags>);

  inline constexpr attribute_definition size(
    value_type = type<font_size>,
    tag = type<font_size>);
}
