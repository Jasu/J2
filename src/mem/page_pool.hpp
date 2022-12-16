#pragma once

#include "mem/page_range.hpp"

namespace j::mem::vmem {
  class page_pool final {
  public:
    J_ALWAYS_INLINE constexpr page_pool() noexcept = default;
    page_range allocate(u32_t num_pages);
    void release(void * J_NOT_NULL J_PAGE_ALIGNED ptr, u32_t sz);
    void * grow(void * J_NOT_NULL J_PAGE_ALIGNED ptr,
                u32_t old_sz,
                u32_t num_pages,
                bool allow_move) J_RETURNS_PAGE_ALIGNED;

    void make_executable(void * J_NOT_NULL J_PAGE_ALIGNED ptr, u32_t sz);
    void make_writable(void * J_NOT_NULL J_PAGE_ALIGNED ptr, u32_t sz);

    ~page_pool();

    page_pool(const page_pool &) = delete;
    page_pool & operator=(const page_pool &) = delete;

    static page_pool instance;
  private:
    /// Number of single free pages to keep.
    constexpr static inline u32_t max_single_pages_v = 64U;
    /// Number of returne page ranges to keep.
    constexpr static inline u32_t max_page_ranges_v = 32U;

    /// Index of the first free single page.
    ///
    /// This will be set to the last page freed, so it will be returned next
    /// when allocating (so the page might even be in TLB.)
    u32_t m_first_single_page = 0U;
    /// Number of free single pages.
    u32_t m_num_single_pages = 0U;

    /// Array of addresses to single pages that are free.
    ///
    /// Each page is a separate mapping.
    void * m_single_pages[max_single_pages_v] = { nullptr };

    /// The index of the page that was most recently released.
    ///
    /// The search for page ranges starts at the last freed index, to return
    /// the most recently released page (which might even exist in TLB)
    u32_t m_last_freed_page_range = 0U;
    /// Sizes of the regions in the area.
    u32_t m_page_range_sizes[max_page_ranges_v] = { 0U };
    void * m_page_ranges[max_page_ranges_v] = { nullptr };
  };
}
