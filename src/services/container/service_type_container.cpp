#include "services/container/service_type_container.hpp"
#include "services/registry/registry_data.hpp"
#include "services/container/injection_context.hpp"
#include "services/container/service_type.hpp"
#include "services/metadata.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "services/configuration/configuration_union.hpp"
#include "containers/obstack.hpp"
#include "containers/hash_map.hpp"
#include "mem/shared_holder.hpp"

J_DEFINE_EXTERN_OBSTACK(j::mem::shared_holder);
J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::service_type*, HASH(j::type_id::hash));

namespace j::services::detail::container {
  service_type_container::service_type_container() noexcept {
  }

  service_type_container::~service_type_container() {
    for (auto & p : m_service_types) {
      ::delete p.second;
    }
  }

  void service_type_container::preinitialize(const registry::registry_data & registry_data, interface_container & interfaces) {
    const auto & services = registry_data.services();
    for (auto & p : services) {
      m_service_types.emplace(p.first, ::new service_type(p.first, p.second, registry_data, interfaces));
    }
  }

  void service_type_container::initialize(services::container * c, initialization_stage stage) {
    J_ASSERT_NOT_NULL(c);
    for (auto & p : m_service_types) {
      p.second->handle_default_initialization(c, stage, this);
    }
  }


  void service_type_container::preconfigure(configuration::configuration_pass & pass) const {
    for (auto & p : m_service_types) {
      p.second->preconfigure(pass);
    }
  }

  void service_type_container::configure(configuration::configuration_pass & pass) {
    for (auto & p : m_service_types) {
      p.second->configure(pass);
    }
  }

  void service_type_container::finalize() noexcept {
    m_default_initialized_instances.clear();
  }
}
