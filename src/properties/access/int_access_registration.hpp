#pragma once

#include "properties/access/int_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "properties/metadata.hpp"
#include "attributes/foreach.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  template<typename Type, typename Access = void>
  class J_TYPE_HIDDEN int_access_registration {
  public:
    static inline int_access_definition definition;
    static inline wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::INT> wrapper;
  private:
    J_HIDDEN static i64_t get_int_value_wrapper(const void * property) noexcept {
      if constexpr (is_void_v<Access>) {
        return *reinterpret_cast<const Type*>(property);
      } else {
        return Access::get_value(*reinterpret_cast<const Type*>(property));
      }
    }

    J_HIDDEN static void set_int_value_wrapper(void * property, i64_t value) noexcept {
      if constexpr (is_void_v<Access>) {
        *reinterpret_cast<Type*>(property) = value;
      } else {
        Access::set_value(*reinterpret_cast<Type*>(property), value);
      }
    }

    J_HIDDEN typed_access_registration m_registration;

  public:
    template<typename... Metadata>
    J_HIDDEN explicit int_access_registration(const char * name, Metadata && ... metadata) {
      constexpr sz_t num_conversions = attributes::count<Metadata...>(convertible_from, convertible_to);
      if constexpr (num_conversions > 0) {
        attributes::foreach_member(
          convertible_from,
          definition.conversions(),
          &conversions::conversion_collection::add_conversion_from_,
          static_cast<Metadata &&>(metadata)...);
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::INT, &wrapper, name);
      if constexpr (num_conversions != sizeof...(Metadata)) {
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from),
          static_cast<Metadata &&>(metadata)...
        );
      }
      definition.initialize_int_access(&get_int_value_wrapper, &set_int_value_wrapper);
      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (num_conversions > 0) {
        attributes::foreach(convertible_to, m_registration, static_cast<Metadata &&>(metadata)...);
      }
    }
  };
}
