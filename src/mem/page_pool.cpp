#include "mem/page_pool.hpp"

#include <sys/mman.h>
#include <cerrno>

namespace j::mem::vmem {
  page_pool page_pool::instance;

  page_range page_pool::allocate(u32_t num_pages) {
    J_ASSERT_NOT_NULL(num_pages);
    const sz_t size = num_pages * J_PAGE_SIZE;
    if (num_pages == 1U) {
      void * address = nullptr;
      if (m_num_single_pages) {
        --m_num_single_pages;
        address = m_single_pages[m_first_single_page];
        m_first_single_page = (m_first_single_page + 1U) % max_single_pages_v;
      } else {
        // Allocate more pages than needed, since mmap likely triggers a TLB
        // shootdown. Thus, mmaps shoul be clustered together, it's not just a
        // fixed system call cost.
        // Don't fill the array though - otherwise the next free would have
        // to unmap one of the pages just allocated (also causing a TLB shootdown).
        for (u32_t i = 0; i < 17U; ++i) {
          address = ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
          J_REQUIRE(address != MAP_FAILED, "mmap failed");
          if (i != 16U) {
            m_single_pages[i] = address;
          }
        }
        m_first_single_page = 0U, m_num_single_pages = 16U;
      }
      return page_range(address, size);
    }

    for (u32_t i = 0; i < max_page_ranges_v; ++i) {
      // Start search from the page that was last freed. This way, releasing and
      // immediately allocating a similar array doesn't have to scan the whole range.
      const u32_t index = (i + m_last_freed_page_range) % max_page_ranges_v;
      if (m_page_range_sizes[index] == size) {
        m_last_freed_page_range = (m_last_freed_page_range - 1) % max_page_ranges_v;
        const u32_t size = m_page_range_sizes[index];
        void * const address = m_page_ranges[index];
        J_ASSERT_NOT_NULL(address);
        m_page_range_sizes[index] = 0;
        m_page_ranges[index] = nullptr;
        return page_range(address, size);
      }
    }
    void * const address = ::mmap(nullptr, size, PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    J_REQUIRE(address != MAP_FAILED, "mmap failed");
    return page_range(address, size);

  }

  void * page_pool::grow(void * J_PAGE_ALIGNED ptr, u32_t old_sz, u32_t num_pages, bool allow_move) {
    J_ASSERT_NOT_NULL(ptr, old_sz, num_pages);
    void * const address = ::mremap(ptr, old_sz, old_sz + num_pages * J_PAGE_SIZE, allow_move ? MREMAP_MAYMOVE : 0);
    if (J_UNLIKELY(address == MAP_FAILED)) {
      J_REQUIRE(errno == ENOMEM && !allow_move, "mremap failed");
      return nullptr;
    }
    return address;
  }

  void page_pool::release(void * J_NOT_NULL J_PAGE_ALIGNED address, const u32_t sz) {
    J_ASSERT_NOT_NULL(sz, address);
    if (sz == J_PAGE_SIZE) {
      // Subtract, don't add - this makes the last page released returned on
      // the next alloc, making it more likely to still be in TLB.
      u32_t index = (m_first_single_page - 1U) % max_single_pages_v;
      if (m_num_single_pages == max_single_pages_v) {
        // Free more pages than needed, to cluster munmaps together, thus
        // reducing TLB shootdowns.
        for (u32_t i = 0; i < 9U; ++i) {
          void * victim = m_single_pages[(index + i) % max_single_pages_v];
          J_ASSERT_NOT_NULL(victim);
          J_REQUIRE(::munmap(victim, sz) == 0, "munmap failed");
        }
        m_num_single_pages -= 8U; // 9 were unmapped, but the returned page is put back
      } else {
        ++m_num_single_pages;
      }
      m_first_single_page = index;
      m_single_pages[index] = address;
      return;
    }

    u32_t max_page_size = 0U, max_index = 0U;
    for (u32_t i = 0U; i < max_page_ranges_v; ++i) {
      if (!m_page_range_sizes[i]) {
        m_page_ranges[i] = address;
        m_page_range_sizes[i] = sz;
        m_last_freed_page_range = i;
        return;
      }
      if (m_page_range_sizes[i] > max_page_size) {
        max_page_size = m_page_range_sizes[i];
        max_index = i;
      }
    }

    J_ASSERT_NOT_NULL(max_page_size, m_page_range_sizes[max_index], m_page_ranges[max_index]);
    J_REQUIRE(::munmap(m_page_ranges[max_index], 1) == 0, "munmap failed");
    m_page_ranges[max_index] = address;
    m_page_range_sizes[max_index] = sz;
    m_last_freed_page_range = max_index;
  }

  page_pool::~page_pool() {
    for (u32_t i = 0U; i < m_num_single_pages; ++i) {
      if (m_single_pages[i]) {
        J_REQUIRE(::munmap(m_single_pages[(i + m_first_single_page) % max_single_pages_v], J_PAGE_SIZE) == 0, "munmap failed");
      }
    }
    for (u32_t i = 0U; i < max_page_ranges_v; ++i) {
      if (m_page_ranges[i]) {
        J_REQUIRE(::munmap(m_page_ranges[i], m_page_range_sizes[i]) == 0, "munmap failed");
      }
    }
  }

  void page_pool::make_executable(void * J_NOT_NULL J_PAGE_ALIGNED ptr, u32_t sz) {
    J_ASSUME(sz >= J_PAGE_SIZE);
    J_REQUIRE(!::mprotect(ptr, sz, PROT_READ | PROT_EXEC),
              "mprotect failed.");
  }

  void page_pool::make_writable(void * J_NOT_NULL J_PAGE_ALIGNED ptr, u32_t sz) {
    J_ASSUME(sz >= J_PAGE_SIZE);
    J_REQUIRE(!::mprotect(ptr, sz, PROT_READ | PROT_WRITE),
              "mprotect failed.");
  }
}
