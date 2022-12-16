#pragma once

#include "hzd/utility.hpp"

namespace j::strings::inline tables {
  enum class border_style : u8_t {
    table_default,
    none,

    light,
    light_round,
    dashed,
    light_triple_dash,
    light_quadruple_dash,
    double_light,
    heavy,
    heavy_dashed,
    heavy_triple_dash,
    heavy_quadruple_dash,
  };
}
