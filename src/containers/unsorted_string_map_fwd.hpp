#pragma once

#include "hzd/mem.hpp"
#include "strings/string.hpp"
#include "containers/pair.hpp"
#include "containers/span.hpp"

namespace j::strings {
  class string;
}

namespace j::detail {
  J_NO_DEBUG extern const strings::string string_sentinel;
}
namespace j::inline containers {
  template<typename, bool> class unsorted_string_map;

  template<typename Value>
  struct unsorted_string_map_iterator {
    const strings::string * first;
    Value * second;

    J_ALWAYS_INLINE unsorted_string_map_iterator & operator++() noexcept {
      ++first;
      ++second;
      return *this;
    }

    J_ALWAYS_INLINE unsorted_string_map_iterator operator++(int) noexcept {
      return {first++, second++};
    }

    J_INLINE_GETTER unsorted_string_map_iterator<Value> & operator*() noexcept {
      return *this;
    }

    J_INLINE_GETTER_NONNULL unsorted_string_map_iterator<Value> * operator->() noexcept {
      return this;
    }

    J_INLINE_GETTER operator unsorted_string_map_iterator<const Value>() const noexcept {
      return { first, second };
    }

    J_INLINE_GETTER bool operator==(const unsorted_string_map_iterator & rhs) const noexcept = default;
  };

  template<typename Value, bool = __is_constructible(Value, const Value &)>
  class unsorted_string_map {
  public:
    using key_type = strings::string;
    using mapped_type J_NO_DEBUG_TYPE = Value;
    using iterator = unsorted_string_map_iterator<Value>;
    using const_iterator = unsorted_string_map_iterator<const Value>;

    J_ALWAYS_INLINE constexpr unsorted_string_map() noexcept = default;

    J_ALWAYS_INLINE_NO_DEBUG J_NO_EXPLICIT unsorted_string_map(unsorted_string_map && rhs) noexcept
      : m_data(rhs.m_data),
        m_size(rhs.m_size),
        m_capacity(rhs.m_capacity)
    {
      rhs.m_data = const_cast<strings::string*>(&detail::string_sentinel), rhs.m_size = rhs.m_capacity = 0;
    }

    unsorted_string_map & operator=(unsorted_string_map && rhs) noexcept;

    J_ALWAYS_INLINE J_NO_EXPLICIT ~unsorted_string_map() {
      clear();
    }

    [[nodiscard]] bool contains(const strings::string & key) const noexcept;

    J_INLINE_GETTER J_NO_EXPLICIT Value & at(const strings::string & J_RESTRICT key) {
      Value * res = maybe_at(key);
      if (J_UNLIKELY(!res)) {
        exceptions::throw_out_of_range(0, key);
      }
      return *res;
    }

    J_INLINE_GETTER J_NO_EXPLICIT const Value & at(const strings::string & J_RESTRICT key) const {
      return const_cast<unsorted_string_map*>(this)->at(key);
    }

    [[nodiscard]] Value * maybe_at(const strings::string & J_RESTRICT key) noexcept;
    J_INLINE_GETTER const Value * maybe_at(const strings::string & J_RESTRICT key) const noexcept {
      return const_cast<unsorted_string_map*>(this)->maybe_at(key);
    }

    [[nodiscard]] Value & operator[](const strings::string & J_RESTRICT key);

    [[nodiscard]] iterator begin() noexcept;
    J_INLINE_GETTER const_iterator begin() const noexcept {
      return const_cast<unsorted_string_map*>(this)->begin();
    }
    [[nodiscard]] iterator end() noexcept;
    J_INLINE_GETTER const_iterator end() const noexcept {
      return const_cast<unsorted_string_map*>(this)->end();
    }

    [[nodiscard]] iterator find(const strings::string & J_RESTRICT key) noexcept;
    J_INLINE_GETTER J_NO_EXPLICIT iterator find(strings::string & J_RESTRICT key) noexcept {
      return find(static_cast<const strings::string &>(key));
    }
    [[nodiscard]] iterator find(strings::const_string_view key) noexcept;
    J_INLINE_GETTER J_NO_EXPLICIT iterator find(const char * J_NOT_NULL key) noexcept {
      return find(strings::const_string_view(key));
    }

    template<typename T>
    J_INLINE_GETTER J_NO_EXPLICIT const_iterator find(T && key) const noexcept {
      return const_cast<unsorted_string_map*>(this)->find(static_cast<T &&>(key));
    }

