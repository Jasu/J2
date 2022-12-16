#include "services/registry/registry.hpp"
#include "mem/shared_ptr.hpp"
#include "containers/hash_map.hpp"
#include "services/registry/service_record.hpp"
#include "services/registry/interface_record.hpp"
#include "services/exceptions.hpp"
#include "services/metadata.hpp"
#include "services/registry/registry_data.hpp"
#include "containers/vector.hpp"

template class j::vector<const j::services::class_metadata*>;
J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::registry::service_record*,
                         HASH(j::type_id::hash));
J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::registry::interface_record*,
                         HASH(j::type_id::hash));
J_DEFINE_EXTERN_HASH_MAP(j::type_id::type_id, j::vector<const j::services::class_metadata*>,
                         HASH(j::type_id::hash));
template struct j::util::singleton<j::services::registry::registry>;

namespace j::services::registry {
  constinit ::j::util::singleton<registry> instance;

  registry::registry() : m_data(new registry_data) { }

  registry::~registry() {
    ::delete m_data;
    m_data = nullptr;
  }

  void registry::register_service(const type_id::type_id & type,
                                  const class_metadata * metadata,
                                  service_getter && get,
                                  const trivial_array<detail::setting> * settings,
                                  const trivial_array<implements_record> * implements,
                                  initialization_stage initialization_stage,
                                  const class_metadata * default_instance,
                                  bool has_interface,
                                  bool one_per_service)
  {
    J_ASSERT_NOT_NULL(metadata);
    m_data->m_services.emplace(
      type,
      ::new service_record{
        .metadata = metadata,
        .get = static_cast<service_getter &&>(get),
        .settings = settings,
        .implements = implements,
        .initialization_stage = initialization_stage,
        .default_instance = default_instance,
      }
    );

    if (has_interface) {
      m_data->m_interfaces.emplace(
          type,
          ::new interface_record{
            .metadata = metadata,
            .default_configuration = configuration::interface_configuration(
              stored_service_reference{ type, metadata->name },
              one_per_service
            ),
          }
      );
    }

    // Just ensure that the vector for implementations exists.
    m_data->m_instances[type];
  }

  void registry::register_instance(const std::type_info & service_type, const class_metadata * instance) {
    J_ASSERT_NOT_NULL(instance);
    const type_id::type_id type(service_type);
    m_data->m_instances[type].emplace_back(instance);
  }

  void registry::unregister_service(type_id::type_id type, bool has_interface) {
    ::delete m_data->m_services.at(type);
    m_data->m_services.erase(m_data->m_services.find(type));
    if (has_interface) {
      unregister_interface(type);
    }
  }

  void registry::register_interface(
    const type_id::type_id & type,
    const class_metadata * metadata,
    const service_reference * default_implementation,
    bool is_per_service
  ) {
    J_ASSERT_NOT_NULL(metadata);
    m_data->m_interfaces.emplace(
      type,
      ::new interface_record{
        .metadata = metadata,
          .default_configuration = default_implementation ? configuration::interface_configuration(
            *default_implementation,
            is_per_service
            ) : configuration::interface_configuration(is_per_service),
          }
      );
  }

  void registry::unregister_interface(type_id::type_id type) {
    auto it = m_data->m_interfaces.find(type);
    J_ASSERT_EX(it != m_data->m_interfaces.end(), exception()
                << message("Interface being unregistered was not found.")
                << err_interface_type(type));
    ::delete it->second;
    m_data->m_interfaces.erase(it);
  }
  registry_data::registry_data() noexcept {
  }

  registry_data::~registry_data() {
    for (auto & p : m_interfaces) {
      ::delete p.second;
    }
    for (auto & p : m_services) {
      ::delete p.second;
    }
  }
}
