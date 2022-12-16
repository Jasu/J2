#include "services/container.hpp"

#include "services/registry/registry.hpp"
#include "services/registry/registry_data.hpp"
#include "services/container/injection_context.hpp"
#include "services/container/initialization_stage.hpp"
#include "services/container/service_instance_state.hpp"
#include "services/service_reference.hpp"
#include "services/container/interface_state.hpp"
#include "services/configuration/static_configuration_pass.hpp"
#include "exceptions/assert.hpp"

namespace j::services {
  container::container()
    : m_state(state::initial),
      m_static_configuration_pass(::new configuration::static_configuration_pass())
  {
  }

  container::~container() {
    ::delete m_static_configuration_pass;
  }

  void container::add_service_configuration(
    strings::const_string_view service_name,
    strings::const_string_view setting_name,
    const std::type_info & type,
    mem::shared_ptr<void> && ptr)
  {
    J_ASSERT_NOT_NULL(service_name, setting_name, ptr);
    m_static_configuration_pass->add_service_configuration_wrapper(
      service_name,
      setting_name,
      { type, static_cast<mem::shared_ptr<void> &&>(ptr) });
  }

  void container::add_interface_configuration(
    strings::const_string_view interface_name,
    const std::type_info & type,
    mem::shared_ptr<void> && ptr
  ) {
    J_ASSERT_NOT_NULL(interface_name, ptr);
    m_static_configuration_pass->add_interface_configuration_wrapper(
      interface_name,
      { type, static_cast<mem::shared_ptr<void> &&>(ptr) });
  }

  void container::preinitialize() {
    J_ASSERT(m_state != state::finalized, "Tried to initialize a finalized container.");
    if (m_state != state::initial) {
      return;
    }
    auto & registry_data = registry::instance->data();
    m_interfaces.preinitialize(registry_data);
    m_service_types.preinitialize(registry_data, m_interfaces);

    m_state = state::preinitialized;
  }

  void container::initialize() {
    preinitialize();
    if (m_state != state::preinitialized) {
      return;
    }

    m_interfaces.initialize();
    // Initialize automatically initialized instances by default.
    m_service_types.initialize(this, initialization_stage::before_static_configuration);

    run_configuration_pass(*m_static_configuration_pass);

    m_service_types.initialize(this, initialization_stage::after_static_configuration);

    // Get the instances as states instead of instantiating them all at once.
    // Otherwise previous passes would not be able to affect subsequent passes.
    auto & interface_state = m_interfaces.get_interface(type_id::type_id(typeid(configuration::configuration_pass)));
    // Copy the pass states as it might get re-ordered.
    auto pass_states = interface_state.ordered_instance_states();
    for (auto pass_state : pass_states) {
      injection_context ic{
        .interface_metadata = interface_state.metadata(),
      };
      auto pass = pass_state->get(this, &ic);
      run_configuration_pass(*mem::reinterpret_pointer_cast<configuration::configuration_pass>(pass));
    }

    m_service_types.initialize(this, initialization_stage::after_dynamic_configuration);

    m_state = state::initialized;
  }

  bool container::has_interface(const type_id::type_id & interface_type) const noexcept {
    return m_interfaces.has_interface(interface_type) && m_state != state::finalized;
  }

  bool container::has_interface(const std::type_info & interface_type) const noexcept {
    return has_interface(type_id::type_id(interface_type));
  }

  void container::finalize() noexcept {
    if (m_state == state::finalized) {
      return;
    }
    m_service_types.finalize();
    m_interfaces.finalize();
    m_state = state::finalized;
  }

  bool container::has_service(const type_id::type_id & interface_type, const service_reference & ref) const noexcept {
    if (m_state == state::finalized) {
      return false;
    }
    auto interface = m_interfaces.maybe_get_interface(interface_type);
    return interface ? interface->has(ref) : false;
  }

  bool container::has_service(const std::type_info & interface_type, const service_reference & ref) const noexcept {
    return has_service(type_id::type_id(interface_type), ref);
  }

  mem::shared_ptr<void> container::get(const type_id::type_id & interface_type) {
    J_ASSERT(m_state != state::finalized, "Tried to get a service from a finalized container.");
    service_reference ref;
    return m_interfaces.get_interface(interface_type).get(this, nullptr, ref);
  }

  mem::shared_ptr<void> container::get(const std::type_info & type_id) {
    return get(type_id::type_id(type_id));
  }

  mem::shared_ptr<void> container::get(type_id::type_id interface_type, const service_reference & ref, const injection_context * parent_ic) {
    J_ASSERT(m_state != state::finalized, "Tried to get a service from a finalized container.");
    return m_interfaces.get_interface(interface_type).get(this, parent_ic, ref);
  }

  vector<mem::shared_ptr<void>> container::get_all(const type_id::type_id & interface_type, const injection_context * parent_ic) {
    J_REQUIRE(m_state != state::finalized, "Tried to get a service from a finalized container.");
    return m_interfaces.get_interface(interface_type).ordered_implementations(this, parent_ic);
  }

  vector<mem::shared_ptr<void>> container::get_all(const std::type_info & interface_type, const injection_context * parent_ic) {
    return get_all(type_id::type_id(interface_type), parent_ic);
  }

  void container::run_configuration_pass(configuration::configuration_pass & pass) {
    m_interfaces.configure(pass);
    m_service_types.preconfigure(pass);
    pass.before_configure_services();
    m_service_types.configure(pass);
  }
}
