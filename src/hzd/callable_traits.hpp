#pragma once

#include "hzd/type_traits.hpp"

namespace j {
  template<typename Callable> struct J_TYPE_HIDDEN callable_traits;

  template<typename R, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (*)(Args...)> {
    using result J_NO_DEBUG_TYPE = R;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename R, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (*)(Args...) noexcept> {
    using result J_NO_DEBUG_TYPE = R;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename R, typename C, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (C::*) (Args...)> {
    using result J_NO_DEBUG_TYPE = R;
    using class_t J_NO_DEBUG_TYPE = C;
    using qclass J_NO_DEBUG_TYPE = C;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename R, typename C, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (C::*) (Args...) noexcept> {
    using result J_NO_DEBUG_TYPE = R;
    using class_t J_NO_DEBUG_TYPE = C;
    using qclass J_NO_DEBUG_TYPE = C;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename R, typename C, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (C::*) (Args...) const> {
    using result J_NO_DEBUG_TYPE = R;
    using class_t J_NO_DEBUG_TYPE = C;
    using qclass J_NO_DEBUG_TYPE = const C;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename R, typename C, typename... Args> struct J_TYPE_HIDDEN callable_traits<R (C::*) (Args...) const noexcept> {
    using result J_NO_DEBUG_TYPE = R;
    using class_t J_NO_DEBUG_TYPE = C;
    using qclass J_NO_DEBUG_TYPE = const C;
    constexpr static inline u8_t nargs J_A(ND,HIDDEN) = sizeof...(Args);
    template<u32_t I> using arg J_NO_DEBUG_TYPE = __type_pack_element<I, Args...>;
  };

  template<typename MemFn>
  using memfn_class_t J_NO_DEBUG_TYPE = typename callable_traits<MemFn>::class_t;

  template<typename MemFn>
  using memfn_qualified_class_t J_NO_DEBUG_TYPE = typename callable_traits<MemFn>::qclass;

  template<typename Fn>
  using callable_result_t J_NO_DEBUG_TYPE = typename callable_traits<Fn>::result;

  template<typename Fn, u32_t I>
  using callable_arg_t J_NO_DEBUG_TYPE = typename callable_traits<Fn>::template arg<I>;
}
