#pragma once

#include "properties/access/typed_access.hpp"
#include "properties/access/nil_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/set_access.hpp"
#include "properties/access/enum_access.hpp"
#include "properties/classes/object_access.hpp"

#include "properties/wrappers/reference_wrapper.hpp"
#include "properties/wrappers/variant_wrapper.hpp"

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

  template<typename T>
  J_ALWAYS_INLINE J_HIDDEN decltype(auto) access_property(T && t) {
    return wrappers::access(static_cast<T &&>(t));
  }
}
