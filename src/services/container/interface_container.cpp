#include "services/container/interface_container.hpp"
#include "services/registry/registry.hpp"
#include "services/registry/registry_data.hpp"
#include "services/exceptions.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "services/container/interface_state.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::interface_state*, HASH(j::type_id::hash));

namespace j::services::detail::container {
  void interface_container::preinitialize(const registry::registry_data & registry_data) {
    auto & records = registry_data.interfaces();
    for (auto & record : records) {
      m_interfaces.emplace(record.first, ::new interface_state(*record.second));
    }
  }

  interface_container::interface_container() noexcept {
  }

  interface_container::~interface_container() {
  }

  void interface_container::initialize() {
    for (auto & p : m_interfaces) {
      p.second->update_configuration();
    }
  }

  void interface_container::configure(configuration::configuration_pass & pass) {
    for (auto & p : m_interfaces) {
      p.second->configure(pass);
    }
  }

  void interface_container::finalize() {
    for (auto & p : m_interfaces) {
      ::delete p.second;
    }
    m_interfaces.clear();
  }

  interface_state & interface_container::get_interface(type_id::type_id interface) {
    auto it = m_interfaces.find(interface);
    J_REQUIRE_EX(it != m_interfaces.end(), exception()
                 << message("Interface not found.")
                 << err_interface_type(interface));
    return *it->second;
  }

  const interface_state & interface_container::get_interface(type_id::type_id interface) const {
    auto it = m_interfaces.find(interface);
    J_REQUIRE_EX(it != m_interfaces.end(), exception()
                 << message("Interface not found.")
                 << err_interface_type(interface));
    return *it->second;
  }

  interface_state * interface_container::maybe_get_interface(type_id::type_id interface) {
    interface_state ** s = m_interfaces.maybe_at(interface);
    return s ? *s : nullptr;
  }

  const interface_state * interface_container::maybe_get_interface(type_id::type_id interface) const {
    const interface_state * const * s = m_interfaces.maybe_at(interface);
    return s ? *s : nullptr;
  }

  bool interface_container::has_interface(type_id::type_id interface) const {
    return m_interfaces.contains(interface);
  }
}
