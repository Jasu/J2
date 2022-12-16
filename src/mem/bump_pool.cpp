#include "mem/bump_pool.hpp"
#include "backtrace/dump_backtrace.hpp"
#include "mem/page_pool.hpp"

namespace j::mem {
  bump_pool::bump_pool() noexcept { }

  static bool do_grow(bump_pool_node * J_NOT_NULL n, u32_t min_sz) {
    const u32_t pages = page_align_up(min_sz - n->size_left) / J_PAGE_SIZE;
    void * result = vmem::page_pool::instance.grow(n, n->size + n->size_left + sizeof(bump_pool_node), pages, false);
    if (!result) {
      return false;
    }
    J_ASSERT(n == result);
    n->size_left += J_PAGE_SIZE;
    return true;
  }
  bool bump_pool::empty() const noexcept
  { return !m_top || (!m_top->size && !m_top->size_before); }

  bump_pool_node::bump_pool_node(u16_t size, u32_t size_before, bump_pool_node * next) noexcept
    : size_left(size - sizeof(bump_pool_node)),
      size_before(size_before),
      next(next)
  { }

  [[nodiscard]] J_RETURNS_NONNULL void * bump_pool::reserve(u32_t sz) {
    J_ASSUME(sz > 0);
    if (J_UNLIKELY(!m_top || m_top->size_left < sz)) {
      if (!m_top || J_UNLIKELY(!do_grow(m_top, sz))) {
        const u32_t pages = page_align_up(max(sz + sizeof(bump_pool_node), 2 * J_PAGE_SIZE)) / J_PAGE_SIZE;
        vmem::page_range r = vmem::page_pool::instance.allocate(pages);
        auto * const n = ::new (r.address()) bump_pool_node(r.size(), size(), m_top);
        if (m_top) {
          J_ASSUME(m_top->previous == nullptr);
          m_top->previous = n;
        } else {
          m_bottom = n;
        }
        m_top = n;
        r.reset_no_free();
      }
      J_ASSUME_NOT_NULL(m_top);
      J_ASSUME(m_top->size_left >= sz);
    }
    return m_top->end<void>();
  }

  J_RETURNS_NONNULL void * bump_pool::allocate(u32_t sz) {
    void * const result = reserve(sz);
    m_top->size += sz;
    m_top->size_left -= sz;
    return result;
  }

  J_RETURNS_NONNULL void * bump_pool::allocate_aligned_zero(u32_t sz, u32_t align) {
    this->align(align);
    void * result = allocate(sz);
    ::j::memzero(result, sz);
    return result;
  }

  [[nodiscard]] bump_pool_checkpoint bump_pool::checkpoint() const noexcept {
    return {
      .node = m_top,
      .size = m_top ? m_top->size : 0U,
    };

  }

  [[nodiscard]] const void * bump_pool::ptr_of(u32_t index) const noexcept {
    const bump_pool_node * n = m_top;
    J_ASSUME_NOT_NULL(n);
    while (n->size_before > index) {
      n = n->next;
      J_ASSUME_NOT_NULL(n);
    }
    return add_bytes(n->begin<const void>(), index - n->size_before);
  }

  [[nodiscard]] u32_t bump_pool::index_of(const void * J_NOT_NULL ptr) const noexcept {
    const bump_pool_node * n = m_top;
    J_ASSUME_NOT_NULL(n);
    while (n->begin<void>() > ptr || n->end<void>() <= ptr) {
      n = n->next;
      J_ASSUME_NOT_NULL(n);
    }
    return n->size_before + byte_offset(n->begin<void>(), ptr);
  }

  void bump_pool::align(u32_t sz) {
    J_ASSUME(sz > 0);
    J_ASSERT(!((sz - 1) & sz));
    if (const u32_t slack = -(sizeof(bump_pool_node) + (m_top ? m_top->size : 0)) & (sz - 1)) {
      j::memzero(allocate(slack), slack);
    }
  }

  void bump_pool::pop_nodes_until(bump_pool_node * target) noexcept {
    bump_pool_node * node = m_top;
    J_ASSUME(target != node);
    while (node != target) {
      J_ASSUME_NOT_NULL(node);
      bump_pool_node * const next = node->next;
      vmem::page_pool::instance.release(node, node->size + node->size_left + sizeof(bump_pool_node));
      node = next;
    }
    m_top = node;
    if (!node || !node->previous) {
      m_bottom = node;
    } else {
      node->previous = nullptr;
    }
  }

  void bump_pool::clear() noexcept {
    bump_pool_node * node = m_top;
    while (node) {
      bump_pool_node * const next = node->next;
      vmem::page_pool::instance.release(node, node->size + node->size_left + sizeof(bump_pool_node));
      node = next;
    }
    m_top = nullptr;
    m_bottom = nullptr;
  }

  void bump_pool::rewind_to_checkpoint(bump_pool_checkpoint point) {
    if (m_top != point.node) {
      pop_nodes_until(point.node);
    }
    if (point.node) {
      J_ASSUME_NOT_NULL(m_top);
      J_ASSUME(m_top->size >= point.size);
      const u32_t size_diff = m_top->size - point.size;
      m_top->size_left += size_diff;
      m_top->size = point.size;
    }
  }

  bump_pool::~bump_pool() {
    clear();
  }

  strings::const_string_view bump_pool::write_string(strings::const_string_view str) {
    const i32_t sz = str.size();
    if (!sz) {
      return {};
    }
    void * ptr = allocate(sz);
    ::j::memcpy(ptr, str.data(), sz);
    return {(const char*)ptr, sz};
  }
}
