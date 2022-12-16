#pragma once

#include "hzd/utility.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

namespace j::services::arg {
  template<typename Argument>
  class container_argument {
  public:
    using result_type = Argument;
    inline constexpr static bool has_dependencies_v = false;
    J_INLINE_GETTER_NO_DEBUG static bool has(void *, const void *) noexcept {
      return true;
    }
    J_INLINE_GETTER_NO_DEBUG static Argument get(services::container * c, const void *, void *) noexcept;
  };

  template<typename Argument>
  J_INLINE_GETTER_NO_DEBUG Argument container_argument<Argument>::get(services::container * c, const void *, void *) noexcept {
    return c;
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG container & container_argument<container &>::get(services::container * c, const void *, void *) noexcept {
    return *c;
  }

  template<>
  J_INLINE_GETTER_NO_DEBUG const container & container_argument<const container &>::get(services::container * c, const void *, void *) noexcept {
    return *c;
  }

  namespace detail {
    struct container_argument_selector {
      template<typename Arg>
      J_ALWAYS_INLINE J_NO_DEBUG static constexpr container_argument<Arg> select(u32_t) noexcept {
        return {};
      }
    };
  }

  J_NO_DEBUG constexpr inline auto container = detail::container_argument_selector{};
}
