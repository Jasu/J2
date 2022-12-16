#pragma once

#include "strings/string_view.hpp"

#include "services/container/service_type_container.hpp"
#include "services/container/interface_container.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include <typeinfo>

namespace j::inline containers {
  template<typename T> class vector;
}
namespace j::services::configuration {
  class static_configuration_pass;
}

namespace j::services {
  struct injection_context;
  class service_reference;

  class container final {
    enum class state {
      initial,
      preinitialized,
      initialized,
      finalized,
    } m_state;

    void add_service_configuration(
      strings::const_string_view service_name,
      strings::const_string_view setting_name,
      const std::type_info & type,
      mem::shared_ptr<void> && ptr);

    void add_interface_configuration(
      strings::const_string_view service_name,
      const std::type_info & type,
      mem::shared_ptr<void> && ptr);
  public:
    container();

    ~container();

    /// Fetch services and interfaces from the registry.
    void preinitialize();

    /// Initializes and configure the container.
    ///
    /// If the container is not preinitialized, preinitialize as well.
    void initialize();

    template<typename StrA, typename StrB, typename Variant>
    J_ALWAYS_INLINE void configure_service(StrA && service_name, StrB && setting_name, Variant && configuration) {
      add_service_configuration(
        static_cast<StrA &&>(service_name),
        static_cast<StrB &&>(setting_name),
        typeid(decltype(identity(configuration))),
        mem::make_void_shared<decltype(identity(configuration))>(static_cast<Variant &&>(configuration)));
    }

    template<typename Str, typename Conf>
    J_ALWAYS_INLINE void configure_interface(Str && interface_name, Conf && configuration) {
      add_interface_configuration(
        static_cast<Str &&>(interface_name),
        typeid(decltype(identity(configuration))),
        mem::make_void_shared<decltype(identity(configuration))>(static_cast<Conf &&>(configuration)));
    }

    /// Shut down the container.
    ///
    /// This releases references to all default-initialized instances.
    void finalize() noexcept;

    /// Check for existence of an interface.
    bool has_interface(const type_id::type_id & interface_type) const noexcept;

    /// Check for existence of an interface.
    bool has_interface(const std::type_info & interface_type) const noexcept;

    /// Check for existence of a service.
    bool has_service(const type_id::type_id & type, const service_reference & ref) const noexcept;

    /// Check for existence of a service.
    bool has_service(const std::type_info & type, const service_reference & ref) const noexcept;

    /// Get a service instance.
    ///
    /// Throw if the service is not found or cannot be initialized.
    mem::shared_ptr<void> get(type_id::type_id, const service_reference & ref, const injection_context * parent_ic);

    /// Get a service instance.
    ///
    /// Throw if the service is not found or cannot be initialized.
    mem::shared_ptr<void> get(const type_id::type_id & type_id);

    /// Get a service instance.
    ///
    /// Throw if the service is not found or cannot be initialized.
    mem::shared_ptr<void> get(const std::type_info & type_id);

    template<typename Interface>
    J_INLINE_GETTER mem::shared_ptr<Interface> get() {
      return mem::reinterpret_pointer_cast<Interface>(get(typeid(Interface)));
    }
    template<typename Interface>
    J_INLINE_GETTER mem::shared_ptr<Interface> get(const service_reference & ref, const injection_context * parent_ic = nullptr) {
      return mem::reinterpret_pointer_cast<Interface>(get(type_id::type_id(typeid(Interface)), ref, parent_ic));
    }

    /// Get a vector of service instances implementing an interface.
    ///
    /// Throw if the interface is not found or cannot be initialized.
    vector<mem::shared_ptr<void>> get_all(const type_id::type_id & type, const injection_context * parent_ic);

    /// Get a vector of service instances implementing an interface.
    ///
    /// Throw if the interface is not found or cannot be initialized.
    vector<mem::shared_ptr<void>> get_all(const std::type_info & type, const injection_context * parent_ic);

    container & operator=(const container &) = delete;
    container & operator=(container &&) = delete;
    container(const container &) = delete;
    container(container &&) = delete;
  private:
    void run_configuration_pass(configuration::configuration_pass & pass);

    detail::container::interface_container m_interfaces;
    detail::container::service_type_container m_service_types;
    configuration::static_configuration_pass * m_static_configuration_pass;
  };
}
