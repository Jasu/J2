#pragma once

#include "attributes/attribute_definition.hpp"

namespace j::cli {
  namespace a = attributes;

  /// Attribute specifying a short description for a command or argument.
  inline constexpr a::attribute_definition short_description{
    a::value_type = type<const char *>,
    a::tag = type<struct short_description_tag>};

  /// Attribute specifying a long description for a command or argument.
  inline constexpr a::attribute_definition description{
    a::value_type = type<const char *>,
    a::tag = type<struct long_description_tag>};
}
