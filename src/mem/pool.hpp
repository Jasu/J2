#pragma once

#include "mem/page_pool.hpp"
#include "hzd/type_traits.hpp"
#include "hzd/mem.hpp"
#include "bits/static_bitset.hpp"

namespace j::mem {
  template<u32_t ElementSize>
  class pool_page {
  private:
    static constexpr inline u32_t element_set_sz_v = sizeof(u64_t) + 64U * ElementSize;
    static constexpr inline u32_t usable_sz_v = J_PAGE_SIZE - sizeof(pool_page*) * 2 - sizeof(u64_t);
    static constexpr inline u32_t num_full_sets_v = usable_sz_v / element_set_sz_v;
  public:
    static constexpr inline u32_t num_elements_v = ((usable_sz_v - num_full_sets_v * element_set_sz_v) - sizeof(u64_t)) / ElementSize + num_full_sets_v * 64U;

    J_INLINE_GETTER_NONNULL u8_t * region_start() noexcept {
      return reinterpret_cast<u8_t*>(this + 1U);
    }

    J_INLINE_GETTER_NONNULL u8_t * region_end() noexcept {
      return region_start() + num_elements_v * ElementSize;
    }

    void assert_valid() const {
      J_ASSERT(is_page_aligned(this), "Not page-aligned");
      J_ASSUME(m_previous->m_next == this);
      J_ASSUME(m_next->m_previous == this);
      J_ASSUME(num_elements_v >= m_size);
    }

    void * allocate() noexcept J_RETURNS_PAGE_ALIGNED {
      assert_valid();
      if (J_UNLIKELY(m_size == num_elements_v)) {
        return nullptr;
      }
      const u32_t index = m_bitset.find_and_clear_first_set();
      J_ASSUME(index < num_elements_v);
      ++m_size;
      u8_t * result = region_start() + ElementSize * index;
      J_ASSERT_RANGE(region_start(), result + ElementSize, reinterpret_cast<u8_t*>(this) + J_PAGE_SIZE);
      return result;
    }

    void move_before(pool_page * J_NOT_NULL page) noexcept {
      J_ASSUME(page != this);
      assert_valid();
      detach();
      m_previous = page->m_previous;
      m_next = page;
      page->m_previous = page->m_previous->m_next = this;
    }

    void move_after(pool_page * J_NOT_NULL page) noexcept {
      J_ASSUME(page != this);
      assert_valid();
      detach();
      m_previous = page;
      m_next = page->m_next;
      page->m_next = page->m_next->m_previous = this;
    }

    J_INLINE_GETTER bool full() const noexcept {
      return m_size == num_elements_v;
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return !m_size;
    }

    J_RETURNS_NONNULL static pool_page * allocate_page(
      pool_page * previous J_PAGE_ALIGNED,
      pool_page * next J_PAGE_ALIGNED
    ) J_RETURNS_PAGE_ALIGNED {
      vmem::page_range rng = vmem::page_pool::instance.allocate(1U);
      void * addr = rng.address();
      J_ASSERT(rng.size() == J_PAGE_SIZE, "Page size mismatch.");
      auto result = ::new (addr) pool_page(previous, next);
      rng.reset_no_free();
      return result;
    }

    J_RETURNS_NONNULL static pool_page * page_of(void * J_NOT_NULL ptr) noexcept J_RETURNS_PAGE_ALIGNED {
      return reinterpret_cast<pool_page*>(align_down(ptr, J_PAGE_SIZE));
    }

    void free(void * J_NOT_NULL ptr) noexcept {
      assert_valid();
      J_ASSERT_RANGE(region_start(), reinterpret_cast<u8_t*>(ptr), region_end());
      const u16_t idx = (reinterpret_cast<u8_t*>(ptr) - region_start()) / ElementSize;
      J_ASSERT((reinterpret_cast<u8_t*>(ptr) - region_start()) % ElementSize == 0,
               "Index not aligned.");
      J_ASSERT(!m_bitset.at(idx), "Freed a pointer that was not allocated.");
      m_bitset.set(idx);
      J_ASSUME(m_size != 0);
      --m_size;
    }

