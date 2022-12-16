#pragma once

#include "services/detail/dependencies_t.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::services::arg {
  template<typename Arg, typename InterfaceType>
  struct service_argument_adapter;

  // Shared ptr to the service
  template<typename InterfaceType>
  struct service_argument_adapter<mem::shared_ptr<InterfaceType> &, InterfaceType> {
    inline constexpr static bool has_dependencies_v = false;
    [[nodiscard]] J_NO_DEBUG inline static mem::shared_ptr<InterfaceType> transform(mem::shared_ptr<void> && ptr, void *) noexcept {
      return mem::reinterpret_pointer_cast<InterfaceType>(static_cast<mem::shared_ptr<void> &&>(ptr));
    }
  };

  template<typename InterfaceType>
  struct service_argument_adapter<const mem::shared_ptr<InterfaceType> &, InterfaceType>
    : service_argument_adapter<mem::shared_ptr<InterfaceType> &, InterfaceType>
  { };

  // Plain reference
  template<typename InterfaceType>
  struct service_argument_adapter<InterfaceType &, InterfaceType> {
    inline constexpr static bool has_dependencies_v = true;

    static InterfaceType & transform(mem::shared_ptr<void> && ptr, services::detail::dependencies_t * deps) {
      void * result = ptr.get();
      deps->emplace(static_cast<mem::shared_ptr<void> &&>(ptr));
      return *reinterpret_cast<InterfaceType*>(result);
    }
  };

  template<typename InterfaceType>
  struct service_argument_adapter<const InterfaceType &, InterfaceType>
    : service_argument_adapter<InterfaceType &, InterfaceType> { };

  // Plain pointer
  template<typename InterfaceType>
  struct service_argument_adapter<InterfaceType * &, InterfaceType> {
    inline constexpr static bool has_dependencies_v = true;

    static InterfaceType * transform(mem::shared_ptr<void> && ptr, services::detail::dependencies_t * deps) {
      void * result = ptr.get();
      deps->emplace(static_cast<mem::shared_ptr<void> &&>(ptr));
      return reinterpret_cast<InterfaceType*>(result);
    }
  };

  template<typename InterfaceType>
  struct service_argument_adapter<const InterfaceType * &, InterfaceType>
    : service_argument_adapter<InterfaceType * &, InterfaceType>
  { };
}
