#pragma once

#include "properties/wrappers/wrapper_definition.hpp"
#include "exceptions/assert_lite.hpp"
#include "properties/detail/value.hpp"

namespace j::properties::access {
  class bool_access;
  class float_access;
  class int_access;
  class list_access;
  class map_access;
  class set_access;
  class nil_access;
  class string_access;
  class enum_access;
}

namespace j::properties::classes {
  class object_access;
}

namespace j::properties::wrappers {
  class wrapper : public properties::detail::value_base {
  public:
    constexpr wrapper() noexcept = default;

    template<typename Tag, typename Ptr>
    J_HIDDEN J_ALWAYS_INLINE wrapper(const Tag & tag, const wrapper_definition & definition, Ptr * wrapper)
      : properties::detail::value_base(tag, static_cast<const detail::value_definition &>(definition), wrapper)
    {
    }

    wrapper(const wrapper_definition & definition, void * wrapper)
      : properties::detail::value_base(
        detail::wrapper_tag,
        static_cast<const detail::value_definition &>(definition),
        wrapper)
    {
      J_ASSERT_NOT_NULL(wrapper);
    }

    const wrapper_definition & definition() const noexcept {
      return static_cast<const wrapper_definition &>(value_definition());
    }

    PROPERTY_TYPE type() const {
      return definition().m_get_current_type(as_void_star());
    }

    bool is_string() const {
      return type() == PROPERTY_TYPE::STRING;
    }

    bool is_int() const {
      return type() == PROPERTY_TYPE::INT;
    }

    bool is_float() const {
      return type() == PROPERTY_TYPE::FLOAT;
    }

    bool is_bool() const {
      return type() == PROPERTY_TYPE::BOOL;
    }

    bool is_nil() const {
      return type() == PROPERTY_TYPE::NIL;
    }

    bool is_list() const {
      return type() == PROPERTY_TYPE::LIST;
    }

    bool is_map() const {
      return type() == PROPERTY_TYPE::MAP;
    }

    bool is_set() const {
      return type() == PROPERTY_TYPE::SET;
    }

    bool is_enum() const {
      return type() == PROPERTY_TYPE::ENUM;
    }

    bool is_object() const {
      return type() == PROPERTY_TYPE::OBJECT;
    }

    template<typename TypedAccess>
    J_HIDDEN J_ALWAYS_INLINE void set_value(TypedAccess && access) {
      definition().m_set_value(as_void_star(), static_cast<TypedAccess &&>(access));
    }

    access::typed_access get_value();

    access::string_access as_string();

    access::int_access as_int();

    access::float_access as_float();

    access::bool_access as_bool();

    access::enum_access as_enum();

    access::nil_access as_nil();

    access::list_access as_list();

    access::map_access as_map();

    access::set_access as_set();

    classes::object_access as_object();
  };
}
