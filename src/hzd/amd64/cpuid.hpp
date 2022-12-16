#pragma once

#include "hzd/types.hpp"

namespace j::inline hzd::amd64 {
  struct cpuid_leaf final {
    u32_t eax = 0U;
    u32_t ebx = 0U;
    u32_t ecx = 0U;
    u32_t edx = 0U;
  };

  struct cpuid_raw final {
    cpuid_leaf leaf_nodes[256];
  };

  extern const cpuid_raw cpuid;
}
