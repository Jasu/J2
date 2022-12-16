#pragma once

#include "attributes/attribute_definition.hpp"
#include "services/service_reference.hpp"

namespace j::strings::attributes {
  namespace a = j::attributes;
  inline constexpr a::attribute_definition sink{
    a::value_type = type<services::service_reference>,
    a::tag = type<struct sink_tag>};

}
