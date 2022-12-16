#include "hzd/amd64/cpuid.hpp"
#include "strings/string_view.hpp"

namespace j::inline hzd::amd64 {
  namespace {
    void read_cpuid_page(u32_t page_index, cpuid_leaf * J_NOT_NULL result) noexcept {
      asm("cpuid" : "=a"(result->eax), "=b"(result->ebx), "=c"(result->ecx),"=d"(result->edx)
          : "0" (page_index));
    }

    cpuid_raw cpuid_init() noexcept {
      cpuid_raw result;
      read_cpuid_page(0U, result.leaf_nodes);
      u32_t count = result.leaf_nodes[0].eax;
      count = min(count, 256);
      for (u32_t i = 1U; i < count; ++i) {
        read_cpuid_page(i, result.leaf_nodes + i);
      }
      return result;
    }
  }

  const cpuid_raw cpuid = cpuid_init();
}
