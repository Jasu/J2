#pragma once

#include "services/detail/service_type.hpp"
#include "services/service_reference.hpp"
#include "services/arg/service_argument_adapter.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

namespace j::services::arg {
  namespace detail {
    class service_argument_base {
      service_reference m_service;
      bool m_is_optional = false;

    protected:
      J_NO_DEBUG bool has_impl(const std::type_info & interface_type, services::container * c, const services::injection_context * ic) const noexcept;

      J_NO_DEBUG mem::shared_ptr<void> get_impl(const std::type_info & interface_type, services::container * c, const services::injection_context * ic) const;
    public:
      constexpr service_argument_base() noexcept = default;

      constexpr service_argument_base(const service_reference & service, bool is_optional) noexcept
        : m_service(service),
          m_is_optional(is_optional)
      {
      }
    };
  }

  template<typename ArgumentType>
  class service_argument : public detail::service_argument_base {
    using interface_type_t J_NO_DEBUG_TYPE = services::detail::service_type_t<ArgumentType>;
  public:
    using service_argument_base::service_argument_base;
    using result_type = J_NO_DEBUG_TYPE remove_ref_t<ArgumentType>;

    J_NO_DEBUG inline constexpr static bool has_dependencies_v = service_argument_adapter<ArgumentType &, interface_type_t>::has_dependencies_v;

    [[nodiscard]] bool has(services::container * c, const services::injection_context * ic) const noexcept {
      return has_impl(typeid(interface_type_t), c, ic);
    }

    [[nodiscard]] decltype(auto) get(services::container * c, const services::injection_context * ic, services::detail::dependencies_t * deps) const {
      return service_argument_adapter<ArgumentType &, interface_type_t>::transform(get_impl(typeid(interface_type_t), c, ic), deps);
    }
  };
}

namespace j::services {
  template<typename Arg>
  J_INLINE_GETTER constexpr arg::service_argument<Arg> service_reference::select(u32_t) const noexcept {
    return arg::service_argument<Arg>(*this, false);
  }
}
