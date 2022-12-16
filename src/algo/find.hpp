#pragma once

#include "containers/span.hpp"

namespace j::algo {
  template<typename Begin, typename End, typename Fn, typename... Args>
  constexpr auto * find(Begin begin, End end, Fn && fn, Args && ... args) noexcept {
    for (; begin != end; ++begin) {
      if (static_cast<Fn &&>(fn)(*begin, static_cast<Args &&>(args)...)) {
        return &*begin;
      }
    }
    return nullptr;
  }

  template<typename T, typename Fn, typename... Args>
  J_INLINE_GETTER constexpr T * find(const span<T> & span, Fn && fn, Args && ... args) noexcept
  { return find(span.begin(), span.end(), static_cast<Fn &&>(fn), static_cast<Args &&>(args)...); }
}
