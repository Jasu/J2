#pragma once

#include "exceptions/assert_lite.hpp"
#include "mem/vmem/vmem.hpp"

namespace j::mem::vmem {
  /// A contiguous sequence of zero or more memory-mapped anonymous pages.
  class page_range final {
  public:
    /// Construct an empty page range.
    J_A(AI,ND,HIDDEN) inline page_range() noexcept = default;

    /// Construct a page range given a memory-mapped address.
    J_A(AI,ND,HIDDEN) inline page_range(void * J_NOT_NULL address J_PAGE_ALIGNED, u32_t size)
      : m_address(address),
        m_size(size)
    { }

    /// Construct a page range given a memory-mapped address.
    J_A(AI,ND,HIDDEN) inline page_range(void * J_NOT_NULL address J_PAGE_ALIGNED,
                               u32_t size,
                               j::mem::vmem::protection_flags flags) noexcept
      : m_address(address),
        m_size(size),
        protection_flags(flags)
    { }

    /// Move-construct a page range.
    ///
    /// rhs will be empty after the call.
    page_range(page_range && rhs) noexcept;

    /// Move-assign to a page range.
    ///
    /// rhs will be empty after the call. The previous mapping will be returned to the page pool.
    page_range & operator=(page_range && rhs) noexcept;

    /// Get the begin address of the page range.
    J_A(AI,ND,RNN,HIDDEN,NODISC) inline void * address() const noexcept { return m_address; }

    /// Get the size of tha page range, in bytes.
    J_A(AI,ND,HIDDEN,NODISC) inline u32_t size() const noexcept {
      return m_size;
    }

    /// mprotect the region with the flags specified.
    void set_protection_flags(j::mem::vmem::protection_flags flags);

    /// Return the page range to the page pool.
    void reset() noexcept;

    /// Resets the page range, without returning it to the page pool.
    ///
    /// The caller is responsible to either returning the range to the pool, or
    /// unmapping it.
    J_A(AI,ND,HIDDEN) inline void reset_no_free() noexcept {
      m_address = nullptr;
      m_size = 0;
    }

    /// Grow the page range, allowing moving. Throws if unsuccessful.
    ///
    /// \note Empty mappings cannot be grown.
    J_A(AI,HIDDEN,ND) inline void grow(allow_move_t, u32_t num_pages) {
      grow(num_pages, true);
    }

    /// Grow the page range, disallowing moving. Returns false if moving is not possible.
    ///
    /// \note Empty mappings cannot be grown.
    J_A(AI,HIDDEN,ND) inline bool grow(disallow_move_t, u32_t num_pages) {
      return grow(num_pages, false);
    }

    /// Returns true if the page range is not empty.
    J_A(AI,ND,HIDDEN,NODISC) inline explicit operator bool() const noexcept { return m_address; }
    /// Returns true if the page range is empty.
    J_A(AI,ND,HIDDEN,NODISC) inline bool operator!() const noexcept { return !m_address; }
    /// Returns true if the page range is empty.
    J_A(AI,ND,HIDDEN,NODISC) inline bool empty() const noexcept { return !m_address; }

    J_A(AI) inline ~page_range() {
      if (m_address) {
        reset();
      }
    }

    void * m_address = nullptr;
    u32_t m_size = 0U;
    j::mem::vmem::protection_flags protection_flags = j::mem::vmem::protection_flags::rw;

    bool grow(u32_t num_pages, bool allow_move);
  };
}
