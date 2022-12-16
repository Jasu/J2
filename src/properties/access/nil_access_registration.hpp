#pragma once

#include "properties/access/nil_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "properties/metadata.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  namespace detail {
    class empty_access {};
  }

  template<typename Type>
  class J_TYPE_HIDDEN nil_access_registration {
  public:
    inline static nil_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::NIL> wrapper;
  private:
    typed_access_registration m_registration;
  public:
    template<typename... Metadata>
    J_HIDDEN explicit nil_access_registration(const char * name, Metadata && ... metadata);
  };

  template<typename Type>
  template<typename... Metadata>
  J_HIDDEN nil_access_registration<Type>::nil_access_registration(const char * name, Metadata && ... metadata) {
    if constexpr (sizeof...(Metadata) != 0) {
      convertible_from.maybe_apply([](const auto & ... convs) {
        auto & conversions = definition.conversions();
        (conversions.add_conversion_from(convs.from, convs.conversion), ...);
      }, static_cast<Metadata &&>(metadata)...);
      definition.metadata() = attributes::attribute_map(
        attributes::without(convertible_to, convertible_from),
        static_cast<Metadata &&>(metadata)...
      );
    }
    initialize_typed_access<Type, detail::empty_access>(definition, PROPERTY_TYPE::NIL, &wrapper, name);
    m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
    if constexpr (sizeof...(Metadata) != 0) {
      convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
    }
  }
}
