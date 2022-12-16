#pragma once

#include "type_id/type_id.hpp"
#include "hzd/crc32.hpp"

namespace j::type_id {
  struct hash final {
    J_A(ND,FLATTEN) u32_t operator()(j::type_id::type_id t) const noexcept {
      return crc32(19, (u64_t)t.m_data | 0xFFFF000000000000UL);
    }
  };
}
