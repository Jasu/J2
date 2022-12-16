#pragma once

#include "properties/access/iterator_data.hpp"
#include "hzd/type_traits.hpp"

namespace j::properties::access {
  template<typename Map, typename Enable = void>
  class J_TYPE_HIDDEN default_map_iterator_functions {
    using iterator_t J_NO_DEBUG_TYPE = typename Map::iterator;
    using key_t J_NO_DEBUG_TYPE = typename Map::key_type;
    using value_t J_NO_DEBUG_TYPE = typename Map::mapped_type;
  public:
    J_ALWAYS_INLINE static iterator_t & iter(iterator_data & it) noexcept {
      return *it.as<iterator_t*>();
    }

    J_ALWAYS_INLINE static const iterator_t & iter(const iterator_data & it) noexcept {
      return iter(const_cast<iterator_data &>(it));
    }

    J_NO_INLINE static iterator_data wrap_iterator(const iterator_t & it) {
      return iterator_data(new iterator_t(it));
    }

    J_ALWAYS_INLINE static iterator_data begin(Map & m) {
      return wrap_iterator(m.begin());
    }

    J_ALWAYS_INLINE static iterator_data end(Map & m) {
      return wrap_iterator(m.end());
    }

    J_ALWAYS_INLINE static iterator_data find(Map & m, const key_t & key) {
      return wrap_iterator(m.find(key));
    }

    J_ALWAYS_INLINE static const key_t & get_iterator_key(iterator_data & it) noexcept {
      return iter(it)->first;
    }

    J_ALWAYS_INLINE static value_t & get_iterator_value(iterator_data & it) noexcept {
      return iter(it)->second;
    }

    J_ALWAYS_INLINE static void release_iterator(iterator_data & it) noexcept {
      delete it.as<iterator_t*>();
    }

    J_ALWAYS_INLINE static void advance_iterator(iterator_data & it) noexcept {
      ++iter(it);
    }

    static iterator_data copy_iterator(const iterator_data & src) {
      return wrap_iterator(iter(src));
    }

    J_ALWAYS_INLINE static bool iterator_equals(const iterator_data & lhs, const iterator_data & rhs) noexcept {
      return iter(lhs) == iter(rhs);
    }

    static iterator_data erase_by_iterator(Map & m, const iterator_data & it) {
      return wrap_iterator(m.erase(iter(it)));
    }
  };

  template<typename Map>
  class J_HIDDEN default_map_iterator_functions<
    Map,
    j::enable_if_t<sizeof(typename Map::iterator) <= 2 * sizeof(uptr_t) && j::is_nicely_copyable_v<typename Map::iterator>>
  > {
    using iterator_t = typename Map::iterator;
    using key_t = typename Map::key_type;
    using value_t = typename Map::mapped_type;
  public:
    J_ALWAYS_INLINE static iterator_t & iter(iterator_data & it) noexcept {
      return it.as<iterator_t>();
    }

    J_ALWAYS_INLINE static const iterator_t & iter(const iterator_data & it) noexcept {
      return it.as<iterator_t>();
    }

    J_ALWAYS_INLINE static iterator_data wrap_iterator(const iterator_t & it) noexcept {
      return iterator_data(it);
    }

    J_ALWAYS_INLINE static iterator_data begin(Map & m) noexcept {
      return iterator_data(m.begin());
    }

    J_ALWAYS_INLINE static iterator_data end(Map & m) noexcept {
      return iterator_data(m.end());
    }

    J_ALWAYS_INLINE static iterator_data find(Map & m, const key_t & key) {
      return iterator_data(m.find(key));
    }

    J_ALWAYS_INLINE static const key_t & get_iterator_key(iterator_data & it) noexcept {
      return iter(it)->first;
    }

    J_ALWAYS_INLINE static value_t & get_iterator_value(iterator_data & it) noexcept {
      return iter(it)->second;
    }

    inline static const null_t release_iterator = nullptr;

    J_ALWAYS_INLINE static void advance_iterator(iterator_data & it) noexcept {
      ++iter(it);
    }

    J_ALWAYS_INLINE static iterator_data copy_iterator(const iterator_data & src) noexcept {
      return src;
    }

    J_ALWAYS_INLINE static bool iterator_equals(const iterator_data & lhs, const iterator_data & rhs) noexcept {
      return iter(lhs) == iter(rhs);
    }

    J_ALWAYS_INLINE static iterator_data erase_by_iterator(Map & m, const iterator_data & it) {
      return iterator_data(m.erase(iter(it)));
    }
  };
}
