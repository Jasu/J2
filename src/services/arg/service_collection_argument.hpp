#pragma once

#include "services/container.hpp"
#include "services/detail/service_type.hpp"
#include "services/arg/service_collection_adapter.hpp"

namespace j::services::arg {
  template<typename ArgumentType>
  class service_collection_argument;

  template<typename ArgumentType>
  class service_collection_argument<vector<ArgumentType>> {
    using interface_type_t = services::detail::service_type_t<ArgumentType>;
  public:
    using result_type = vector<ArgumentType>;
    J_NO_DEBUG inline constexpr static bool has_dependencies_v = service_collection_adapter<ArgumentType, interface_type_t>::has_dependencies_v;

    [[nodiscard]] J_NO_DEBUG inline static bool has(services::container * c, const services::injection_context *) {
      return c->has_interface(typeid(interface_type_t));
    }

    [[nodiscard]] J_NO_DEBUG inline static auto get(services::container * c, const services::injection_context * ic, services::detail::dependencies_t * deps) {
      return service_collection_adapter<ArgumentType, interface_type_t>::transform(c->get_all(typeid(interface_type_t), ic), deps);
    }
  };

  namespace detail {
    struct service_collection_selector {
      template<typename Arg>
      [[nodiscard]] inline J_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return service_collection_argument<j::remove_cref_t<Arg>>{};
      }
    };
  }

  constexpr inline auto service_collection J_NO_DEBUG_TYPE = detail::service_collection_selector{};
}
