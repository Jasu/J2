#pragma once

#include "util/singleton.hpp"
#include "services/container/initialization_stage.hpp"
#include "type_id/type_id.hpp"

namespace j::mem {
  template<typename T> class shared_ptr;
}

namespace j::inline containers {
  template<typename> class trivial_array;
}

namespace j::services {
  class container;
  struct class_metadata;
  struct injection_context;
  class service_reference;
}

namespace j::services::detail {
  struct setting;
}


namespace j::services::registry {
  struct implements_record;

  /// Factory function type for services.
  using service_getter = mem::shared_ptr<void> (*)(container*, const injection_context*);

  /// Converter method for casting services to interfaces.
  using service_caster = mem::shared_ptr<void>(*)(mem::shared_ptr<void>);

  class registry_data;

  /// Singleton class, managing the registration of all services and interfaces in the application.
  class registry final {
    friend struct ::j::util::singleton<registry>;
  private:
    registry_data * m_data;
    registry();
  public:
    ~registry();

    void register_service(
      const type_id::type_id & type,
      const class_metadata * metadata,
      service_getter && get,
      const trivial_array<detail::setting> * settings,
      const trivial_array<implements_record> * implements,
      initialization_stage initialization_stage,
      const class_metadata * default_instance,
      bool has_interface,
      bool one_per_service);

    void register_instance(
      const std::type_info & service_type,
      const class_metadata * instance);

    void unregister_service(type_id::type_id type, bool has_interface);

    void register_interface(
      const type_id::type_id & type,
      const class_metadata * metadata,
      const service_reference * default_implementation,
      bool is_per_service
    );

    void unregister_interface(type_id::type_id type);

    const registry_data & data() const noexcept {
      return *m_data;
    }

    registry(const registry &) = delete;
    registry & operator=(const registry &) = delete;
  };
}

extern template struct j::util::singleton<j::services::registry::registry>;

namespace j::services::registry {
  extern constinit ::j::util::singleton<registry> instance;
}
