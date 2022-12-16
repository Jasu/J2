#pragma once

#include "services/arg/injection_context_argument.hpp"

namespace j::services::arg {
  namespace detail {
    template<typename Argument, typename ContextArgument>
    class service_metadata_argument_impl {
    public:
      using result_type J_NO_DEBUG_TYPE = Argument;

      J_NO_DEBUG inline constexpr static bool has_dependencies_v = false;

      [[nodiscard]] J_NO_DEBUG inline constexpr static bool has(void *, const services::injection_context * ic) noexcept
      { return ContextArgument::has(nullptr, ic); }

      [[nodiscard]] J_NO_DEBUG inline static decltype(auto) get(
        void *,
        const services::injection_context * ic,
        void *
      )

    {
      auto arg = ContextArgument::get(nullptr, ic, nullptr)->service_metadata;
      if constexpr (j::is_ptr_v<Argument>) {
        return arg;
      } else {
        return *arg;
      }
    }
    };

    template<typename Argument, typename ContextArgument>
    class interface_metadata_argument_impl {
    public:
      J_NO_DEBUG inline constexpr static bool has_dependencies_v = false;
      using result_type J_NO_DEBUG_TYPE = Argument;

      [[nodiscard]] J_NO_DEBUG inline static bool has(void *, const services::injection_context * ic) noexcept {
        return ContextArgument::has(nullptr, ic) && ContextArgument::get(nullptr, ic, nullptr)->interface_metadata;
      }

      [[nodiscard]] J_NO_DEBUG inline static decltype(auto) get(void *, const services::injection_context * ic, void *) {
        auto interface_metadata = ContextArgument::get(nullptr, ic, nullptr)->interface_metadata;
        J_ASSERT(interface_metadata, "Interface metadata does not exist when injecting.");
        if constexpr (j::is_ptr_v<Argument>) {
          return interface_metadata;
        } else {
          return *interface_metadata;
        }
      }
    };


  }

  namespace detail {
    struct interface_metadata_selector {
      template<typename Argument>
      J_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::interface_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>{};
      }
    };

    struct service_metadata_selector {
      template<typename Argument>
      J_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::service_metadata_argument_impl<Argument, injection_context_argument<const services::injection_context *>>{};
      }
    };

    struct parent_interface_metadata_selector {
      template<typename Argument>
      J_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::interface_metadata_argument_impl<Argument, parent_injection_context_argument<const services::injection_context *>>{};
      }
    };

    struct parent_service_metadata_selector {
      template<typename Argument>
      J_NO_DEBUG constexpr static auto select(u32_t) noexcept {
        return detail::service_metadata_argument_impl<Argument, parent_injection_context_argument<const services::injection_context *>>{};
      }
    };
  }

  J_NO_DEBUG constexpr inline auto interface_metadata = detail::interface_metadata_selector{};
  J_NO_DEBUG constexpr inline auto service_metadata = detail::service_metadata_selector{};
  J_NO_DEBUG constexpr inline auto parent_interface_metadata = detail::parent_interface_metadata_selector{};
  J_NO_DEBUG constexpr inline auto parent_service_metadata = detail::parent_service_metadata_selector{};
}
