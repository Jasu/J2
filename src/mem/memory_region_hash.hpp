#pragma once

#include "hzd/crc32.hpp"
#include "hzd/string.hpp"
#include "mem/memory_region.hpp"

namespace j::mem {
  struct memory_region_hash final {
    J_A(ND,FLATTEN) u32_t operator()(const const_memory_region & reg) const noexcept {
      return crc32(0xE284AAF1, reg);
    }
  };

  struct memory_region_equals final {
    template<typename Rhs>
    J_A(ND,FLATTEN) bool operator()(const const_memory_region & lhs, const basic_memory_region<Rhs> & rhs) const noexcept {
      return lhs.size() == rhs.size() && ::j::memcmp(lhs.begin(), rhs.begin(), lhs.size()) == 0;
    }
  };
}