    [[nodiscard]] u32_t erase(const strings::string & J_RESTRICT key) noexcept;
    [[nodiscard]] iterator erase(const const_iterator & J_RESTRICT it) noexcept;

    template<typename K, typename T>
    iterator_pair<iterator> insert(K && key, T && J_RESTRICT value) {
      return emplace(static_cast<K &&>(key), static_cast<T &&>(value));
    }

    template<typename Key, typename... Args>
    J_ALWAYS_INLINE iterator_pair<iterator> emplace(Key && key, Args && ... args) {
      iterator_pair<iterator> result = start_emplace(static_cast<Key &&>(key));
      if (J_LIKELY(result.second)) {
        ::new (const_cast<strings::string *>(result.first.first)) strings::string(static_cast<Key &&>(key));
        ::new (result.first.second) Value(static_cast<Args &&>(args)...);
      }
      return result;
    }

    void reserve(u32_t capacity);
    void clear() noexcept;

    J_INLINE_GETTER u32_t size() const noexcept { return m_size; }
    J_INLINE_GETTER u32_t capacity() const noexcept { return m_capacity; }
    J_INLINE_GETTER bool empty() const noexcept { return !m_size; }

    J_INLINE_GETTER J_NO_EXPLICIT auto keys() noexcept {
      return span<strings::string>(m_data, m_size);
    }

    J_INLINE_GETTER J_NO_EXPLICIT auto keys() const noexcept {
      return span<const strings::string>(m_data, m_size);
    }

    J_INLINE_GETTER J_NO_EXPLICIT auto values() noexcept {
      return span<Value>(values_ptr(), m_size);
    }

    J_INLINE_GETTER J_NO_EXPLICIT auto values() const noexcept {
      return span<const Value>(values_ptr(), m_size);
    }
  protected:
    J_INLINE_GETTER_NONNULL Value * values_ptr() const noexcept;
    void grow();
    strings::string * m_data = const_cast<strings::string *>(&detail::string_sentinel);
    u32_t m_size = 0U;
    u32_t m_capacity = 0U;
  private:
    iterator_pair<iterator> start_emplace(const strings::string & J_RESTRICT key);
    iterator_pair<iterator> start_emplace(strings::const_string_view key);
    J_INLINE_GETTER J_NO_EXPLICIT iterator_pair<iterator> start_emplace(const char * J_NOT_NULL key) {
      return start_emplace(strings::const_string_view(key));
    }
    void do_erase(u32_t index) noexcept;
  };

  template<typename Value>
  class unsorted_string_map<Value, true> final : public unsorted_string_map<Value, false> {
  public:
    using typename unsorted_string_map<Value, false>::iterator;
    constexpr unsorted_string_map() noexcept = default;
    unsorted_string_map(unsorted_string_map && rhs) noexcept = default;
    unsorted_string_map & operator=(unsorted_string_map && rhs) noexcept = default;

    J_ALWAYS_INLINE unsorted_string_map(const unsorted_string_map & J_RESTRICT rhs) {
      copy_initialize(rhs);
    }

    J_ALWAYS_INLINE J_NO_EXPLICIT unsorted_string_map & operator=(const unsorted_string_map & J_RESTRICT rhs) {
      if (J_LIKELY(this != &rhs)) {
        unsorted_string_map<Value, false>::clear();
        copy_initialize(rhs);
      }
      return *this;
    }
  private:
    void copy_initialize(const unsorted_string_map & J_RESTRICT rhs);
  };

  template<typename Value>
  using copyable_unsorted_string_map J_NO_DEBUG_TYPE = unsorted_string_map<Value, true>;

  template<typename Value>
  using uncopyable_unsorted_string_map J_NO_DEBUG_TYPE = unsorted_string_map<Value, false>;

  template<typename Value, bool IsCopyable>
  bool operator==(const unsorted_string_map<Value, IsCopyable> & lhs, const unsorted_string_map<Value, IsCopyable> & rhs) noexcept;
}


#define J_DECLARE_EXTERN_UNSORTED_STRING_MAP(VALUE)                                                             \
  extern template class j::containers::unsorted_string_map<VALUE, false>

#define J_DECLARE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(VALUE)                                                             \
  extern template class j::containers::unsorted_string_map<VALUE, false>; \
  extern template class j::containers::unsorted_string_map<VALUE, true>
