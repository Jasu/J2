#pragma once

#include "containers/hash_map_fwd.hpp"

namespace j::detail {
  [[maybe_unused]] static constexpr inline u32_t max_load_factor_v = 0xCCCCCCCCU; // (1U << 32) / 5U * 4U;
}

namespace j::inline containers {
  template<typename K, typename V, typename H, typename Eq, typename Ck>
  hash_map<K, V, H, Eq, Ck>::hash_map(i32_t capacity) noexcept {
    if (capacity) {
      u32_t capacity_shift = bits::clz(capacity | 4U);
      m_inv_capacity_shift = capacity_shift;
      m_capacity = 1 << (32 - capacity_shift);
      m_probe_one = 1U << capacity_shift;
      m_data = (node_t*)detail::allocate_hash_map(m_capacity, sizeof(node_t));
    }
  }
  template<typename K, typename V, typename H, typename Eq, typename Ck>
  detail::hash_map_node<K, V> * hash_map<K, V, H, Eq, Ck>::find_impl(typename Ck::arg_type key) const noexcept {
    if (!m_data) {
      return nullptr;
    }
    u32_t hash = m_hash(key);
    node_t * J_AV(NOALIAS) it = const_cast<node_t*>(m_data) + (hash >> m_inv_capacity_shift);
    hash |= -m_probe_one;
    i8_t probe_count = m_max_probe_count;
    while (probe_count-- >= 0) {
      if (it->hash == hash && m_key_eq(it->first, key)) {
        return it;
      }
      hash -= m_probe_one;
      ++it;
      if (it->hash == 1U) {
        it = m_data;
      }
    }
    return nullptr;
  }

  template<typename K, typename V, typename H, typename Eq, typename Ck>
  void hash_map<K, V, H, Eq, Ck>::clear() noexcept {
    if (!m_data) {
      return;
    }
    if constexpr (!is_trivially_destructible_v<node_t>) {
      for (auto * J_AV(NOALIAS) it = m_data; it->hash != 1U; ++it) {
        if (it->hash) {
          it->~node_t();
        }
      }
    }
    ::j::free(m_data);
    ::j::memzero(this, sizeof(hash_map));
  }


  template<typename K, typename V, typename H, typename Eq, typename Ck>
  [[nodiscard]] auto hash_map<K, V, H, Eq, Ck>::prepare_emplace(typename Ck::arg_type key) -> iterator_pair<hash_map_iterator<K, V>> {
    if (!m_data || (detail::max_load_factor_v <= (u32_t)m_size << m_inv_capacity_shift)) {
      rehash(m_data ? m_inv_capacity_shift - 1U : 28U);
    }

    u32_t hash = m_hash(key);
    node_t * ptr = m_data + (hash >> m_inv_capacity_shift);
    hash |= -m_probe_one;

    while (ptr->hash) {
      if (ptr->hash == 1U) {
        ptr = m_data;
        continue;
      }
      if (ptr->hash > hash) {
        insert_move(ptr->hash, ptr, ptr);
        break;
      } else if (ptr->hash == hash && m_key_eq(ptr->first, key)) {
        return {{{(const char*)ptr}}, false };
      }
      ++ptr;
      hash -= m_probe_one;
    }

    ++m_size;
    ptr->hash = hash;
    hash = ~hash >> m_inv_capacity_shift;
    m_max_probe_count = J_MAX(m_max_probe_count, (i8_t)hash);
    return {{{(const char*)ptr}}, true };
  }

  template<typename K, typename V, typename H, typename Eq, typename Ck>
  void hash_map<K, V, H, Eq, Ck>::insert_move(u32_t hash, node_t * J_NOT_NULL to, node_t * J_NOT_NULL from) {
    while (to->hash) {
      if (to->hash > hash) {
        insert_move(to->hash, to, to);
        break;
      }
      if ((++to)->hash == 1U) {
        to = m_data;
      }
      hash -= m_probe_one;
    }

    if constexpr (is_void_v<V>) {
      ::new (to) node_t{ hash, static_cast<K &&>(from->first) };
    } else {
      ::new (to) node_t{ hash, static_cast<K &&>(from->first), static_cast<V &&>(from->second) };
    }
    from->~node_t();
    hash = ~hash >> m_inv_capacity_shift;
    m_max_probe_count = J_MAX(m_max_probe_count, (i8_t)hash);
  }

  template<typename K, typename V, typename H, typename Eq, typename Ck>
  void hash_map<K, V, H, Eq, Ck>::rehash(u8_t capacity_shift) {
    u32_t old_probe = m_probe_one;
    node_t * old_data = m_data;

    m_capacity = 1 << (32 - capacity_shift);
    m_probe_one = 1U << capacity_shift;
    m_inv_capacity_shift = capacity_shift;
    m_data = (node_t*)detail::allocate_hash_map(m_capacity, sizeof(node_t));
    m_max_probe_count = 0;

    if (old_data) {
      // Loop counter, but incrementing the bit field in hash used as index.
      u32_t index_probe = 0U;
      auto * J_AV(NOALIAS) it = old_data;
      do {
        index_probe += old_probe; // Must be incremented before use, don't move
        if (it->hash) {
          // Recover the complete hash - the upper bits encode the ideal index
          // assuming no collisions, so undo that first.
          // When the instruction is first inserted, the top bits (i.e. those
          // touched by index_probe) are used as the ideal insertion index at
          // the hash table. Then they are all set to one. All-one-bit numbers
          // represent -1, so to get the correct value, the current index + 1
          // must be added to the top bits. To account for the extra one,
          // `index_probe` is incremented at the beginning of this loop, rather
          // than at end.
          insert_move(it->hash | -m_probe_one, m_data + ((it->hash + index_probe) >> m_inv_capacity_shift), it);
        }
        ++it;
      } while (index_probe);
      ::j::free(old_data);
    }
  }
}

#define J_DEFINE_EXTERN_HASH_MAP(KEY, VALUE, ...) \
  template class J_HASH_MAP(KEY, VALUE __VA_OPT__(, __VA_ARGS__)) \
  J_FE_PREARG(J_HM_APPLY_EXTRA, ARGS(ARG(J_HASH_MAP(KEY, VALUE __VA_OPT__(, __VA_ARGS__))), ARG(template), ARG(KEY), ARG(VALUE), ARG(J_HASH_MAP_ITER(KEY, VALUE))) \
              __VA_OPT__(, __VA_ARGS__))

#define J_DEFINE_EXTERN_HASH_SET(KEY, ...) J_DEFINE_EXTERN_HASH_MAP(KEY, void __VA_OPT__(,) __VA_ARGS__)
