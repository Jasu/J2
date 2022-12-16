#pragma once

#include "attributes/attribute_definition.hpp"
#include "attributes/attribute_map.hpp"
#include "properties/conversions/conversion_definition.hpp"
#include "strings/formatters/enum_value.hpp"

namespace j::properties {
  J_NO_DEBUG inline constexpr attributes::attribute_definition key_metadata{
    attributes::value_type = type<attributes::attribute_map>,
    attributes::tag = type<struct key_metadata_tag>};

  J_NO_DEBUG inline constexpr attributes::attribute_definition value_metadata{
    attributes::value_type = type<attributes::attribute_map>,
    attributes::tag = type<struct value_metadata_tag>};

  J_NO_DEBUG inline constexpr attributes::attribute_definition convertible_from{
    attributes::tag = type<struct convertible_from_tag>,
    attributes::value_type = type<conversions::conversion_definition>,
    attributes::is_multiple};

  J_NO_DEBUG inline constexpr attributes::attribute_definition convertible_to{
    attributes::tag = type<struct convertible_to_tag>,
    attributes::value_type = type<conversions::conversion_definition>,
    attributes::is_multiple};

  J_NO_DEBUG inline constexpr attributes::attribute_definition property{
    attributes::tag = type<struct property_tag>,
    attributes::is_multiple};

  /// Enum value to add.
  J_NO_DEBUG inline constexpr attributes::attribute_definition value{
    attributes::tag = type<strings::formatters::enum_value>,
    attributes::value_type = type<strings::formatters::enum_value>,
    attributes::is_multiple};

  using metadata J_NO_DEBUG_TYPE = attributes::attribute_map;
}