    void detach() noexcept {
      m_next->m_previous = m_previous;
      m_previous->m_next = m_next;
    }

    pool_page * destroy() noexcept {
      assert_valid();
      pool_page * result = nullptr;
      if (m_next != this) {
        result = m_next;
        detach();
      }
      vmem::page_pool::instance.release(this, J_PAGE_SIZE);
      return result;
    }

    J_INLINE_GETTER_NONNULL pool_page * next() const noexcept J_RETURNS_PAGE_ALIGNED {
      assert_valid();
      return m_next;
    }

    J_INLINE_GETTER_NONNULL pool_page * previous() const noexcept J_RETURNS_PAGE_ALIGNED {
      assert_valid();
      return m_previous;
    }

  private:
    pool_page(
      pool_page * previous J_PAGE_ALIGNED,
      pool_page * next J_PAGE_ALIGNED
    ) noexcept
      : m_next(!next ? this : next),
        m_previous(!previous ? this : previous)
    {
      m_previous->m_next = m_next->m_previous = this;
      m_bitset.set_first_n(num_elements_v);
    }

    bits::static_bitset<num_elements_v> m_bitset;

    pool_page * m_next;
    pool_page * m_previous;
    u32_t m_size = 0U;
  };

  template<typename T>
  class pool final {
  public:
    using page_t = pool_page<sizeof(T)>;

    J_ALWAYS_INLINE constexpr pool() noexcept = default;

    J_ALWAYS_INLINE constexpr pool(pool && rhs) noexcept
      : m_first_page(rhs.m_first_page),
        m_last_page(rhs.m_last_page)
    {
      rhs.m_first_page = rhs.m_last_page = nullptr;
    }

    constexpr pool & operator=(pool && rhs) noexcept {
      if (&rhs != this) {
        reset();
        m_first_page = rhs.m_first_page;
        m_last_page = rhs.m_last_page;
        rhs.m_first_page = rhs.m_last_page = nullptr;
      }
      return *this;
    }

    void reset() noexcept {
      for (page_t * p = m_first_page; p; p = p->destroy()) { }
      m_first_page = m_last_page = nullptr;
    }

    ~pool() noexcept {
      reset();
    }

    template<typename... Args>
    J_RETURNS_NONNULL T * allocate(Args && ... args) {
      if (!m_first_page || m_first_page->full()) {
        m_first_page = m_last_page = page_t::allocate_page(m_first_page, m_last_page);
      }
      void * result = m_first_page->allocate();
      if (m_first_page->full()) {
        m_last_page = m_first_page;
        m_first_page = m_first_page->next();
      }
      assert_valid();
      return ::new (result) T(static_cast<Args &&>(args)...);
    }

    void free(T * J_NOT_NULL t) noexcept {
      page_t * page = page_t::page_of(t);
      page->free(t);
      if (m_first_page != m_last_page) {
        if (page->empty()) {
          page_t * next = page->destroy();
          if (page == m_first_page) {
            m_first_page = next;
          }
          if (page == m_last_page) {
            m_last_page = next ? next->previous() : nullptr;
          }
        } else if (page != m_first_page) {
          if (page == m_last_page) {
            m_first_page = page;
            m_last_page = page->previous();
          } else {
            page->move_before(m_first_page);
            m_first_page = page;
          }
        }
      }
      assert_valid();
    }

    private:
      page_t * m_first_page = nullptr;
      page_t * m_last_page = nullptr;

      void assert_valid() const {
#ifndef NDEBUG
        J_ASSERT(!m_first_page == !m_last_page, "Pages are all or nothing.");
        if (!m_first_page) {
          return;
        }
        page_t * p = m_first_page;
        do {
          J_ASSERT_NOT_NULL(p);
          p->assert_valid();
          p = p->next();
        } while (p != m_last_page);
#endif
      }
    };
  }
