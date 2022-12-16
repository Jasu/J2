#include "services/container/service_type.hpp"
#include "containers/trivial_array.hpp"
#include "services/registry/service_record.hpp"
#include "services/exceptions.hpp"
#include "services/container/service_instance_state.hpp"
#include "services/container/injection_context.hpp"
#include "services/container/interface_container.hpp"
#include "services/metadata.hpp"
#include "services/registry/registry_data.hpp"
#include "services/container/interface_state.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "containers/unsorted_string_map.hpp"

J_DEFINE_EXTERN_UNSORTED_STRING_MAP(j::services::detail::container::service_instance_state);

namespace j::services::detail::container {
  service_type::~service_type() {
  }

  service_type::service_type(
    const type_id::type_id & service_type_id,
    const registry::service_record * record,
    const registry::registry_data & registry_data,
    interface_container & interfaces
  )
    : m_type_id(service_type_id),
      m_record(record)
  {

    J_ASSERT_NOT_NULL(m_record);
    J_ASSERT(!m_type_id.empty(), "Type id of a service cannot be empty.");
    // Default instance
    if (m_record->default_instance) {
      m_default_instance = &(m_instances[m_record->metadata->name] = {
        m_record, this, m_record->metadata
      });
    }

    for (auto & md : registry_data.instances(m_type_id)) {
      m_instances[md->name] = { record, this, md };
    }
    J_REQUIRE_EX(m_record->get, exception()
                 << message("Service factory was null.")
                 << err_service_name(m_record->metadata->name));
    J_REQUIRE_EX(m_record->implements, exception()
                 << message("Service record has null implementations")
                 << err_service_name(m_record->metadata->name));

    // Register to all interfaces
    for (auto & implements : *m_record->implements) {
      auto interface = interfaces.maybe_get_interface(implements.interface_type);
      if (!interface) {
        // Implements an interface that was not registered.
        continue;
      }
      interface->add_implementation(service_type_id, this);
      for (auto & instance : m_instances) {
        interface->add_instance(instance.second->name(), instance.second);
      }
    }
  }

  void service_type::handle_default_initialization(
    services::container * c,
    initialization_stage stage,
    service_type_container * type_container
  ) {
    J_ASSERT_NOT_NULL(c, type_container);
    if (m_record->initialization_stage == stage) {
      injection_context ic;
      ic.service_metadata = m_record->metadata;
      for (auto & instance : m_instances) {
        type_container->default_initialized_instances().push(mem::shared_holder(instance.second->get(c, &ic)));
      }
    }
  }

  mem::shared_ptr<void> service_type::create_instance(services::container * c, injection_context * ic) {
    J_ASSERT_NOT_NULL(c, ic);
    ic->service_metadata = m_record->metadata;
    return m_record->get(c, ic);
  }

  mem::shared_ptr<void> service_type::get_instance(services::container * c, injection_context * ic, const strings::string & name) {
    J_ASSERT_NOT_NULL(c, ic, name);
    auto it = m_instances.find(name);
    J_REQUIRE_EX(it != m_instances.end(), exception()
                 << message("Service instance was not found.")
                 << err_service_name(m_record->metadata->name)
                 << err_service_type(m_type_id)
                 << err_service_instance_name(name)
                 << err_interface_name(ic->interface_metadata->name));
    ic->service_metadata = m_record->metadata;
    return it->second->get(c, ic);
  }

  mem::shared_ptr<void> service_type::get_default_instance(services::container * c, injection_context * ic) {
    J_ASSERT_NOT_NULL(c, ic);
    J_REQUIRE_EX(m_default_instance,
                 exception()
                 << message("Service has no default instance.")
                 << err_service_name(m_record->metadata->name)
                 << err_service_type(m_type_id));
    ic->service_metadata = m_record->metadata;
    return m_default_instance->get(c, ic);
  }

  void service_type::preconfigure(configuration::configuration_pass & pass) const {
    for (auto & p : m_instances) {
      p.second->preconfigure(pass);
    }
  }

  void service_type::configure(configuration::configuration_pass & pass) {
    for (auto & p : m_instances) {
      p.second->configure(pass);
    }
  }

  const char * service_type::get_name() const noexcept {
    return m_record->metadata->name;
  }
}
