#pragma once

#include "containers/hash_map_fwd.hpp"
#include "containers/vector.hpp"
#include "type_id/type_id.hpp"
#include "type_id/type_hash.hpp"

namespace j::services {
  struct class_metadata;
}
namespace j::services::registry {
  struct service_record;
  struct interface_record;
  class registry;
}

extern template class j::vector<const j::services::class_metadata*>;

J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::registry::service_record*,
                          HASH(j::type_id::hash));
J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::services::registry::interface_record*,
                          HASH(j::type_id::hash));
J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::vector<const j::services::class_metadata*>,
                          HASH(j::type_id::hash));

namespace j::services::registry {
  class registry_data {
    friend class ::j::services::registry::registry;
    hash_map<type_id::type_id, service_record*, type_id::hash> m_services;
    hash_map<type_id::type_id, interface_record*, type_id::hash> m_interfaces;
    hash_map<type_id::type_id, vector<const class_metadata*>, type_id::hash> m_instances;
  public:
    registry_data() noexcept;
    registry_data(registry_data &&) = delete;
    registry_data(const registry_data &) = delete;
    ~registry_data();

    const auto & interfaces() const noexcept {
      return m_interfaces;
    }

    const auto & services() const noexcept {
      return m_services;
    }

    const auto & instances(const type_id::type_id & service_type) const {
      return m_instances.at(service_type);
    }
  };
}
