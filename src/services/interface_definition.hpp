#pragma once

#include "services/registry/registry.hpp"
#include "services/metadata.hpp"
#include "services/service_reference.hpp"

#include <typeinfo>

namespace j::services {
  template<class Interface>
  class J_HIDDEN interface_definition final {
    const class_metadata m_metadata;
    service_reference m_default_implementation;
  public:
    template<typename... Args>
    explicit interface_definition(const char * J_NOT_NULL name, const char * J_NOT_NULL title, Args && ... args)
      : m_metadata(name, title, static_cast<Args &&>(args)...)
    {
      constexpr bool has_default_implementation = j::attributes::has<Args...>(default_implementation);

      if constexpr (has_default_implementation) {
        m_default_implementation = default_implementation.get(static_cast<Args &&>(args)...);
      }

      registry::instance->register_interface(
        type_id::type_id(typeid(Interface)),
        &m_metadata,
        has_default_implementation ? &m_default_implementation : nullptr,
        j::attributes::has<Args...>(one_per_service)
      );
    }

    ~interface_definition() {
      // This usually happens in static destruction phase, and the registry might already be
      // destroyed. Avoid re-creating it.
      if (!registry::instance.is_initialized) {
        return;
      }
      registry::instance->unregister_interface(type_id::type_id(typeid(Interface)));
    }
  };
}
