#pragma once

#include "hzd/utility.hpp"

namespace j::algo {
  struct less_than final {
    template<typename Lhs, typename Rhs>
    J_INLINE_GETTER constexpr bool operator()(const Lhs & lhs, const Rhs & rhs) const noexcept
     {return lhs < rhs; }
  };

  struct equal_to final {
    template<typename Lhs, typename Rhs>
    J_INLINE_GETTER constexpr bool operator()(const Lhs & lhs, const Rhs & rhs) const noexcept
    { return lhs == rhs; }
  };
}
