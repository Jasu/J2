#pragma once

#include "properties/access/typed_access.hpp"

#include "properties/metadata.hpp"
#include "properties/wrappers/wrapper_definition.hpp"
#include "properties/wrappers/wrapper_registration.hpp"

namespace j::properties {
  using access::typed_access;

  using access::nil_access;
  using access::string_access;
  using access::int_access;
  using access::bool_access;
  using access::float_access;
  using access::list_access;
  using access::map_access;
  using access::set_access;
  using access::enum_access;
  using classes::object_access;

  using access::typed_access_definition;

  using access::nil_access_definition;
  using access::string_access_definition;
  using access::int_access_definition;
  using access::bool_access_definition;
  using access::float_access_definition;
  using access::list_access_definition;
  using access::map_access_definition;
  using access::set_access_definition;
  using access::enum_access_definition;
  using classes::object_access_definition;

  using wrappers::wrapper;
  using wrappers::wrapper_definition;
  using wrappers::wrapper_registration;
}
