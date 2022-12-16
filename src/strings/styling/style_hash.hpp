#pragma once

#include "strings/styling/style.hpp"
#include "hzd/crc32.hpp"

namespace j::strings::inline styling {
  struct style_hash final {
    J_A(ND, FLATTEN) u32_t operator()(const style & s) const noexcept {
      return crc32(91, (const char*)&s, sizeof(style));
    }
  };

}
