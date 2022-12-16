#pragma once

#include "containers/span.hpp"
#include "hzd/concepts.hpp"

namespace j::algo {
  enum class iteration_result : u8_t {
    next,
    exit,
  };

  template<typename Fn, typename... Args>
  concept WithIterationResult = requires(Fn fn, Args && ... args) {
    { fn(static_cast<Args &&>(args)...) } -> SameAs<iteration_result>;
  };

  template<typename T, typename Fn, typename... Args>
  constexpr void for_each(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    for (auto & v : span) {
      static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...);
    }
  }

  template<typename T, typename Fn, typename... Args> requires WithIterationResult<Fn, Args && ...>
  constexpr iteration_result for_each(const span<T> & span, Fn && fn, Args && ... args)
    noexcept(is_nothrow_callable_v<Fn, T &, Args...>)
  {
    for (auto & v : span) {
      if (static_cast<Fn &&>(fn)(v, static_cast<Args &&>(args)...) == iteration_result::exit) {
        return iteration_result::exit;
      }
    }
    return iteration_result::next;
  }
}
