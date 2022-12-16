#pragma once

#include "hzd/concepts.hpp"

namespace j {
  template<typename InputIt, typename OutputIt>
  constexpr inline OutputIt copy(InputIt start, InputIt end, OutputIt out) {
    while (start != end) {
      *out = *start;
      ++out, ++start;
    }
    return out;
  }

  template<typename OutputIt, typename V>
  constexpr inline OutputIt fill(OutputIt start, OutputIt end, const V & value) {
    while (start != end) {
      *start = value;
      ++start;
    }
    return start;
  }

  template<typename T>
  struct ref_eq {
    template<typename Rhs>
    J_A(ND,NODISC) inline constexpr bool operator()(const T & lhs, const Rhs & rhs) const noexcept {
      return lhs == rhs;
    }
  };

  struct int_eq {
    J_A(AI,ND,NODISC) inline constexpr bool operator()(u64_t lhs, u64_t rhs) const noexcept {
      return lhs == rhs;
    }
  };

  struct ptr_eq {
    J_A(AI,ND,NODISC) inline constexpr bool operator()(const void * lhs, const void * rhs) const noexcept {
      return lhs == rhs;
    }
  };

  template<typename T>
  struct equal_to_helper { using type = ref_eq<T>; };

  template<typename T>
  struct equal_to_helper<T*> { using type = ptr_eq; };

  template<Integral T>
  struct equal_to_helper<T> { using type = int_eq; };

  template<typename T>
  using equal_to = typename equal_to_helper<T>::type;
}
