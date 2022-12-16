#pragma once

#include "containers/hash_map_fwd.hpp"
#include "type_id/type_hash.hpp"

namespace j::services {
  namespace registry {
    class registry_data;
  }
  namespace configuration {
    class configuration_pass;
  }
}

namespace j::services::detail::container {
  class interface_state;
}

J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::interface_state*, HASH(j::type_id::hash));

namespace j::services::detail::container {
  /// Container for all interface states.
  class interface_container {
  public:
    interface_container() noexcept;

    /// Initialize the interface container with all interface states from the registry.
    void preinitialize(const registry::registry_data & registry_data);

    /// Initialize the interfaces to use their default configuration.
    void initialize();

    /// Destroy the interfaces.
    void finalize();

    interface_state & get_interface(type_id::type_id interface);

    const interface_state & get_interface(type_id::type_id interface) const;

    interface_state * maybe_get_interface(type_id::type_id interface);

    const interface_state * maybe_get_interface(type_id::type_id interface) const;

    bool has_interface(type_id::type_id interface) const;

    void configure(configuration::configuration_pass & pass);

    ~interface_container();

    interface_container(interface_container &&) = delete;
    interface_container(const interface_container &) = delete;
    interface_container & operator=(interface_container &&) = delete;
    interface_container & operator=(const interface_container &) = delete;

  private:
    hash_map<type_id::type_id, interface_state*, j::type_id::hash> m_interfaces;
  };
}
