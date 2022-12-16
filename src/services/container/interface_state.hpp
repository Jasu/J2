#pragma once

#include "mem/shared_ptr_fwd.hpp"
#include "containers/hash_map_fwd.hpp"
#include "containers/vector.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "type_id/type_hash.hpp"
#include "strings/string.hpp"
#include "services/configuration/configuration_union.hpp"
#include "services/container/service_instance_state.hpp"

J_DECLARE_EXTERN_VECTOR(j::services::detail::container::service_instance_state*);

namespace j::services {
  class container;
  struct injection_context;
  struct class_metadata;
  class service_reference;
  namespace configuration {
    class configuration_pass;
  }
  namespace registry {
    struct interface_record;
  }
}
namespace j::services::detail::container {
  class service_type;
}

J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::service_type*, HASH(j::type_id::hash));
J_DECLARE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::services::detail::container::service_instance_state*);

namespace j::services::detail::container {
  /// State of an interface in a container.
  class interface_state {
    /// Static metadata, e.g. name and title of the interface.
    const registry::interface_record * m_record = nullptr;
    /// Interface configuration object.
    configuration::configuration_union m_configuration;
    /// Vector of interface implementations, sorted by priority (highest priority first).
    ///
    /// Pointers point to the implementation in service_type_state.
    vector<service_instance_state *> m_ordered_instances;
    /// Implementing instances, keyed by name.
    unsorted_string_map<service_instance_state*> m_instances_by_name;
    /// Implementing service types, keyed by type.
    hash_map<type_id::type_id, service_type*, type_id::hash> m_implementations;
    /// Pointer to the configured default implementation (if any).
    service_type * m_default_implementation = nullptr;
    /// Pointer to the configured default implementation (if any).
    service_instance_state * m_default_instance = nullptr;
    /// Name of the interface.
    strings::string m_name;

    /// Sort m_ordered_instances based on configuration.
    void sort_instances();

    mem::shared_ptr<void> get_default_configuration() const;
  public:
    interface_state();

    interface_state(const interface_state &) = delete;
    interface_state(interface_state &&) = delete;
    interface_state & operator=(const interface_state &) = delete;
    interface_state & operator=(interface_state &&) = delete;

    ~interface_state();

    /// Construct interface state based on a record from the registry.
    explicit interface_state(const registry::interface_record & record);

    /// Register a service type as implementing the interface.
    void add_implementation(const type_id::type_id & service_type_index, service_type * service_type);

    /// Register a service instance as an interface implementing the interface.
    void add_instance(const strings::string & name, service_instance_state * service_type);

    /// Get ordered instance states.
    ///
    /// Does not instantiate the instances.
    auto & ordered_instance_states() noexcept {
      return m_ordered_instances;
    }

    /// Get implementations instances, ordered by priority.
    ///
    /// Instantiates all implementations.
    vector<mem::shared_ptr<void>> ordered_implementations(services::container * c, const injection_context * parent_ic);

    /// Checks if the interface contains a service by ref.
    bool has(const service_reference & ref) const noexcept;

    /// Get a service by ref, throwing if it does not exist.
    mem::shared_ptr<void> get(services::container * c, const injection_context * parent_ic, const service_reference & ref);

    void configure(configuration::configuration_pass & pass);

    void update_configuration();

    const class_metadata * metadata() const;
  };
}
