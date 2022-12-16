#include "properties/wrappers/wrapper.hpp"

#include "properties/access/typed_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/access/nil_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/set_access.hpp"
#include "properties/access/enum_access.hpp"
#include "properties/classes/object_access.hpp"

namespace j::properties::wrappers {
  access::typed_access wrapper::get_value() {
    return definition().m_get_value(as_void_star());
  }

  access::string_access wrapper::as_string() {
    return get_value().as_string();
  }

  access::int_access wrapper::as_int() {
    return get_value().as_int();
  }

  access::float_access wrapper::as_float() {
    return get_value().as_float();
  }

  access::bool_access wrapper::as_bool() {
    return get_value().as_bool();
  }

  access::nil_access wrapper::as_nil() {
    return get_value().as_nil();
  }

  access::list_access wrapper::as_list() {
    return get_value().as_list();
  }

  access::map_access wrapper::as_map() {
    return get_value().as_map();
  }

  access::set_access wrapper::as_set() {
    return get_value().as_set();
  }

  access::enum_access wrapper::as_enum() {
    return get_value().as_enum();
  }

  classes::object_access wrapper::as_object() {
    return get_value().as_object();
  }
}
