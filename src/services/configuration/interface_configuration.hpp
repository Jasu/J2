#pragma once

#include "services/configuration/implementation_configuration.hpp"
#include "services/stored_service_reference.hpp"
#include "containers/hash_map_fwd.hpp"
#include "strings/string_hash.hpp"
#include "type_id/type_hash.hpp"

namespace j::services::configuration {
  struct stored_service_reference_hash final {
    J_A(ND, FLATTEN, NODISC) u32_t operator()(const stored_service_reference & ref) const noexcept {
      return m_type_hash(ref.service_type_id()) ^ strings::hash_string(ref.service_instance_name());
    }
    [[no_unique_address]] type_id::hash m_type_hash;
  };
}

J_DECLARE_EXTERN_HASH_MAP(
  j::services::stored_service_reference,
  j::services::configuration::implementation_configuration,
  HASH(j::services::configuration::stored_service_reference_hash));

namespace j::services::configuration {
  /// Interface configuration.
  struct interface_configuration final {
    using implementations_t = hash_map<stored_service_reference, implementation_configuration,
                                                   stored_service_reference_hash>;

    /// Default implementation of the interface.
    stored_service_reference default_implementation;

    /// Configuration for all the implementations.
    implementations_t implementations;

    /// If set, a new instance of the interface is created for each service requesting it.
    bool is_per_service;

    interface_configuration() noexcept;

    explicit interface_configuration(bool is_per_service) noexcept;

    template<typename ServiceReference>
    explicit interface_configuration(ServiceReference && default_implementation,
                                     bool is_per_service)
      : default_implementation(static_cast<ServiceReference &&>(default_implementation)),
        is_per_service(is_per_service)
    {
    }

    interface_configuration(const interface_configuration & rhs);
    interface_configuration(interface_configuration && rhs) noexcept;
    interface_configuration & operator=(const interface_configuration & rhs);
    interface_configuration & operator=(interface_configuration && rhs) noexcept;

    ~interface_configuration();

    // bool operator==(const interface_configuration & rhs) const noexcept;
    // bool operator!=(const interface_configuration & rhs) const noexcept;
  };
}
