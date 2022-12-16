#include "services/arg/service_argument.hpp"
#include "services/container.hpp"

namespace j::services::arg::detail {
  bool service_argument_base::has_impl(const std::type_info & interface_type,
                                       services::container * c,
                                       const services::injection_context *) const noexcept
  {
    return m_is_optional || c->has_service(interface_type, m_service);
  }

  mem::shared_ptr<void> service_argument_base::get_impl(const std::type_info & interface_type,
                                                        services::container * c,
                                                        const services::injection_context * ic) const
  {
    if (m_is_optional) {
      if (!c->has_service(interface_type, m_service)) {
        return {};
      }
    }
    return c->get(type_id::type_id(interface_type), m_service, ic);
  }
}
