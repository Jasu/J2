#pragma once

#include "hzd/utility.hpp"

namespace j::inline containers {
  template<typename First, typename Second = First>
  struct pair final {
    First first;
    Second second;
  };

  template<typename First, typename Second>
  pair(First, Second) -> pair<First, Second>;

  template<typename F, typename S>
  J_ALWAYS_INLINE bool operator==(const pair<F, S> & lhs, const pair<F, S> & rhs) noexcept {
    return lhs.first == rhs.first && lhs.second == rhs.second;
  }

  template<typename F, typename S>
  J_ALWAYS_INLINE bool operator<(const pair<F, S> & lhs, const pair<F, S> & rhs) noexcept {
    return lhs.first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
  }

  template<typename F, typename S>
  J_ALWAYS_INLINE bool operator>(const pair<F, S> & lhs, const pair<F, S> & rhs) noexcept {
    return operator<(rhs, lhs);
  }

  template<typename F, typename S>
  J_ALWAYS_INLINE bool operator<=(const pair<F, S> & lhs, const pair<F, S> & rhs) noexcept {
    return !operator<(rhs, lhs);
  }

  template<typename F, typename S>
  J_ALWAYS_INLINE bool operator>=(const pair<F, S> & lhs, const pair<F, S> & rhs) noexcept {
    return !operator<(lhs, rhs);
  }

  template<typename It>
  struct iterator_pair final {
    It first;
    bool second;
  };
}
