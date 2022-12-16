#include "services/service_definition.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::services::detail::setting);
J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::services::registry::implements_record);
J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::services::injected_calls::type_erased_injected_call);

namespace j::services {
  namespace {
    inline mem::shared_ptr<void> identity_cast(mem::shared_ptr<void> value) noexcept {
      return value;
    }
  }

  service_definition_base::service_definition_base(sz_t num_settings,
                                                   sz_t num_initializers,
                                                   sz_t num_implements,
                                                   sz_t num_finalizers,
                                                   bool was_interface_registered,
                                                   const std::type_info & type)
    : m_settings(containers::uninitialized, num_settings),
      m_initializers(containers::uninitialized, num_initializers),
      m_implements(containers::uninitialized, num_implements + (was_interface_registered ? 1 : 0)),
      m_finalizers(num_finalizers),
      m_type(type),
      m_was_interface_registered(was_interface_registered)
  {
    if (was_interface_registered) {
      m_implements.initialize_element(registry::implements_record{
        m_type,
        &identity_cast,
        0
      });
    }
  }

  service_definition_base::~service_definition_base() {
    // This usually happens in static destruction phase, and the registry might already be
    // destroyed. Avoid re-creating it.
    if (registry::instance.is_initialized) {
      registry::instance.unsafe_instance()->unregister_service(
        m_type,
        m_was_interface_registered
      );
    }
  }
}
