#pragma once

#include "services/arg/metadata_arguments.hpp"
#include "hzd/type_traits.hpp"

namespace j::services {
  class container;
}

namespace j::services::arg {
  namespace detail {
    template<typename Argument, typename MetadataArgument, bool IsTitle>
    class name_argument_impl {
    public:
      using result_type = Argument;
      constexpr static bool has_dependencies_v = false;

      constexpr static auto has = MetadataArgument::has;

      J_NO_DEBUG inline static Argument get(
        services::container * c,
        const services::injection_context * ic,
        void *
      );
    };

    template<typename Argument, typename MetadataArgument, bool IsTitle>
    J_NO_DEBUG inline Argument name_argument_impl<Argument, MetadataArgument, IsTitle>::get(
      services::container * c,
      const services::injection_context * ic,
      void *
    ) {
      if constexpr (IsTitle) {
        return MetadataArgument::get(c, ic, nullptr)->title;
      } else {
        return MetadataArgument::get(c, ic, nullptr)->name;
      }
    }
  }

  template<typename Argument>
  using interface_name_argument J_NO_DEBUG_TYPE = detail::name_argument_impl<Argument, detail::interface_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>, false>;

  template<typename Argument>
  using interface_title_argument J_NO_DEBUG_TYPE = detail::name_argument_impl<Argument, detail::interface_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>, true>;

  template<typename Argument>
  using service_name_argument J_NO_DEBUG_TYPE = detail::name_argument_impl<Argument, detail::service_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>, false>;

  template<typename Argument>
  using service_title_argument J_NO_DEBUG_TYPE = detail::name_argument_impl<Argument, detail::service_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>, true>;

  namespace detail {
    struct service_name_selector {
      template<typename Arg>
      J_NO_DEBUG constexpr static auto select(u32_t) { return service_name_argument<j::remove_cref_t<Arg>>{}; }
    };
    struct service_title_selector {
      template<typename Arg>
      J_NO_DEBUG constexpr static auto select(u32_t) { return service_title_argument<j::remove_cref_t<Arg>>{}; }
    };
    struct interface_name_selector {
      template<typename Arg>
      J_NO_DEBUG constexpr static auto select(u32_t) { return interface_name_argument<j::remove_cref_t<Arg>>{}; }
    };
    struct interface_title_selector {
      template<typename Arg>
      J_NO_DEBUG constexpr static auto select(u32_t) { return interface_title_argument<j::remove_cref_t<Arg>>{}; }
    };
  }

  J_NO_DEBUG constexpr inline auto service_name = detail::service_name_selector{};
  J_NO_DEBUG constexpr inline auto service_title = detail::service_title_selector{};
  J_NO_DEBUG constexpr inline auto interface_name = detail::interface_name_selector{};
  J_NO_DEBUG constexpr inline auto interface_title = detail::interface_title_selector{};
}
