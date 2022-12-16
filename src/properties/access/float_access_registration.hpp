#pragma once

#include "properties/access/float_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "properties/metadata.hpp"
#include "attributes/foreach.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  template<typename Type, typename Access = void>
  class J_TYPE_HIDDEN float_access_registration {
  public:
    inline static float_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::FLOAT> wrapper;
  private:
    J_HIDDEN static double get_float_value_wrapper(const void * property) noexcept {
      if constexpr (::j::is_void_v<Access>) {
        return *reinterpret_cast<const Type*>(property);
      } else {
        return Access::get_value(*reinterpret_cast<const Type*>(property));
      }
    }

    J_HIDDEN static void set_float_value_wrapper(void * property, double value) noexcept {
      if constexpr (::j::is_void_v<Access>) {
        *reinterpret_cast<Type*>(property) = value;
      } else {
        Access::set_value(*reinterpret_cast<Type*>(property), value);
      }
    }

    J_HIDDEN typed_access_registration m_registration;
  public:
    template<typename... Metadata>
    J_HIDDEN explicit float_access_registration(const char * name, Metadata && ... metadata) {
      constexpr u32_t num_conversions_from = attributes::count<Metadata...>(convertible_from);
      constexpr u32_t num_conversions_to = attributes::count<Metadata...>(convertible_to);
      if constexpr (num_conversions_from > 0) {
        attributes::foreach_member(
          convertible_from,
          definition.conversions(),
          &conversions::conversion_collection::add_conversion_from_,
          static_cast<Metadata &&>(metadata)...);
      }
      if constexpr ((num_conversions_from + num_conversions_to) != sizeof...(Metadata)) {
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from),
          static_cast<Metadata &&>(metadata)...
        );
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::FLOAT, &wrapper, name);
      definition.initialize_float_access(&get_float_value_wrapper, &set_float_value_wrapper);
      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (num_conversions_to > 0) {
        attributes::foreach(convertible_to, m_registration, static_cast<Metadata &&>(metadata)...);
      }
    }
  };
}
