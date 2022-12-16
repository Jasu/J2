#include "services/container/interface_state.hpp"

#include "services/container/service_type.hpp"
#include "services/container/injection_context.hpp"
#include "services/registry/interface_record.hpp"
#include "services/service_reference.hpp"
#include "services/exceptions.hpp"
#include "services/metadata.hpp"
#include "services/configuration/configuration_pass.hpp"
#include "services/configuration/configuration_overlay.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/hash_map.hpp"
#include "algo/quicksort.hpp"

namespace j::services::detail::container {
  namespace {
    J_INLINE_GETTER i64_t get_service_instance_key(const service_instance_state * J_NOT_NULL lhs) noexcept {
      return (i64_t)lhs->priority;
    }
  }
}

J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::detail::container::service_type*, HASH(j::type_id::hash));
J_DEFINE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::services::detail::container::service_instance_state*);
J_DEFINE_EXTERN_VECTOR(j::services::detail::container::service_instance_state*);

namespace j::services::detail::container {
  void interface_state::sort_instances() {
    const auto conf = mem::reinterpret_pointer_cast<configuration::interface_configuration>(m_configuration.create_configuration());
    m_ordered_instances.clear();
    for (auto v : m_instances_by_name.values()) {
      v->priority = 0;
      for (auto & impl : conf->implementations) {
        if (impl.first.matches(v->type_id(), v->name())) {
          v->priority = impl.second.priority;
          break;
        }
      }
      m_ordered_instances.push_back(v);
      algo::quicksort(m_ordered_instances.begin(), m_ordered_instances.end(), &get_service_instance_key);
    }
  }

  interface_state::interface_state()
    : m_configuration(
      type_id::type_id(typeid(configuration::interface_configuration)),
      configuration::config_factory_t(&mem::make_void_shared<configuration::interface_configuration>))
  {
  }

  interface_state::~interface_state() {
  }

  mem::shared_ptr<void> interface_state::get_default_configuration() const {
    J_ASSERT_NOT_NULL(m_record);
    return mem::make_void_shared<configuration::interface_configuration>(m_record->default_configuration);
  }

  /// Construct interface state based on a record from the registry.
  interface_state::interface_state(const registry::interface_record & record)
    : m_record(&record),
      m_configuration(
        type_id::type_id(typeid(configuration::interface_configuration)),
        configuration::config_factory_t::bind<&interface_state::get_default_configuration>(this)
  ),
      m_name(record.metadata->name)
  {
  }

  /// Register a service type as implementing the interface.
  void interface_state::add_implementation(const type_id::type_id & service_type_id, service_type * service_type) {
    J_REQUIRE_EX(service_type, exception()
                 << message("Tried to register null service type.")
                 << err_service_type(service_type_id)
                 << err_interface_name(m_name));
    bool did_insert = m_implementations.emplace(service_type_id, service_type).second;
    J_REQUIRE_EX(did_insert, exception()
                 << message("Implementation was already registered.")
                 << err_interface_name(m_name)
                 << err_service_type(service_type_id)
                 << err_service_name(service_type->get_name()));
  }

  void interface_state::add_instance(const strings::string & name, service_instance_state * instance) {
    J_REQUIRE_EX(instance, exception()
                 << message("Tried to register null service instance.")
                 << err_interface_name(m_name)
                 << err_service_instance_name(name));
    bool did_insert = m_instances_by_name.emplace(name, instance).second;
    J_REQUIRE_EX(did_insert, exception()
                 << message("Instance was already registered.")
                 << err_interface_name(m_name)
                 << err_service_instance_name(name));
    const auto conf = mem::reinterpret_pointer_cast<configuration::interface_configuration>(m_configuration.create_configuration());
    for (auto & impl : conf->implementations) {
      if (impl.first.matches(instance->type_id(), name)) {
        instance->priority = impl.second.priority;
        break;
      }
    }
    for (auto it = m_ordered_instances.begin(), end = m_ordered_instances.end(); it != end; ++it) {
      if ((*it)->priority <= instance->priority) {
        m_ordered_instances.emplace(it, instance);
        return;
      }
    }
    m_ordered_instances.emplace_back(instance);
  }

