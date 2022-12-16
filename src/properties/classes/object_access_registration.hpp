#pragma once

#include "properties/classes/object_access.hpp"
#include "properties/classes/object_property_registration.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "properties/metadata.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "exceptions/assert_lite.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::classes {
  namespace detail {
    class empty_access {
    };
  }

  template<typename Class, typename Access = detail::empty_access>
  class J_TYPE_HIDDEN object_access_registration {
  public:
    inline static object_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Class, &definition, PROPERTY_TYPE::OBJECT> wrapper;
  private:
    J_HIDDEN access::typed_access_registration m_registration;

    J_HIDDEN static void property_wrapper_available(
      const wrappers::wrapper_definition * def,
      void * user_data
    ) {
      definition.on_property_wrapper_available(
        reinterpret_cast<object_property_definition*>(user_data),
        def);
    }
  public:
    template<typename... Args>
    J_HIDDEN explicit object_access_registration(const char * name, Args && ... args) {
      constexpr sz_t num_properties = ::j::attributes::count<Args...>(property);
      if constexpr (num_properties != sizeof...(Args)) {
        constexpr sz_t num_conversions = attributes::count<Args...>(convertible_from, convertible_to);
        if constexpr (num_conversions > 0) {
          convertible_from.maybe_apply([](const auto & ... convs) {
            auto & conversions = definition.conversions();
            (conversions.add_conversion_from(convs.from, convs.conversion), ...);
          }, static_cast<Args &&>(args)...);
        }
        if constexpr (num_conversions + num_properties != sizeof...(Args)) {
          definition.metadata() = attributes::attribute_map(
            attributes::without(convertible_to, convertible_from, property),
            static_cast<Args &&>(args)...
          );
        }
      }
      access::initialize_typed_access<Class, Access>(definition, PROPERTY_TYPE::OBJECT, &wrapper, name);

      if constexpr (num_properties != 0) {
        property.apply(
          [](auto && ... vs) {
            (definition.initialize_definition(vs.template to_definition<Class>()), ...);
          }, static_cast<Args &&>(args)...);
      }
      definition.initialize_object_access_done();

      m_registration.add_to_registry(typeid(Class), &definition, &wrapper);
      if constexpr (num_properties != sizeof...(Args)) {
        convertible_to.maybe_apply(m_registration, static_cast<Args &&>(args)...);
      }

      for (auto & prop : definition.get_properties()) {
        access::registry::wait_for_wrapper(type_id::type_id(typeid(Class)),
                                           prop.second->type_id(),
                                           &property_wrapper_available,
                                           prop.second);
      }
    }
  };
}
