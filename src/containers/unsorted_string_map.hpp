#pragma once

#include "containers/unsorted_string_map_fwd.hpp"
#include "strings/string.hpp"
#include "exceptions/assert_lite.hpp"
#include "hzd/type_traits.hpp"
#include "hzd/mem.hpp"
#include "containers/common.hpp"

namespace j::detail {

  [[nodiscard]] J_RETURNS_NONNULL inline strings::string * string_list_find_ptr(const strings::string * J_NOT_NULL J_RESTRICT strings,
                                                                                strings::const_string_view key) noexcept {
    for (; !strings->is_not_a_string() && *strings != key; ++strings);
    return const_cast<strings::string*>(strings);
  }

  [[nodiscard]] J_RETURNS_NONNULL inline strings::string * string_list_find_ptr(const strings::string * J_NOT_NULL J_RESTRICT strings,
                                                                                const strings::string & J_RESTRICT key) noexcept {
    for (; !strings->is_not_a_string() && *strings != key; ++strings);
    return const_cast<strings::string*>(strings);
  }


  [[nodiscard]] inline u32_t string_list_find(const strings::string * J_NOT_NULL J_RESTRICT strings,
                                              const strings::string & J_RESTRICT key) noexcept {
    return string_list_find_ptr(strings, key) - strings;
  }

  [[nodiscard]] inline u32_t string_list_find(const strings::string * J_NOT_NULL J_RESTRICT strings,
                                              strings::const_string_view key) noexcept {
    return string_list_find_ptr(strings, key) - strings;
  }

  strings::string * allocate_strings(u32_t size, u32_t value_size) J_RETURNS_NONNULL;
  strings::string * reallocate_strings(strings::string * J_NOT_NULL ptr, u32_t old_size, u32_t new_size, u32_t value_size) J_RETURNS_NONNULL;
  strings::string * allocate_and_copy_strings(const strings::string * J_NOT_NULL ptr, u32_t old_size, u32_t new_size, u32_t value_size) J_RETURNS_NONNULL;
  void free_strings(strings::string * J_NOT_NULL strings, u32_t size) noexcept;
}

