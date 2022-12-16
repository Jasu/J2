#pragma once

#include "hzd/crc32.hpp"
#include "lisp/common/id.hpp"

namespace j::lisp::inline common {
  struct id_hash final {
    J_A(ND, FLATTEN) u32_t operator()(id id) const noexcept
    { return crc32(114, id.raw); }
  };
}
