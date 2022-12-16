#pragma once

#include "algo/for_each.hpp"

namespace j::algo {
  template<typename T, typename Fn, typename... Args>
  constexpr u32_t count(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    u32_t result = 0U;
    for (auto & v : span) {
      result += static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...);
    }
    return result;
  }

  template<typename T>
  J_INLINE_GETTER constexpr u32_t count_eq(const span<T> & span, const T & value) noexcept
  { return count(span, [&value](const T & v) { return v == value; }); }

  template<typename T, typename Fn, typename... Args>
  constexpr bool any(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    return iteration_result::exit == for_each(
      span,
      [fn{static_cast<Fn &&>(fn)}](T & v, Args && ... args) -> iteration_result {
        return static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...)
          ? iteration_result::exit
          : iteration_result::next;
      }, static_cast<Args &&>(args)...);
  }

  template<typename T, typename Fn, typename... Args>
  constexpr bool all(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    return iteration_result::next == for_each(
      span,
      [fn{static_cast<Fn &&>(fn)}](T & v, Args && ... args) -> iteration_result {
        return static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...)
          ? iteration_result::next
          : iteration_result::exit;
      }, static_cast<Args &&>(args)...);
  }

  template<typename T, typename Fn, typename... Args>
  constexpr bool none(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    return iteration_result::next == for_each(
      span,
      [fn{static_cast<Fn &&>(fn)}](T & v, Args && ... args) -> iteration_result {
        return static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...)
          ? iteration_result::exit
          : iteration_result::next;
      }, static_cast<Args &&>(args)...);
  }
}
