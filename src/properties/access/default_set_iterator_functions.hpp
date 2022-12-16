#pragma once

#include "exceptions/assert_lite.hpp"
#include "properties/access/iterator_data.hpp"
#include "hzd/type_traits.hpp"

namespace j::properties::access {
  template<typename Set, typename Enable = void>
  class J_TYPE_HIDDEN default_set_iterator_functions {
    using iterator_t J_NO_DEBUG_TYPE = typename Set::iterator;
    using item_t J_NO_DEBUG_TYPE = typename Set::key_type;
  public:
    J_ALWAYS_INLINE static iterator_t & iter(iterator_data & it) {
      iterator_t * ptr = it.as<iterator_t*>();
      J_ASSERT_NOT_NULL(ptr);
      return *ptr;
    }

    J_ALWAYS_INLINE static const iterator_t & iter(const iterator_data & it) {
      const iterator_t * ptr = it.as<const iterator_t*>();
      J_ASSERT_NOT_NULL(ptr);
      return *ptr;
    }

    static iterator_data wrap_iterator(const iterator_t & it) {
      return iterator_data(new iterator_t(it));
    }

    J_ALWAYS_INLINE static iterator_data begin(Set & m) {
      return wrap_iterator(m.begin());
    }

    J_ALWAYS_INLINE static iterator_data end(Set & m) {
      return wrap_iterator(m.end());
    }

    J_ALWAYS_INLINE static iterator_data find(Set & m, const item_t & key) {
      return wrap_iterator(m.find(key));
    }

    J_ALWAYS_INLINE static decltype(auto) get_iterator_item(iterator_data & it) {
      return *iter(it);
    }

    J_ALWAYS_INLINE static void release_iterator(iterator_data & it) noexcept {
      delete it.as<iterator_t*>();
    }

    J_ALWAYS_INLINE static void advance_iterator(iterator_data & it) {
      ++iter(it);
    }

    J_ALWAYS_INLINE static iterator_data copy_iterator(const iterator_data & src) {
      return wrap_iterator(iter(src));
    }

    J_ALWAYS_INLINE static bool iterator_equals(const iterator_data & lhs, const iterator_data & rhs) {
      return iter(lhs) == iter(rhs);
    }

    J_ALWAYS_INLINE static iterator_data erase_by_iterator(Set & m, const iterator_data & it) {
      return wrap_iterator(m.erase(iter(it)));
    }
  };

  template<typename Set>
  class J_HIDDEN default_set_iterator_functions<
    Set,
    j::enable_if_t<sizeof(typename Set::iterator) <= 2 * sizeof(uptr_t) && j::is_nicely_copyable_v<typename Set::iterator>>
  > {
    using iterator_t J_NO_DEBUG_TYPE = typename Set::iterator;
    using item_t J_NO_DEBUG_TYPE = typename Set::key_type;
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

    J_ALWAYS_INLINE static iterator_data begin(Set & m) {
      return iterator_data(m.begin());
    }

    J_ALWAYS_INLINE static iterator_data end(Set & m) {
      return iterator_data(m.end());
    }

    J_ALWAYS_INLINE static iterator_data find(Set & m, const item_t & key) {
      return iterator_data(m.find(key));
    }

    J_ALWAYS_INLINE static decltype(auto) get_iterator_item(iterator_data & it) {
      return *iter(it);
    }

    J_NO_DEBUG inline static const null_t release_iterator = nullptr;

    J_ALWAYS_INLINE static void advance_iterator(iterator_data & it) {
      ++iter(it);
    }

    J_ALWAYS_INLINE static iterator_data copy_iterator(const iterator_data & src) {
      return src;
    }

    J_ALWAYS_INLINE static bool iterator_equals(const iterator_data & lhs, const iterator_data & rhs) {
      return iter(lhs) == iter(rhs);
    }

    J_ALWAYS_INLINE static iterator_data erase_by_iterator(Set & m, const iterator_data & it) {
      return iterator_data(m.erase(iter(it)));
    }
  };
}
