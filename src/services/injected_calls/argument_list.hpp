#pragma once

#include "util/tuple.hpp"
#include "services/detail/util.hpp"
#include "services/arg/arg.hpp"

namespace j::services::injected_calls {
  template<u32_t... Is, typename... Ts, typename Fn, typename... PrefixArgs>
  inline static decltype(auto) invoke_wired_argument_list(
    const util::detail::tuple<integer_sequence<u32_t, Is...>, Ts...> & args,
    Fn && fn,
    container * c,
    const injection_context * ic,
    detail::dependencies_t * deps,
    PrefixArgs && ... prefix_args
  ) {
    return static_cast<Fn &&>(fn)(
      static_cast<PrefixArgs &&>(prefix_args)...,
      static_cast<const util::detail::tuple_element<Ts, Is> &>(args).value.get(c, ic, deps)...);
  }

  template<u32_t... Is, typename... Ts, typename Result, typename Service, typename... FnArgs>
  inline static Result invoke_wired_argument_list(
    const util::detail::tuple<integer_sequence<u32_t, Is...>, Ts...> & args,
    Result (Service::*fn)(FnArgs...),
    container * c,
    const injection_context * ic,
    detail::dependencies_t * deps,
    Service * service
  ) {
    return (service->*fn)(static_cast<const util::detail::tuple_element<Ts, Is> &>(args).value.get(c, ic, deps)...);
  }

  template<typename... WiredArgs>
  class J_HIDDEN wired_argument_list {
    util::tuple<WiredArgs...> m_args;
  public:
    J_INLINE_GETTER constexpr const auto & as_tuple() const noexcept {
      return m_args;
    }

    J_BOILERPLATE(wired_argument_list, CTOR_CE, COPY_DEL, MOVE_CE)

    inline constexpr static bool has_dependencies_v = (WiredArgs::has_dependencies_v || ...);

    template<typename... Args>
    J_ALWAYS_INLINE explicit constexpr wired_argument_list(Args && ... args) noexcept
      : m_args(static_cast<Args &&>(args)...)
    { }

    template<typename Fn, typename... PrefixArgs>
    J_ALWAYS_INLINE decltype(auto) invoke(Fn && fn, container * c, const injection_context * ic,
                                    detail::dependencies_t * deps, PrefixArgs && ... prefix_args) const;

    template<typename Result, typename Service, typename... Args>
    J_ALWAYS_INLINE decltype(auto) invoke(Result (Service::* callable)(Args...), container * c,
                                    const injection_context * ic, detail::dependencies_t * deps,
                                    Service * service) const;

    J_INLINE_GETTER bool can_invoke(container * c, const injection_context * ic) const noexcept;
  };


  template<typename... WiredArgs>
  template<typename Fn, typename... PrefixArgs>
  J_ALWAYS_INLINE decltype(auto) wired_argument_list<WiredArgs...>::invoke(
    Fn && fn, container * c, const injection_context * ic,
    detail::dependencies_t * deps, PrefixArgs && ... prefix_args) const
  {
    return invoke_wired_argument_list(m_args, static_cast<Fn &&>(fn), c, ic, deps,
                                      static_cast<PrefixArgs &&>(prefix_args)...);
  }

  template<typename... WiredArgs>
  template<typename Result, typename Service, typename... Args>
  J_ALWAYS_INLINE decltype(auto) wired_argument_list<WiredArgs...>::invoke(
    Result (Service::* callable)(Args...), container * c, const injection_context * ic,
    detail::dependencies_t * deps, Service * service) const
  { return invoke_wired_argument_list(m_args, callable, c, ic, deps, service); }

  template<typename... WiredArgs>
  J_INLINE_GETTER bool wired_argument_list<WiredArgs...>::can_invoke(container * c, const injection_context * ic) const noexcept {
    return m_args.apply([=](const auto & ... args) noexcept {
      return (args.has(c, ic) && ...);
    });
  }

  template<>
  class wired_argument_list<> {
  public:
    J_INLINE_GETTER constexpr const util::tuple<> as_tuple() const noexcept {
      return util::tuple<>{};
    }

    J_NO_DEBUG inline constexpr static bool has_dependencies_v = false;

    template<typename Fn, typename... PrefixArgs>
    J_ALWAYS_INLINE decltype(auto) invoke(Fn && fn,
                                    container *,
                                    const injection_context *,
                                    detail::dependencies_t *,
                                    PrefixArgs && ... prefix_args) const
    { return static_cast<Fn &&>(fn)(static_cast<PrefixArgs &&>(prefix_args)...); }

    template<typename Result, typename Service, typename... Args>
    J_ALWAYS_INLINE  decltype(auto) invoke(Result (Service::* fn)(Args...),
                                     container *,
                                     const injection_context *,
                                     detail::dependencies_t *,
                                     Service * service) const
    { return (service->*fn)(); }

    J_INLINE_GETTER bool can_invoke(container *, const injection_context *) const noexcept {
      return true;
    }
  };

  template<typename... Args>
  explicit wired_argument_list(Args ... args) -> wired_argument_list<Args...>;

  template<typename... FunctionArgs, typename... Selectors>
  J_INLINE_GETTER constexpr auto wire(Selectors && ... selectors) noexcept {
    u32_t i = 0;
    return wired_argument_list{static_cast<Selectors &&>(selectors).template select<FunctionArgs>(i++)...};
  }

  template<typename... FunctionArgs>
  J_INLINE_GETTER constexpr auto wire() noexcept {
    u32_t i = 0;
    return wired_argument_list{arg::detail::autowire_selector::select<FunctionArgs>(i++)...};
  }
}
