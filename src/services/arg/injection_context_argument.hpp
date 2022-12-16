#pragma once

#include "services/container/injection_context.hpp"
#include "hzd/type_traits.hpp"

namespace j::services {
  struct injection_context;
}

namespace j::services::arg {
  template<typename Argument>
  class injection_context_argument {
  public:
    inline constexpr static bool has_dependencies_v = false;
    using result_type = Argument;

    J_ALWAYS_INLINE J_NO_DEBUG static bool has(void *, const services::injection_context * ic) noexcept {
      return ic;
    }

    J_NO_DEBUG static Argument get(void *, const services::injection_context * ic, void *) {
      if constexpr (j::is_lref_v<Argument>) {
        return *ic;
      } else {
        return ic;
      }
    }
  };

  template<typename Argument>
  class parent_injection_context_argument {
  public:
    using result_type = Argument;
    inline constexpr static bool has_dependencies_v = false;

    J_ALWAYS_INLINE J_NO_DEBUG static bool has(void *, const services::injection_context * ic) noexcept {
      return ic && ic->parent;
    }

    J_NO_DEBUG static Argument get(void *, const services::injection_context * ic, void *) {
      if constexpr (j::is_lref_v<Argument>) {
        return *(ic->parent);
      } else {
        return ic->parent;
      }
    }
  };

  namespace detail {
    struct injection_context_argument_selector {
      template<typename Arg>
      J_NO_DEBUG J_ALWAYS_INLINE constexpr static injection_context_argument<Arg> select(u32_t) noexcept {
        return injection_context_argument<Arg>{};
      }
    };

    struct parent_injection_context_argument_selector {
      template<typename Arg>
      J_NO_DEBUG J_ALWAYS_INLINE constexpr static parent_injection_context_argument<Arg> select(u32_t) noexcept {
        return parent_injection_context_argument<Arg>{};
      }
    };
  }

  J_NO_DEBUG constexpr inline detail::injection_context_argument_selector injection_context{};
  J_NO_DEBUG constexpr inline detail::parent_injection_context_argument_selector parent_injection_context{};
}
