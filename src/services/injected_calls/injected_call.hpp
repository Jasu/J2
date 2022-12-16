#pragma once

#include "services/injected_calls/argument_list.hpp"

namespace j::services::injected_calls {
  enum class required_tag_t { v };
  enum class optional_tag_t { v };

  template<bool IsOptional, typename Callable, typename WiredArgumentList>
  class J_HIDDEN injected_call {
    Callable m_fn;
    WiredArgumentList m_arguments;
  public:
    using injected_call_tag_t J_NO_DEBUG_TYPE = void;
    J_INLINE_GETTER const WiredArgumentList & get_arguments() const noexcept {
      return m_arguments;
    }

    inline constexpr static bool has_dependencies_v = WiredArgumentList::has_dependencies_v;

    constexpr injected_call() noexcept = default;

    template<typename Tag, typename C>
    J_ALWAYS_INLINE explicit constexpr injected_call(Tag, C && fn, WiredArgumentList && wired_argument_list) noexcept
      : m_fn(static_cast<C &&>(fn)),
        m_arguments(static_cast<WiredArgumentList &&>(wired_argument_list))
    {
    }

    template<typename... PrefixArguments>
    J_ALWAYS_INLINE decltype(auto) operator()(container * c, const injection_context * ic, services::detail::dependencies_t * deps, PrefixArguments && ... prefix_arguments) const {
      if constexpr (IsOptional) {
        if (m_arguments.can_invoke(c, ic)) {
          m_arguments.invoke(m_fn, c, ic, deps, static_cast<PrefixArguments &&>(prefix_arguments)...);
        }
      } else {
        return m_arguments.invoke(m_fn, c, ic, deps, static_cast<PrefixArguments &&>(prefix_arguments)...);
      }
    }
  };

  template<typename Callable, typename ArgList>
  explicit injected_call(required_tag_t, const Callable &, const ArgList &) -> injected_call<false, Callable, ArgList>;

  template<typename Callable, typename ArgList>
  explicit injected_call(optional_tag_t, const Callable &, const ArgList &) -> injected_call<true, Callable, ArgList>;

  template<typename T>
  inline constexpr bool is_injected_call_v = false;

  template<bool O, typename T, typename U>
  inline constexpr bool is_injected_call_v<injected_call<O, T, U>> = true;
}
