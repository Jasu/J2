#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::air_to_cir {
  enum class result_use : u8_t {
    used,
    unused,
    ptr_only,
    size_only,
    truthiness_only,
  };

  constexpr inline auto result_used = result_use::used;
  constexpr inline auto result_unused = result_use::unused;
  constexpr inline auto ptr_used = result_use::ptr_only;
  constexpr inline auto size_used = result_use::size_only;
  constexpr inline auto bool_used = result_use::truthiness_only;
}
