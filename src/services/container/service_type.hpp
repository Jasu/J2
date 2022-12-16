#pragma once

#include "containers/unsorted_string_map_fwd.hpp"
#include "services/container/service_instance_state.hpp"
#include "services/container/initialization_stage.hpp"
#include "type_id/type_id.hpp"

namespace j::services {
  class container;
  struct injection_context;
  namespace configuration {
    class configuration_pass;
  }
  namespace registry {
    class registry_data;
    struct service_record;
  }
}

J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::services::detail::container::service_instance_state);

namespace j::services::detail::container {
  class interface_container;
  class service_type_container;

  /// Information and state of a single service type.
  class service_type {
    friend struct service_instance_state;
    /// State of the default instance, if any. Points to an instance in the m_instances map.
    service_instance_state * m_default_instance = nullptr;
    /// Map of named instances, keyed by instance name.
    ///
    /// The default instances typically have the name of the service type.
    containers::uncopyable_unsorted_string_map<service_instance_state> m_instances;
    /// Type index of the service type.
    type_id::type_id m_type_id;
    /// Service record of the registration.
    const registry::service_record * m_record = nullptr;
  public:
    service_type(service_type && rhs) = delete;
    service_type(const service_type & rhs) = delete;

    /// Construct a service type based on a record from the registry.
    service_type(
      const type_id::type_id & service_type_id,
      const registry::service_record * record,
      const registry::registry_data & registry_data,
      interface_container & interfaces);

    ~service_type();

    const char * get_name() const noexcept;

    void handle_default_initialization(services::container * c, initialization_stage stage, service_type_container * type_container);

    const type_id::type_id & type_id() const noexcept {
      return m_type_id;
    }

    bool has_default_instance() const noexcept {
      return m_default_instance;
    }

    /// Create a new service instance.
    mem::shared_ptr<void> create_instance(services::container * c, injection_context * ic);

    /// Gets a named instance. Throws if an instance by that name is not configured.
    ///
    /// Will attempt to initialize the instance, if not initialized.
    mem::shared_ptr<void> get_instance(services::container * c, injection_context * ic, const strings::string & name);

    /// Gets the default instance. Throws if a default instance is not configured.
    ///
    /// Will attempt to initialize the instance, if not initialized.
    mem::shared_ptr<void> get_default_instance(services::container * c, injection_context * ic);

    void preconfigure(configuration::configuration_pass & pass) const;

    void configure(configuration::configuration_pass & pass);
  };
}
