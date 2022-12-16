#pragma once

#include "properties/property_defs.hpp"

namespace j::properties {
  namespace detail {
    [[noreturn]] void throw_empty_visit();
    [[noreturn]] void throw_object_visit();
    [[noreturn]] void throw_enum_visit();
    [[noreturn]] void throw_set_visit();

    struct visit_helper {
      template<typename T, typename Property, typename Visitor, typename... TailArgs>
      decltype(auto) operator()(Property && property, Visitor && visitor, TailArgs && ... tail) {
        return static_cast<Visitor &&>(visitor)(static_cast<Property &&>(property).template as<T>(),
                                                static_cast<TailArgs &&>(tail)...);
      }
    };
  }

  template<typename Visitor, typename... Args>
  decltype(auto) dispatch_variant(PROPERTY_TYPE property_type, Visitor && visitor, Args && ... args) {
    switch (property_type) {
    case PROPERTY_TYPE::EMPTY:
      detail::throw_empty_visit();
    case PROPERTY_TYPE::OBJECT:
      detail::throw_object_visit();
    case PROPERTY_TYPE::ENUM:
      detail::throw_enum_visit();
    case PROPERTY_TYPE::SET:
      // TODO: Add set to variant
      detail::throw_set_visit();
    case PROPERTY_TYPE::NIL:
      return static_cast<Visitor &&>(visitor).template operator()<nil_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::STRING:
      return static_cast<Visitor &&>(visitor).template operator()<string_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::INT:
      return static_cast<Visitor &&>(visitor).template operator()<int_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::BOOL:
      return static_cast<Visitor &&>(visitor).template operator()<bool_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::FLOAT:
      return static_cast<Visitor &&>(visitor).template operator()<float_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::LIST:
      return static_cast<Visitor &&>(visitor).template operator()<list_t>(static_cast<Args &&>(args)...);
    case PROPERTY_TYPE::MAP:
      return static_cast<Visitor &&>(visitor).template operator()<map_t>(static_cast<Args &&>(args)...);
    }
  }

  template<typename Property, typename Visitor, typename... TailArgs>
  decltype(auto) visit_variant(Property && property, Visitor && visitor, TailArgs && ... tail) {
    return dispatch_variant(property.type(),
                            detail::visit_helper{},
                            static_cast<Property &&>(property),
                            static_cast<Visitor &&>(visitor),
                            static_cast<TailArgs &&>(tail)...);
  }
}
