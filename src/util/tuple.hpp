#pragma once

#include "hzd/type_traits.hpp"
#include "hzd/type_list.hpp"

namespace j::util {
  namespace detail {
    template<typename T, u32_t I>
    struct tuple_element {
      T value;
    };

    template<typename Is, typename... Ts>
    struct tuple;

    template<u32_t... Is, typename... Ts>
    struct tuple<integer_sequence<u32_t, Is...>, Ts...> : tuple_element<Ts, Is>... {
      J_BOILERPLATE(tuple, CTOR_NE)

      template<typename... Us>
      J_A(AI,ND) explicit constexpr tuple(Us && ... values) noexcept((is_nothrow_constructible_v<Ts, Us &&> && ...))
        : tuple_element<Ts, Is>{static_cast<Us &&>(values)}...
      {
      }

      template<u32_t I>
      [[nodiscard]] auto & get() noexcept {
        return tuple_element<__type_pack_element<I, Ts...>, I>::value;
      }

      template<u32_t I>
      [[nodiscard]] const auto & get() const noexcept {
        return tuple_element<__type_pack_element<I, Ts...>, I>::value;
      }

      template<typename Fn>
      inline decltype(auto) apply(Fn && fn) noexcept(noexcept(declval<Fn>()(declref<Ts>()...))) {
        return static_cast<Fn &&>(fn)(tuple_element<Ts, Is>::value...);
      }

      template<typename Fn>
      inline decltype(auto) apply(Fn && fn) const noexcept(noexcept(declval<Fn>()(declref<const Ts>()...))) {
        return static_cast<Fn &&>(fn)(tuple_element<Ts, Is>::value...);
      }

      template<typename O, typename Fn>
      inline decltype(auto) apply(O & o, Fn fn) noexcept(noexcept((declval<O>().*(declval<Fn>()))(declref<Ts>()...))) {
        return (o.*fn)(tuple_element<Ts, Is>::value...);
      }

      template<typename O, typename Fn>
      inline decltype(auto) apply(O & o, Fn fn) const noexcept(noexcept((declval<O>().*(declval<Fn>()))(declref<const Ts>()...))) {
        return (o.*fn)(tuple_element<Ts, Is>::value...);
      }
    };

    template<>
    struct tuple<integer_sequence<u32_t>> {
      template<typename Fn>
      decltype(auto) apply(Fn && fn) const noexcept(noexcept(declval<Fn>()())) {
        return static_cast<Fn &&>(fn)();
      }

      template<typename O, typename Fn>
      decltype(auto) apply(O & o, Fn fn) const noexcept(noexcept(declval<O>().*(declval<Fn>())())) {
        return (o.*fn)();
      }
    };


    template<typename... Ts>
    explicit tuple(const Ts & ...) -> tuple<make_index_sequence<sizeof...(Ts), u32_t>, Ts...>;
  }

  template<typename... Ts>
  using tuple J_NO_DEBUG_TYPE = detail::tuple<make_index_sequence<sizeof...(Ts), u32_t>, Ts...>;

  template<typename... Args>
  auto forward_as_tuple(Args && ... args)
    noexcept(noexcept(detail::tuple<::j::make_index_sequence<sizeof...(Args), u32_t>, Args &&...>(declval<Args>()...)))
  {
    return detail::tuple<make_index_sequence<sizeof...(Args), u32_t>, Args && ...>(static_cast<Args &&>(args)...);
  }
}
