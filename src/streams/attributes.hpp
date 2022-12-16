#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "attributes/attribute_definition.hpp"

namespace j::files::inline fds {
  struct fd;
}

namespace j::streams::attributes {
  namespace a = j::attributes;
  inline constexpr a::attribute_definition fd{
    a::value_type = type<files::fd>,
    a::tag = type<struct fd_tag>};

}
