#include "hzd/mem.hpp"
#include "exceptions/exceptions.hpp"

#include <cstdlib>

namespace j {
  J_RETURNS_NONNULL void * allocate_aligned(u32_t align, u32_t size) noexcept {
    J_ASSERT(size, "Cannot allocate zero-sized objects.");
    void * result = ::aligned_alloc(align, size);
    if (J_UNLIKELY(!result)) {
      J_FAIL("::j::allocate_aligned failed.");
    }
    return result;
  }

  constinit const malloc_allocator malloc_allocator_v J_A(NODESTROY,ND);
}
