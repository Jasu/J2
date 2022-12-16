#include "mem/vmem/vmem.hpp"
#include "hzd/mem.hpp"
#include "exceptions/exceptions.hpp"

#include <sys/mman.h>
#include <cerrno>

namespace j::mem::vmem {
  static_assert(PROT_NONE == (i32_t)protection_flags::none);
  static_assert(PROT_READ == (i32_t)protection_flags::r);
  static_assert(PROT_WRITE == (i32_t)protection_flags::w);
  static_assert(PROT_EXEC == (i32_t)protection_flags::x);

  [[nodiscard]] J_RETURNS_NONNULL void * map(u32_t bytes,
                                             alloc_flags flags,
                                             protection_flags prot)
  {
    J_ASSERT((bytes & J_PAGE_SIZE_MASK) == 0U && bytes > 0U);
    int f = MAP_ANONYMOUS | MAP_PRIVATE;
    if (flags & alloc_flags::no_reserve) {
      f |= MAP_NORESERVE;
    }
    errno = 0;
    void * const result = ::mmap(nullptr, bytes, prot, f, -1, 0U);
    if (result == MAP_FAILED) {
      J_THROW(bad_alloc_exception() << message("mmap failed."));
    }
    return result;
  }

  [[nodiscard]] J_RETURNS_NONNULL void * map(u32_t bytes) {
    errno = 0;
    void * const result = ::mmap(nullptr, bytes, PROT_WRITE | PROT_READ, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0U);
    if (result == MAP_FAILED) {
      J_THROW(bad_alloc_exception() << message("mmap failed."));
    }
    return result;
  }

  void unmap(void * J_NOT_NULL page, u32_t bytes) {
    J_ASSUME((bytes & J_PAGE_SIZE_MASK) == 0U);
    J_ASSUME(bytes > 0U);
    errno = 0;
    if (::munmap(page, bytes)) {
      J_THROW(system_error_exception() << message("munmap failed."));
    }
  }

  void protect(void * J_NOT_NULL ptr, u32_t bytes, protection_flags flags) {
    J_ASSUME((bytes & J_PAGE_SIZE_MASK) == 0U && bytes > 0U);
    errno = 0;
    if (::mprotect(ptr, bytes, flags)) {
      J_THROW(system_error_exception() << message("mprotect failed."));
    }
  }

  [[nodiscard]] void * grow(void * J_NOT_NULL ptr, u32_t old_size, u32_t new_size, bool allow_move) {
    J_ASSUME((new_size & J_PAGE_SIZE_MASK) == 0U);
    J_ASSUME(new_size > 0U);
    J_ASSUME((old_size & J_PAGE_SIZE_MASK) == 0U);
    J_ASSUME(old_size > 0U);
    J_ASSUME(old_size < new_size);
    errno = 0;
    void * result = ::mremap(ptr, old_size, new_size, allow_move ? MREMAP_MAYMOVE : 0);
    if (result != MAP_FAILED) {
      return result;
    }
    if (errno == ENOMEM && !allow_move) {
      return nullptr;
    }
    J_THROW(system_error_exception() << message("mremap failed"));
  }

  void replace_pages(void * J_NOT_NULL to_ptr, void * J_NOT_NULL from_ptr, u32_t bytes) {
    J_ASSUME((bytes & J_PAGE_SIZE_MASK) == 0U);
    J_ASSUME(bytes > 0U);
    J_ASSUME(to_ptr != from_ptr);
    errno = 0;
    if (MAP_FAILED == ::mremap(from_ptr, bytes, bytes, MREMAP_FIXED | MREMAP_MAYMOVE, to_ptr)) {
      J_THROW(system_error_exception() << message("mremap failed"));
    }
  }
}
