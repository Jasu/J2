#include "hash_map.hpp"

namespace j::detail {
  namespace {
    struct J_TYPE_HIDDEN hash_map_data final {
      void * m_data;
      i32_t m_capacity;
      i32_t m_size;
      u32_t m_probe_one;
      u8_t m_inv_capacity_shift;
      i8_t m_max_probe_count;
    };
  }

  void move_hash_map(void * J_NOT_NULL J_RESTRICT to, void * J_NOT_NULL J_RESTRICT from) noexcept {
    ::j::memcpy(to, from, sizeof(hash_map_data));
    ::j::memzero(from, sizeof(hash_map_data));
  }

  J_A(RNN,ALIGN(16),MALLOC,NODISC) void * allocate_hash_map(i32_t capacity, u32_t node_size) noexcept {
    void * result = ::j::allocate_zero((capacity + 1U) * node_size);
    *add_bytes<u32_t*>(result, capacity * node_size) = 1U;
    return result;
  }

  void copy_hash_map_trivial(void * J_NOT_NULL J_RESTRICT to_, const void * J_NOT_NULL J_RESTRICT from_, u32_t node_size) noexcept {
    hash_map_data * to = (hash_map_data *)to_;
    const hash_map_data * from = (const hash_map_data *)from_;
    ::j::memcpy(to, from, sizeof(hash_map_data));
    if (from->m_data) {
      i32_t alloc_size = (from->m_capacity + 1U) * node_size;
      to->m_data = ::j::allocate(alloc_size);
      j::memcpy(to->m_data, from->m_data, alloc_size);
    }
  }
}