namespace j::inline containers {
  template<typename Value, bool IsCopyable>
  unsorted_string_map<Value, IsCopyable> & unsorted_string_map<Value, IsCopyable>::operator=(unsorted_string_map && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      clear();
      m_data = rhs.m_data, m_size = rhs.m_size, m_capacity = rhs.m_capacity;
      rhs.m_data = const_cast<strings::string*>(&detail::string_sentinel), rhs.m_size = rhs.m_capacity = 0;
    }
    return *this;
  }

  template<typename Value, bool IsCopyable>
  J_INLINE_GETTER_NONNULL Value * unsorted_string_map<Value, IsCopyable>::values_ptr() const noexcept {
    return const_cast<Value*>(reinterpret_cast<const Value*>(m_data + m_capacity + 1U));
  }

  template<typename Value, bool IsCopyable>
  Value * unsorted_string_map<Value, IsCopyable>::maybe_at(const strings::string & J_RESTRICT key) noexcept {
    const u32_t index = detail::string_list_find(m_data, key);
    return index == m_size ? nullptr : values_ptr() + index;
  }

  template<typename Value, bool IsCopyable>
  Value & unsorted_string_map<Value, IsCopyable>::operator[](const strings::string & J_RESTRICT key) {
    const u32_t index = detail::string_list_find(m_data, key);
    if (index == m_size) {
      grow();
      ::new (m_data + index) strings::string(key);
      return *::new (values_ptr() + index) Value();
    } else {
      return values_ptr()[index];
    }
  }

  template<typename Value, bool IsCopyable>
  bool unsorted_string_map<Value, IsCopyable>::contains(const strings::string & J_RESTRICT key) const noexcept {
    return !detail::string_list_find_ptr(m_data, key)->is_not_a_string();
  }

  template<typename Value, bool IsCopyable>
  typename unsorted_string_map<Value, IsCopyable>::iterator unsorted_string_map<Value, IsCopyable>::begin() noexcept {
    return iterator{ m_data, values_ptr() };
  }

  template<typename Value, bool IsCopyable>
  typename unsorted_string_map<Value, IsCopyable>::iterator unsorted_string_map<Value, IsCopyable>::end() noexcept {
    return iterator{ m_data + m_size,  values_ptr() + m_size };
  }

  template<typename Value, bool IsCopyable>
  typename unsorted_string_map<Value, IsCopyable>::iterator unsorted_string_map<Value, IsCopyable>::find(const strings::string & J_RESTRICT key) noexcept {
    strings::string * const str = detail::string_list_find_ptr(m_data, key);
    return iterator{str, values_ptr() + (str - m_data)};
  }

  template<typename Value, bool IsCopyable>
  typename unsorted_string_map<Value, IsCopyable>::iterator unsorted_string_map<Value, IsCopyable>::find(strings::const_string_view key) noexcept {
    strings::string * const str = detail::string_list_find_ptr(m_data, key);
    return iterator{str, values_ptr() + (str - m_data)};
  }

  template<typename Value, bool IsCopyable>
  void unsorted_string_map<Value, IsCopyable>::do_erase(u32_t offset) noexcept {
    J_ASSUME(m_size > offset);
    ::j::memmove(m_data + offset, m_data + offset + 1U, (m_size-- - offset) * sizeof(strings::string));
    Value * const vals = values_ptr();
    if constexpr (j::is_nicely_copyable_v<Value>) {
      vals[offset].~Value();
      if (m_size != offset) {
        ::j::memmove(vals + offset, vals + offset + 1U, (m_size - offset - 1U) * sizeof(Value));
      }
    } else {
      for (; offset < m_size; ++offset) {
        vals[offset] = static_cast<Value &&>(vals[offset + 1U]);
      }
      vals[offset].~Value();
    }
  }

  template<typename Value, bool IsCopyable>
  u32_t unsorted_string_map<Value, IsCopyable>::erase(const strings::string & J_RESTRICT key) noexcept {
    const u32_t offset = detail::string_list_find(m_data, key);
    if (J_UNLIKELY(offset == m_size)) {
      return 0U;
    }
    do_erase(offset);
    return 1U;
  }

  template<typename Value, bool IsCopyable>
  typename unsorted_string_map<Value, IsCopyable>::iterator unsorted_string_map<Value, IsCopyable>::erase(const const_iterator & J_RESTRICT it) noexcept {
    do_erase(it.first - m_data);
    return reinterpret_cast<const iterator &>(it);
  }

  template<typename Value, bool IsCopyable>
  iterator_pair<typename unsorted_string_map<Value, IsCopyable>::iterator> unsorted_string_map<Value, IsCopyable>::start_emplace(const strings::string & J_RESTRICT key) {
    const u32_t offset = detail::string_list_find(m_data, key);
    const bool did_insert = offset == m_size;
    if (J_LIKELY(did_insert)) {
      grow();
    }
    return {iterator{m_data + offset, values_ptr() + offset}, did_insert};
  }

  template<typename Value, bool IsCopyable>
  iterator_pair<typename unsorted_string_map<Value, IsCopyable>::iterator>
  unsorted_string_map<Value, IsCopyable>::start_emplace(strings::const_string_view key) {
    const u32_t offset = detail::string_list_find(m_data, key);
    const bool did_insert = offset == m_size;
    if (J_LIKELY(did_insert)) {
      grow();
    }
    return {iterator{m_data + offset, values_ptr() + offset}, did_insert};
  }

  template<typename Value, bool IsCopyable>
  void unsorted_string_map<Value, IsCopyable>::reserve(u32_t capacity) {
    if (J_UNLIKELY(capacity <= m_capacity)) {
      return;
    }
    if (m_capacity == 0) {
      m_data = reinterpret_cast<strings::string*>(detail::allocate_strings(capacity, sizeof(Value)));
    } else if constexpr (j::is_nicely_copyable_v<Value>) {
      m_data = detail::reallocate_strings(m_data, m_capacity, capacity, sizeof(Value));
    } else {
      strings::string * new_values = detail::allocate_and_copy_strings(m_data, m_capacity, capacity, sizeof(Value));
      Value * dst = reinterpret_cast<Value*>(new_values + capacity + 1U);
      Value * src = values_ptr();
      for (u32_t i = 0U; i < m_size; ++i) {
        ::new (dst++) Value(static_cast<Value &&>(*src));
        (src++)->~Value();
      }
      ::j::free(m_data);
      m_data = new_values;
    }
    m_capacity = capacity;
  }

  template<typename Value, bool IsCopyable>
  void unsorted_string_map<Value, IsCopyable>::clear() noexcept {
    if (m_capacity) {
      if constexpr (!j::is_trivially_copyable_v<Value>) {
        Value * val = values_ptr();
        for (u32_t i = 0U, sz = m_size; i < sz; ++i, ++val) {
          val->~Value();
        }
      }
      detail::free_strings(m_data, m_size);
      m_data = const_cast<strings::string*>(&detail::string_sentinel);
      m_size = 0U;
      m_capacity = 0U;
    }
  }

  template<typename Value>
  void unsorted_string_map<Value, true>::copy_initialize(const unsorted_string_map & J_RESTRICT rhs) {
    unsorted_string_map<Value, false>::m_size = rhs.m_size;
    unsorted_string_map<Value, false>::m_capacity = rhs.m_size;
    if (unsorted_string_map<Value, false>::m_size) {
      unsorted_string_map<Value, false>::m_data = detail::allocate_and_copy_strings(rhs.m_data, rhs.m_capacity, rhs.m_size, sizeof(Value));
      Value * values = unsorted_string_map<Value, false>::values_ptr();
      const Value * src_values = rhs.values_ptr();
      if constexpr (j::is_trivially_copyable_v<Value>) {
        ::j::memcpy(values, src_values, rhs.m_size * sizeof(Value));
      } else {
        for (u32_t i = 0U; i < rhs.m_size; ++i) {
          ::new (values++) Value(*src_values++);
        }
      }
    } else {
      unsorted_string_map<Value, false>::m_data = const_cast<strings::string*>(&detail::string_sentinel);
    }
  }

  template<typename Value, bool IsCopyable>
  void unsorted_string_map<Value, IsCopyable>::grow() {
    reserve(m_capacity + (m_capacity < 4U ? 4U : (m_capacity > 16U ? 16U : m_capacity)));
    ++m_size;
  }

  template<typename Value, bool IsCopyable>
  bool operator==(const unsorted_string_map<Value, IsCopyable> & lhs, const unsorted_string_map<Value, IsCopyable> & rhs) noexcept {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (auto & v : lhs) {
      if (!rhs.contains(*v.first)) {
        return false;
      }
    }
    return true;
  }
}

#define J_DEFINE_EXTERN_UNSORTED_STRING_MAP(VALUE)                                                       \
  template class j::containers::unsorted_string_map<VALUE, false>

#define J_DEFINE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(VALUE)                                                       \
  template class j::containers::unsorted_string_map<VALUE, false>; \
  template class j::containers::unsorted_string_map<VALUE, true>
