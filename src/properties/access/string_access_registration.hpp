#pragma once

#include "properties/access/string_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "attributes/basic_operations.hpp"
#include "properties/metadata.hpp"

namespace j::properties::access {

  template<typename Type, typename Access>
  class J_TYPE_HIDDEN string_access_registration {
  public:
    inline static string_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::STRING> wrapper;
  private:
    static decltype(auto) as_j_string(const void * property) {
      return Access::as_j_string(*reinterpret_cast<const Type*>(property));
    }

    static const char * as_char_ptr(const void * property) noexcept {
      return Access::as_char_ptr(*reinterpret_cast<const Type*>(property));
    }

    static sz_t size(const void * property) noexcept {
      return Access::get_size(*reinterpret_cast<const Type*>(property));
    }

    static decltype(auto) as_j_string_view(const void * property) noexcept {
      return Access::as_j_string_view(*reinterpret_cast<const Type*>(property));
    }

    J_HIDDEN typed_access_registration m_registration;
  public:
    template<typename... Metadata>
    J_HIDDEN explicit string_access_registration(const char * name, Metadata && ... metadata) {
      if constexpr (sizeof...(Metadata) != 0) {
        constexpr sz_t num_conversions = attributes::count<Metadata...>(convertible_from, convertible_to);
        if constexpr (num_conversions > 0) {
          convertible_from.maybe_apply([](const auto & ... convs) {
            auto & conversions = definition.conversions();
            (conversions.add_conversion_from(convs.from, convs.conversion), ...);
          }, static_cast<Metadata &&>(metadata)...);
        }

        if constexpr (num_conversions != sizeof...(Metadata)) {
          definition.metadata() = attributes::attribute_map(
            attributes::without(convertible_to, convertible_from),
            static_cast<Metadata &&>(metadata)...
          );
        }
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::STRING, &wrapper, name);

      definition.initialize_string_access(&size, &as_j_string, &as_j_string_view, &as_char_ptr);

      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (sizeof...(Metadata) != 0) {
        convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
      }
    }
  };
}
