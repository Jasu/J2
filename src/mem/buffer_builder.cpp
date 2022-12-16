#include "buffer_builder.hpp"

namespace j::mem {
  void buffer_builder::commit(char * J_NOT_NULL end) noexcept {
    char * current = (char*)m_pool.current();
    J_ASSUME_NOT_NULL(current);
    if (end == current) {
      return;
    }
    J_ASSUME(end > current);
    m_pool.allocate(end - current);
  }

  void * buffer_builder::append(const void * J_NOT_NULL src, u32_t sz) {
    J_ASSUME(sz > 0);
    void * result = allocate(sz);
    ::j::memcpy(result, src, sz);
    return result;
  }

  void buffer_builder::copy_to(void * J_NOT_NULL to) const noexcept {
    char * ptr = (char*)to;
    u32_t sz = m_pool.size();
    for (auto & p : m_pool) {
      J_ASSUME_NOT_NULL(ptr);
      ::j::memcpy(ptr, p.begin<void>(), min(sz, p.size));
      ptr += p.size;
      sz -= p.size;
    }
  }

  [[nodiscard]] buffer buffer_builder::build() const {
    if (!m_pool.size()) {
      return buffer{};
    }
    buffer result(allocate_tag, m_pool.size());
    copy_to(result.begin());
    return result;
  }
}
