#pragma once

#include "properties/access/bool_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "properties/metadata.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  namespace detail {
    template<typename T>
    struct J_TYPE_HIDDEN default_bool_access {
      static bool get_value(const T & value) noexcept {
        return value;
      }

      static void set_value(T & target, bool source) noexcept {
        target = source;
      }
    };
  }

  template<typename Type, typename Access = detail::default_bool_access<Type>>
  class J_TYPE_HIDDEN bool_access_registration {
  public:
    inline static bool_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::BOOL> wrapper;
  private:
    static bool get_bool_value_wrapper(const void * property) noexcept {
      return Access::get_value(*reinterpret_cast<const Type*>(property));
    }

    static void set_bool_value_wrapper(void * property, bool value) noexcept {
      Access::set_value(*reinterpret_cast<Type*>(property), value);
    }

    typed_access_registration m_registration;

  public:
    template<typename... Metadata>
    bool_access_registration(const char * name, Metadata && ... metadata) {
      constexpr u32_t num_conversions = attributes::count<Metadata...>(convertible_from, convertible_to);
      if constexpr (num_conversions > 0) {
        convertible_from.maybe_apply([](const auto & ... convs) {
          auto & conversions = definition.conversions();
          (conversions.add_conversion_from(convs.from, convs.conversion), ...);
        }, static_cast<Metadata &&>(metadata)...);
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::BOOL, &wrapper, name);
      if constexpr (num_conversions != sizeof...(Metadata)) {
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from),
          static_cast<Metadata &&>(metadata)...
        );
      }
      definition.initialize_bool_access(&get_bool_value_wrapper, &set_bool_value_wrapper);
      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (num_conversions > 0) {
        convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
      }
    }
  };
}
