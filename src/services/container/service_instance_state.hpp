#pragma once

#include "services/configuration/configuration_union.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "mem/weak_ptr.hpp"

namespace std {
  class type_info;
}
namespace j::services {
  class container;
  struct injection_context;
  struct class_metadata;
  namespace configuration {
    class configuration_pass;
  }
  namespace registry {
    struct service_record;
  }
}

namespace j::services::detail {
  struct setting;
}
namespace j::services::detail::container {
  struct service_setting {
    const detail::setting * setting;
    configuration::configuration_union value;
  };
}

J_DECLARE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::services::detail::container::service_setting);

namespace j::services::detail::container {
  class service_type;

  /// State of a single service instance.
  struct service_instance_state {
    /// The instance of the state, if instantiated.
    mem::weak_ptr<void> m_instance;
    /// Service type.
    const service_type * m_service_type;
    /// Metadata of the instance.
    const class_metadata * m_metadata;
    /// Configured settings of the service.
    containers::copyable_unsorted_string_map<service_setting> m_settings;
    /// Priority of the instance, set from configuration.
    int priority = 0;
    /// Initialize m_instance.
    mem::shared_ptr<void> create_instance(services::container * J_NOT_NULL c, injection_context * J_NOT_NULL ic);

    service_instance_state();

    /// Creates an instance state based on a service record.
    service_instance_state(
      const registry::service_record * J_NOT_NULL record,
      const service_type * J_NOT_NULL type,
      const class_metadata * J_NOT_NULL metadata);

    type_id::type_id type_id() const noexcept;

    const char * name() const noexcept;

    /// Get the instance, but ony if it is available.
    ///
    /// If not available, return nullptr.
    mem::shared_ptr<void> get_if_instantiated() const noexcept;

    /// Gets the instance, instantiating it if not available.
    mem::shared_ptr<void> get(services::container * c, injection_context * ic);

    void preconfigure(configuration::configuration_pass & pass) const;

    void configure(configuration::configuration_pass & pass);

    bool has_setting(const type_id::type_id & setting_type, const char * name) const noexcept;

    bool has_setting(const std::type_info & setting_type, const char * name) const noexcept;

    mem::shared_ptr<void> get_setting(const type_id::type_id & setting_type, const char * J_NOT_NULL name);

    mem::shared_ptr<void> get_setting(const std::type_info & setting_type, const char * J_NOT_NULL name);

    service_instance_state(service_instance_state && rhs) noexcept;
    service_instance_state(const service_instance_state & rhs);
    service_instance_state & operator=(service_instance_state && rhs) noexcept;
    service_instance_state & operator=(const service_instance_state & rhs);

    ~service_instance_state();
  };
}