  /// Get implementations instances, ordered by priority.
  ///
  /// Instantiates all implementations.
  vector<mem::shared_ptr<void>> interface_state::ordered_implementations(services::container * c, const injection_context * parent_ic) {
    injection_context ic{
      .parent = parent_ic,
      .interface_metadata = m_record->metadata,
    };
    vector<mem::shared_ptr<void>> result;
    result.reserve(m_ordered_instances.size());
    for (auto instance : m_ordered_instances) {
      result.emplace_back(instance->get(c, &ic));
    }
    return result;
  }

  /// Checks if the interface contains a service by ref.
  bool interface_state::has(const service_reference & ref) const noexcept {
    if (ref.empty()) {
      if (mem::reinterpret_pointer_cast<configuration::interface_configuration>(m_configuration.create_configuration())->is_per_service) {
        return m_default_implementation;
      }
      return m_default_instance;
    }
    auto & name = ref.service_instance_name();
    if (!name.empty()) {
      strings::string name_str(name);
      auto it = m_instances_by_name.find(name_str);
      if (it == m_instances_by_name.end()) {
        return false;
      }
      auto type = ref.service_type_id();
      return type.empty() || (*it->second)->type_id() == type;
    }

    auto it = m_implementations.find(ref.service_type_id());
    if (it == m_implementations.end()) {
      return false;
    }
    return it->second->has_default_instance();
  }

  /// Get a service by ref, throwing if it does not exist.
  mem::shared_ptr<void> interface_state::get(services::container * c, const injection_context * parent_ic, const service_reference & ref) {
    injection_context ic{
      .parent = parent_ic,
      .interface_metadata = m_record->metadata,
    };
    if (ref.empty()) {
      if (mem::reinterpret_pointer_cast<configuration::interface_configuration>(m_configuration.create_configuration())->is_per_service) {
        J_REQUIRE_EX(m_default_implementation, exception()
                     << message("Interface has no default implementation.")
                     << err_interface_name(m_name));
        return m_default_implementation->create_instance(c, &ic);
      }
      J_REQUIRE_EX(m_default_instance, exception()
                   << message("Interface has no default instance.")
                   << err_interface_name(m_name));
      return m_default_instance->get(c, &ic);
    }
    auto & name = ref.service_instance_name();
    if (!name.empty()) {
      strings::string name_str(name);
      auto it = m_instances_by_name.find(name_str);
      J_REQUIRE_EX(it != m_instances_by_name.end(), exception()
                   << message("Interface has no named instance.")
                   << err_interface_name(m_name)
                   << err_service_instance_name(name_str));
      auto type_id = ref.service_type_id();
      J_REQUIRE_EX(type_id.empty() || type_id == (*it->second)->type_id(), exception()
                   << message("Named instance does not match type.")
                   << err_interface_name(m_name)
                   << err_service_instance_name(name_str)
                   << err_service_type(type_id));
      return (*it->second)->get(c, &ic);
    }

    auto type_id = ref.service_type_id();
    auto it = m_implementations.find(type_id);
    J_REQUIRE_EX(it != m_implementations.end(), exception()
                 << message("Interface has no instance by type.")
                 << err_interface_name(m_name)
                 << err_service_type(type_id));
    J_REQUIRE_EX(it->second->has_default_instance(), exception()
                 << message("Implementation by type has no default instance.")
                 << err_interface_name(m_name)
                 << err_service_name(it->second->get_name())
                 << err_service_type(type_id));
    return it->second->get_default_instance(c, &ic);
  }

  void interface_state::configure(configuration::configuration_pass & pass) {
    pass.configure_interface(m_name, m_configuration);
    update_configuration();
  }

  void interface_state::update_configuration() {
    sort_instances();
    const auto def = mem::reinterpret_pointer_cast<configuration::interface_configuration>(m_configuration.create_configuration())->default_implementation;
    for (auto & impl : m_implementations) {
      if (def.matches(impl.second->type_id(), impl.second->get_name())) {
        m_default_implementation = impl.second;
      }
    }
    for (auto & inst : m_instances_by_name) {
      if (def.matches((*inst.second)->type_id(), *inst.first)) {
        m_default_instance = *inst.second;
      }
    }
  }

  const class_metadata * interface_state::metadata() const {
    J_ASSERT_NOT_NULL(m_record);
    return m_record->metadata;
  }
}
