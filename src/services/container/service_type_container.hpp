#pragma once

#include "services/container/initialization_stage.hpp"
#include "containers/hash_map_fwd.hpp"
#include "containers/obstack_fwd.hpp"
#include "type_id/type_id.hpp"
#include "type_id/type_hash.hpp"

namespace j::mem {
  class shared_holder;
}
namespace j::services::detail::container {
  class service_type;
}

J_DECLARE_EXTERN_OBSTACK(j::mem::shared_holder);
J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::service_type*, HASH(j::type_id::hash));

namespace j::services {
  class container;
  namespace configuration {
    class configuration_pass;
  }
  namespace registry {
    class registry_data;
  }
}

namespace j::services::detail::container {
  class interface_container;

  class service_type_container {
  public:
    service_type_container() noexcept;

    ~service_type_container();

    /// Initialize the service type container with all service types from the registry.
    void preinitialize(const registry::registry_data & registry_data, interface_container & interfaces);

    /// Initialize default-initialized instances.
    void initialize(services::container * c, initialization_stage stage);

    void preconfigure(configuration::configuration_pass & pass) const;

    /// Reset default-initialized instances.
    void finalize() noexcept;

    void configure(configuration::configuration_pass & pass);

    obstack<mem::shared_holder> & default_initialized_instances() noexcept {
      return m_default_initialized_instances;
    }

    service_type_container(service_type_container &&) = delete;
    service_type_container(const service_type_container &) = delete;
    service_type_container & operator=(service_type_container &&) = delete;
    service_type_container & operator=(const service_type_container &) = delete;
  private:
    hash_map<type_id::type_id, service_type*, type_id::hash> m_service_types;
    obstack<mem::shared_holder> m_default_initialized_instances;
  };
}
