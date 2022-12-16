#pragma once

#include "hzd/type_traits.hpp"
#include "services/arg/container_argument.hpp"
#include "services/arg/injection_context_argument.hpp"
#include "services/arg/service_argument.hpp"
#include "services/arg/service_collection_argument.hpp"

namespace j::services::arg {
  namespace detail {
    template<typename>
    J_NO_DEBUG inline constexpr bool is_container_v = false;
    template<typename T>
    J_NO_DEBUG inline constexpr bool is_container_v<vector<T>> = true;
    // template<typename T>
    // J_NO_DEBUG inline constexpr bool is_container_v<span<T>> = true;
    // template<typename T>
    // J_NO_DEBUG inline constexpr bool is_container_v<deque<T>> = true;

    struct autowire_selector {
      template<typename Arg>
      J_NO_DEBUG J_HIDDEN inline static constexpr decltype(auto) select(u32_t index) noexcept {
        using bare_t J_NO_DEBUG_TYPE = j::remove_const_t<j::remove_ref_or_ptr_t<Arg>>;
        if constexpr (j::is_same_v<bare_t, services::container>) {
          return container_argument_selector::select<Arg>(index);
        } else if constexpr (j::is_same_v<bare_t, services::injection_context>) {
          return injection_context_argument_selector::select<Arg>(index);
        } else if constexpr (is_container_v<bare_t>) {
          return service_collection_selector::select<bare_t>(index);
        } else {
          return service_reference{}.select<Arg>(index);
        }
      }
    };
  }

  J_NO_DEBUG constexpr inline detail::autowire_selector autowire{};
}
