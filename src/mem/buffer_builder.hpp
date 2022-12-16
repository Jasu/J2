#pragma once

#include "mem/bump_pool.hpp"
#include "mem/buffer.hpp"

namespace j::mem {
  class buffer_builder {
  public:
    J_RETURNS_NONNULL char * reserve(u32_t sz) {
      J_ASSUME(sz > 0);
      return (char*)m_pool.reserve(sz);
    }

    void commit(char * J_NOT_NULL end) noexcept;

    J_INLINE_GETTER u32_t index_of(const void * J_NOT_NULL ptr) const noexcept
    { return m_pool.index_of(ptr); }

    J_ALWAYS_INLINE_NONNULL char * allocate(u32_t sz) {
      J_ASSUME(sz > 0);
      return (char*)m_pool.allocate(sz);
    }

    J_RETURNS_NONNULL void * append(const void * J_NOT_NULL src, u32_t sz);

    template<Integral T>
    J_ALWAYS_INLINE_NONNULL void * append(T num) {
      return append(&num, sizeof(T));
    }

    template<typename Ptr>
    J_ALWAYS_INLINE_NONNULL void * append(const basic_memory_region<Ptr> & region) {
      return append(region.begin(), region.size());
    }

    J_ALWAYS_INLINE void align(u32_t a) {
      m_pool.align(a);
    }

    J_INLINE_GETTER u32_t size() const noexcept { return m_pool.size(); }

    J_INLINE_GETTER bool empty() const noexcept { return m_pool.empty(); }

    J_ALWAYS_INLINE void clear() noexcept {
      m_pool.clear();
    }

    void copy_to(void * J_NOT_NULL to) const noexcept;

    J_INLINE_GETTER u32_t size_available() const noexcept
    { return m_pool.size_available(); }

    [[nodiscard]] buffer build() const;
  private:
    mem::bump_pool m_pool;
  };
}
