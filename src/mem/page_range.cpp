#include "mem/page_range.hpp"

#include "mem/page_pool.hpp"
#include "hzd/mem.hpp"

#include <sys/mman.h>

namespace j::mem::vmem {
  page_range::page_range(page_range && rhs) noexcept
    : m_address(rhs.m_address),
      m_size(rhs.m_size),
      protection_flags(rhs.protection_flags)
  {
    rhs.m_address = nullptr;
    rhs.m_size = 0U;
  }

  page_range & page_range::operator=(page_range && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      if (m_address) { reset(); }
      m_address = J_ASSUME_PAGE_ALIGNED(rhs.m_address);
      m_size = rhs.m_size;
      protection_flags = rhs.protection_flags;
      rhs.m_address = nullptr, rhs.m_size = 0U;
    }
    return *this;
  }

  void page_range::reset() noexcept {
    J_ASSUME_NOT_NULL(m_address);
    J_ASSUME(m_size >= J_PAGE_SIZE);
    page_pool::instance.release(m_address, m_size);
    m_address = nullptr, m_size = 0U;
  }

  bool page_range::grow(u32_t num_pages, bool allow_move) {
    J_ASSERT(m_address && m_size >= J_PAGE_SIZE);
    void * new_ptr = page_pool::instance.grow(m_address, m_size, num_pages, allow_move);
    if (!new_ptr) {
      return false;
    }
    m_size += J_PAGE_SIZE;
    m_address = new_ptr;
    return true;
  }

  void page_range::set_protection_flags(j::mem::vmem::protection_flags flags) {
    J_ASSERT(m_address && m_size >= J_PAGE_SIZE);
    if (J_LIKELY(!mprotect(m_address, m_size, flags))) {
      return;
    }
    J_THROW("mprotect failed.");
  }
}
