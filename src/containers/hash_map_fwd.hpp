#pragma once

#include "containers/hash.hpp"
#include "containers/pair.hpp"
#include "containers/common.hpp"
#include "exceptions/assert.hpp"
#include "hzd/algorithm_base.hpp"
#include "bits/bitops.hpp"

namespace j::detail {
  template<typename Key, typename Value>
  struct hash_map_node final {
    u32_t hash = 0U;
    Key first;
    Value second;
  };

  template<typename Key>
  struct hash_map_node<Key, void> final {
    u32_t hash = 0U;
    Key first;
  };

  inline const char * fast_forward(const void * n_, u32_t sz) noexcept {
    const char * n = (const char*)n_;
    if (n) {
      while (!*(const u32_t*)n) { n += sz; }
    }
    return n;
  }

  void move_hash_map(void * J_NOT_NULL J_RESTRICT to, void * J_NOT_NULL J_RESTRICT from) noexcept;

  void copy_hash_map_trivial(void * J_NOT_NULL J_RESTRICT to_, const void * J_NOT_NULL J_RESTRICT from_, u32_t node_size) noexcept;

  J_A(RNN,ALIGN(16),MALLOC,NODISC) void * allocate_hash_map(i32_t capacity, u32_t node_size) noexcept;

  J_A(AI,ND,NODISC,HIDDEN) inline i8_t probe_count(u32_t hash, u8_t inv_capacity_shift) noexcept {
    return ~hash >> inv_capacity_shift;
  }

  template<NonRefNonArrayNotTriviallyCopyable T>
  J_A(HIDDEN,AI,ND) T && arg_fwd_helper(T &&) noexcept;

  template<NonArrayNonScalar T>
  J_A(HIDDEN,AI,ND) const T & arg_fwd_helper(const T &) noexcept;

  template<typename T>
  J_A(HIDDEN,AI,ND) T arg_fwd_helper(T) noexcept;

  template<typename T>
  using arg_fwd_t J_NO_DEBUG_TYPE = decltype(j::detail::arg_fwd_helper(declval<T>()));
}

namespace j::inline containers {
  struct hash_map_iterator_base {
    const char * m_node = nullptr;

    J_A(NODISC) inline explicit operator bool() const noexcept {
      return m_node && *(const u32_t*)m_node != 1U;
    }

    J_A(AI,NODISC) inline bool operator==(const hash_map_iterator_base &) const noexcept = default;
  };

  template<typename Key, typename Value>
  struct hash_map_iterator final : hash_map_iterator_base {
    J_A(AI,HIDDEN) inline hash_map_iterator & operator++() noexcept {
      constexpr u32_t sz = sizeof(detail::hash_map_node<Key, Value>);
      m_node = detail::fast_forward(m_node + sz, sz);
      return *this;
    }

    J_A(AI,HIDDEN) inline hash_map_iterator operator++(int) noexcept {
      const char * n = m_node;
      operator++();
      return {{n}};
    }

    J_A(AI,NODISC,RNN) inline auto * operator->() const noexcept {
      if constexpr (is_void_v<Value>) {
        return &((detail::hash_map_node<Key, Value>*)(m_node))->first;
      } else {
        return (detail::hash_map_node<Key, Value>*)(m_node);
      }
    }

    J_A(AI,NODISC,ND,HIDDEN) inline auto & operator*() const noexcept {
      return *operator->();
    }

    J_A(AI,NODISC,ND,HIDDEN) inline operator hash_map_iterator<const Key, const_if_t<!is_void_v<Value>, Value>> () const noexcept
    { return {{m_node}}; }
  };

  template<typename Key>
  struct default_const_key final {
    using type J_NO_DEBUG_TYPE = Key;
    using arg_type J_NO_DEBUG_TYPE = const Key &;
    using getter_type J_NO_DEBUG_TYPE = Key;
  };

  template<Scalarish Key>
  struct default_const_key<Key> final {
    using type J_NO_DEBUG_TYPE = Key;
    using arg_type J_NO_DEBUG_TYPE = Key;
    using getter_type J_NO_DEBUG_TYPE = Key;
  };

  template<typename T, typename ConstKey>
  concept HasGetterTypeConv = requires (const T & t, const ConstKey & ck) { ck(t); };

  template<typename T, typename ConstKey>
  concept NoGetterTypeConv = !HasGetterTypeConv<T, ConstKey>;

  template<typename T>
  concept WithoutGetterTypeConvs = requires { T::no_getter_type_conversion_v; } && T::no_getter_type_conversion_v;


  template<typename Key, typename Value, typename Hash = hash<Key>, typename KeyEq = equal_to<Key>, typename ConstKey = default_const_key<Key>>
  class hash_map {
  public:
  private:
    using node_t = detail::hash_map_node<Key, Value>;
  public:
    using getter_key_t = typename ConstKey::getter_type;

    J_A(ND) hash_map() noexcept = default;

    explicit hash_map(i32_t capacity) noexcept;

    J_A(ND,AI,NE,HIDDEN) inline hash_map(hash_map && rhs) noexcept {
      detail::move_hash_map(this, &rhs);
    }

    J_A(AI,NE,ND,HIDDEN) inline hash_map(const hash_map & rhs) noexcept { copy_from(rhs); }

    J_A(AI,NE,ND,HIDDEN) inline hash_map & operator=(hash_map && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        clear();
        detail::move_hash_map(this, &rhs);
      }
      return *this;
    }

    J_A(AI,NE,ND,HIDDEN) inline hash_map & operator=(const hash_map & rhs) {
      if (J_LIKELY(this != &rhs)) {
        clear();
        copy_from(rhs);
      }
      return *this;
    }

    template<HasGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(AI,NODISC,NE,HIDDEN) inline bool contains(const K & key) const noexcept
    { return find_impl(m_const_key(key)); }

    template<NoGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(AI,NODISC,NE,HIDDEN) inline bool contains(const K & key) const noexcept
    { return find_impl(key); }


    template<HasGetterTypeConv<ConstKey> K = getter_key_t>
    [[nodiscard]] auto * maybe_at(const K & key) noexcept {
      node_t * it = find_impl(m_const_key(key));
      if constexpr (is_void_v<Value>) {
        return it ? &it->first : nullptr;
      } else {
        return it ? &it->second : nullptr;
      }
    }

    template<NoGetterTypeConv<ConstKey> K = getter_key_t>
    [[nodiscard]] auto * maybe_at(const K & key) noexcept {
      node_t * it = find_impl(key);
      if constexpr (is_void_v<Value>) {
        return it ? &it->first : nullptr;
      } else {
        return it ? &it->second : nullptr;
      }
    }

    template<typename K>
    J_A(AI,NE,ND,HIDDEN,NODISC) inline const auto * maybe_at(const K & key) const noexcept {
      return const_cast<hash_map*>(this)->maybe_at<K>(key);
    }

    template<HasGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(NODISC) auto & at(const K & key) {
      if (node_t * it = find_impl(m_const_key(key))) {
        if constexpr (is_void_v<Value>) {
          return it->first;
        } else {
          return it->second;
        }
      }
      exceptions::throw_out_of_range(0, key);
    }

    template<NoGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(NODISC) auto & at(const K & key) {
      if (node_t * it = find_impl(key)) {
        if constexpr (is_void_v<Value>) {
          return it->first;
        } else {
          return it->second;
        }
      }
      exceptions::throw_out_of_range(0, key);
    }

    template<typename K = getter_key_t>
    J_A(NODISC,AI,NE,HIDDEN) inline const auto & at(const K& key) const {
      return const_cast<hash_map*>(this)->at<K>(key);
    }

    template<HasGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(AI,ND,HIDDEN,NE) inline auto find(const K & key) noexcept {
      node_t * it = find_impl(m_const_key(key));
      return hash_map_iterator<Key, Value>{{(const char*)(it ? it : m_data + m_capacity)}};
    }

    template<NoGetterTypeConv<ConstKey> K = getter_key_t>
    J_A(AI,ND,HIDDEN,NE) inline auto find(const K & key) noexcept {
      node_t * it = find_impl(key);
      return hash_map_iterator<Key, Value>{{(const char*)(it ? it : m_data + m_capacity)}};
    }

    template<typename K = getter_key_t>
    J_A(AI,ND,HIDDEN,NE) inline auto find(const K & key) const noexcept {
      return const_cast<hash_map*>(this)->find<K>(key);
    }

    template<typename K = const getter_key_t &>
    J_A(AI,HIDDEN,NE,ND) inline auto & operator[](K && key) {
      if constexpr (is_void_v<Value>) {
        return do_emplace<detail::arg_fwd_t<K>>(static_cast<K &&>(key)).first->first;
      } else {
        return do_emplace<detail::arg_fwd_t<K>>(static_cast<K &&>(key)).first->second;
      }
    }

    template<NoGetterTypeConv<ConstKey> K>
    J_A(ND) auto do_emplace(K key) requires is_void_v<Value> {
      auto pair = prepare_emplace(key);
      if (pair.second) {
        ::new (pair.first.operator->()) Key(static_cast<K &&>(key));
      }
      return pair;
    }

    template<NoGetterTypeConv<ConstKey> K, typename... Vs>
    J_A(ND) auto do_emplace(K key, Vs ... vs) requires (!is_void_v<Value>) {
      auto pair = prepare_emplace(key);
      if (pair.second) {
        ::new (&pair.first->first) Key(static_cast<K &&>(key));
        ::new (&pair.first->second) Value(static_cast<Vs &&>(vs)...);
      }
      return pair;
    }

    template<HasGetterTypeConv<ConstKey> K>
    J_A(ND) auto do_emplace(K key) requires is_void_v<Value> {
      auto pair = prepare_emplace(m_const_key(key));
      if (pair.second) {
        ::new (pair.first.operator->()) Key(static_cast<K &&>(key));
      }
      return pair;
    }

    template<HasGetterTypeConv<ConstKey> K, typename... Vs>
    J_A(ND) auto do_emplace(K key, Vs ... vs) requires (!is_void_v<Value>) {
      auto pair = prepare_emplace(m_const_key(key));
      if (pair.second) {
        ::new (&pair.first->first) Key(static_cast<K &&>(key));
        ::new (&pair.first->second) Value(static_cast<Vs &&>(vs)...);
      }
      return pair;
    }

    template<typename... Ts>
    J_A(AI,ND,NE,HIDDEN) inline auto emplace(Ts && ... ts) requires(!is_void_v<Value>) {
      return do_emplace<detail::arg_fwd_t<Ts>...>(static_cast<Ts &&>(ts)...);
    }

    template<typename T>
    J_A(AI,ND,NE,HIDDEN) inline auto emplace(T && t) requires is_void_v<Value> {
      return do_emplace<detail::arg_fwd_t<T>>(static_cast<T &&>(t));
    }

    template<typename K = Key, typename V = Value>
    J_A(AI,ND,NE,HIDDEN) inline auto insert(K && key, V && value) requires(!is_void_v<Value>) {
      return do_emplace<detail::arg_fwd_t<K>, detail::arg_fwd_t<V>>(static_cast<K &&>(key), static_cast<V &&>(value));
    }

    template<typename T = Key>
    J_A(AI,ND,NE,HIDDEN) inline auto insert(T && key) {
      return do_emplace<detail::arg_fwd_t<T>>(static_cast<T &&>(key));
    }

    J_A(AI,NE,HIDDEN) inline auto erase(hash_map_iterator<const Key, const_if_t<!is_void_v<Value>, Value>> it) noexcept {
      node_t * target = (node_t*)(it.m_node);
      node_t * cur = target + 1;
      i32_t i = 0, offset = 0;
      while (i <= m_max_probe_count && cur->hash) {
        if (cur->hash == 1U) {
          cur = m_data;
          continue;
        }
        if (detail::probe_count(cur->hash, m_inv_capacity_shift) <= offset) {
          ++offset;
          ++i, cur = ++cur;
          continue;
        }
        *target = static_cast<node_t &&>(*cur);
        target->hash += (1+offset) << m_inv_capacity_shift;
        target = cur;
        i = offset + 1;
        cur = ++cur;
      }
      target->~node_t();
      target->hash = 0U;
      --m_size;
      return hash_map_iterator<Key, Value>{{detail::fast_forward(it.m_node, sizeof(node_t))}};
    }

    void clear() noexcept;


    J_A(AI,NE,HIDDEN,NODISC) inline auto begin() noexcept {
      return hash_map_iterator<Key, Value>{{ detail::fast_forward(m_data, sizeof(node_t)) }};
    }

    J_A(AI,NE,HIDDEN,NODISC) inline auto begin() const noexcept {
      return hash_map_iterator<const Key, const_if_t<!is_void_v<Value>, Value>>{{ detail::fast_forward(m_data, sizeof(node_t)) }};
    }

    J_A(AI,NE,HIDDEN,NODISC) inline auto end() noexcept
    { return hash_map_iterator<Key, Value>{{ (const char *)(m_data + m_capacity) }}; }

    J_A(AI,NE,HIDDEN,NODISC) inline auto end() const noexcept
    { return hash_map_iterator<const Key, const_if_t<!is_void_v<Value>, Value>>{{ (const char *)(m_data + m_capacity) }}; }

    J_A(AI,HIDDEN,NE,ND,NODISC) inline bool operator!() const noexcept         { return !m_size; }
    J_A(AI,HIDDEN,NE,ND,NODISC) inline explicit operator bool() const noexcept { return m_size; }
    J_A(AI,HIDDEN,NE,ND,NODISC) inline bool empty() const noexcept             { return !m_size; }
    J_A(AI,HIDDEN,NE,ND,NODISC) inline i32_t size() const noexcept             { return m_size; }

    J_A(AI,HIDDEN,NE,ND,NODISC) inline i32_t capacity() const noexcept { return m_capacity; }

    J_A(AI,NE,HIDDEN) inline ~hash_map() { clear(); }
  private:
    [[nodiscard]] node_t * find_impl(typename ConstKey::arg_type key) const noexcept;

    [[nodiscard]] iterator_pair<hash_map_iterator<Key, Value>> prepare_emplace(typename ConstKey::arg_type key);

    void rehash(u8_t capacity_shift);

    J_A(AI,NE,HIDDEN) inline void copy_from(const hash_map & J_RESTRICT rhs) noexcept {
      if constexpr (is_trivially_copyable_v<node_t>) {
        detail::copy_hash_map_trivial(this, &rhs, sizeof(node_t));
      } else {
        J_MEMCPY_INLINE(this, &rhs, sizeof(hash_map));
        if (m_data) {
          node_t * J_AV(NOALIAS) new_data = (node_t*)detail::allocate_hash_map(m_capacity, sizeof(node_t));
          m_data = new_data;
          for (node_t * J_AV(NOALIAS) old_data = rhs.m_data; old_data->hash != 1U; ++old_data, ++new_data) {
            if (old_data->hash) {
              ::new (new_data) node_t(*old_data);
            }
          }
        }
      }
    }

    void insert_move(u32_t hash, node_t * J_NOT_NULL to, node_t * J_NOT_NULL from);

    node_t * m_data = nullptr;
    i32_t m_capacity = 0U;
    i32_t m_size = 0U;
    u32_t m_probe_one = 0U;
    u8_t m_inv_capacity_shift = 0U;
    i8_t m_max_probe_count = 0U;

    [[no_unique_address]] J_HIDDEN Hash m_hash;
    [[no_unique_address]] J_HIDDEN KeyEq m_key_eq;
    [[no_unique_address]] J_HIDDEN ConstKey m_const_key;
  };

  template<typename Key, typename Hash = hash<Key>, typename KeyEq = equal_to<Key>, typename ConstKey = default_const_key<Key>>
  using hash_set = hash_map<Key, void, Hash, KeyEq, ConstKey>;
}

#define J_HASH_MAP_NODE(KEY, VALUE) j::detail::hash_map_node<KEY, VALUE>
#define J_HASH_MAP_ITER(KEY, VALUE) j::hash_map_iterator<KEY, VALUE>

#define J_HASH_MAP(KEY, VALUE, ...) \
  j::hash_map<KEY, VALUE \
              __VA_OPT__(, J_PREFIX_FILTER(J_HASH_MAP_HASH_EXPAND_, J_FIRST, __VA_ARGS__, HASH(j::hash<KEY>)), \
                         J_PREFIX_FILTER(J_HASH_MAP_KEYEQ_EXPAND_, J_FIRST, __VA_ARGS__, KEYEQ(j::equal_to<KEY>)), \
                         J_PREFIX_FILTER(J_HASH_MAP_CONST_KEY_EXPAND_, J_FIRST, __VA_ARGS__, DEFAULT_CONST_KEY(KEY)))>


// Mechanism tp pass multiple arguments to children from outer macros.
// Only a partial macro `ARGS` is passed:
// `ARGS(ARG(j::lisp::foo::bar), ARG(j::lisp::foo))`
// `ARGS` and `ARG` are not defined, so the macro does not spontaneously
// expand.
// When an `ARG`ument is needed from the `ARGS(...)` wrapper, the `ARGS(...)`
// is passed to the `J_HM_ARG` macro with the name of the variable needed.
// The expansion steps happen as follows:
//
//   0. `J_HM_ARG(HASHMAP, ARGS(ARG(j::hash_map<>) ARG(foo)))`
//   1. `J_HM_EXTRACT_ ## HASHMAP ## _ ## ARGS(ARG(j::hash_map<>) ARG(foo))`
//   2, `J_HM_EXTRACT_HASHMAP_ARGS(ARG(j::hash_map<>) ARG(foo))`
//   3. `J_HM_EXTRACT_ ## ARG(j::hash_map<>)`
//   4. `J_HM_EXTRACT_ARG(j::hash_map<>)`
//   5. `j::hash_map<>`
//
// This case would also work without two layers of wrappers (`ARGS` and `ARG`)
// but some common cases fail. If the argument contains commas not wrapped in
// parenthesis (e.g. template parameter separators wrapped in angle brackets),
// the preprocessor handles it as multiple tokens.
#define J_HM_EXTRACT_ARG(...) __VA_ARGS__
#define J_HM_EXTRA_ARGS(...) J_HM_ARGS_##__VA_ARGS__
#define J_HM_ARG(NAME, ARGS) J_HM_EXTRACT_ ## NAME ## _ ## ARGS

#define J_HM_EXTRACT_HASHMAP_ARGS(A, ...) J_HM_EXTRACT_##A
#define J_HM_EXTRACT_DECLARATION_ARGS(A, B, ...) J_HM_EXTRACT_##B
#define J_HM_EXTRACT_KEY_ARGS(A, B, C, ...) J_HM_EXTRACT_##C
#define J_HM_EXTRACT_VALUE_ARGS(A, B, C, D, ...) J_HM_EXTRACT_##D
#define J_HM_EXTRACT_ITERATOR_ARGS(A, B, C, D, E, ...) J_HM_EXTRACT_##E



#define J_HM_APPLY_EXTRA(ARGS, EXTRA) J_HM_EXTRA_##EXTRA(ARGS, J_HM_EXTRA_ARGS_##EXTRA)

// Macros that merely eat their arguments, replacing FIND(foo::key)
// with `J_HM_FIND`.  `J_HM_FIND` etc. are the actual macros that
// expand to C++. Note that the macro expands to the name only. not a
// function-like token.  `J_HM_APPLY_EXTRA` has set things up so that
// the `J_HM_FIND` will be directly followed by parenthesized arguments.
// The setup is needed to access user-provided arugments, and to yet
// allow the macro to pass the its own `ARGS` argument to each macro
// given by user.
#define J_HM_EXTRA_EMPLACE(...) J_HM_EMPLACE
// Those arguments that are used within the main template parameters expand to J_HM_DISCARD,
// which will expand to nothing.
#define J_HM_EXTRA_KEYEQ(...) J_HM_DISCARD
#define J_HM_EXTRA_HASH(...) J_HM_DISCARD
#define J_HM_EXTRA_CONST_KEY(...) J_HM_DISCARD
#define J_HM_EXTRA_DEFAULT_CONST_KEY(...) J_HM_DISCARD
#define J_HM_DISCARD(...)

// These unwrap the user-provided macro arguments, e.g:
// 0. `J_HM_EXTRA_ARGS_ ## FIND(foo::key)`
// 1. `J_HM_EXTRA_ARGS_FIND(foo::key)`
// 2. `foo::key`
#define J_HM_EXTRA_ARGS_EMPLACE(...) __VA_ARGS__


#define J_HM_EMPLACE(ARGS, ...) ; J_HM_ARG(DECLARATION, ARGS) \
  auto J_HM_ARG(HASHMAP, ARGS)::emplace(__VA_ARGS__)

#define J_HASH_MAP_HASH_EXPAND_HASH(V) V,
#define J_HASH_MAP_HASH_EXPAND_KEYEQ(...)
#define J_HASH_MAP_HASH_EXPAND_DEFAULT_CONST_KEY(...)
#define J_HASH_MAP_HASH_EXPAND_CONST_KEY(...)
#define J_HASH_MAP_HASH_EXPAND_EMPLACE(...)

#define J_HASH_MAP_KEYEQ_EXPAND_KEYEQ(V) V,
#define J_HASH_MAP_KEYEQ_EXPAND_HASH(...)
#define J_HASH_MAP_KEYEQ_EXPAND_CONST_KEY(...)
#define J_HASH_MAP_KEYEQ_EXPAND_DEFAULT_CONST_KEY(...)
#define J_HASH_MAP_KEYEQ_EXPAND_EMPLACE(...)

#define J_HASH_MAP_CONST_KEY_EXPAND_CONST_KEY(K) K,
#define J_HASH_MAP_CONST_KEY_EXPAND_DEFAULT_CONST_KEY(K) j::default_const_key<K>,
#define J_HASH_MAP_CONST_KEY_EXPAND_KEYEQ(...)
#define J_HASH_MAP_CONST_KEY_EXPAND_HASH(...)
#define J_HASH_MAP_CONST_KEY_EXPAND_EMPLACE(...)


#define J_DECLARE_EXTERN_HASH_MAP(KEY, VALUE, ...)                      \
  extern template class J_HASH_MAP(KEY, VALUE __VA_OPT__(, __VA_ARGS__)) \
  J_FE_PREARG(J_HM_APPLY_EXTRA,                                         \
              ARGS(ARG(J_HASH_MAP(KEY, VALUE __VA_OPT__(, __VA_ARGS__))), \
                   ARG(extern template), ARG(KEY), ARG(VALUE),          \
                   ARG(J_HASH_MAP_ITER(KEY, VALUE)))                   \
              __VA_OPT__(, __VA_ARGS__))

#define J_DECLARE_EXTERN_HASH_SET(KEY, ...) J_DECLARE_EXTERN_HASH_MAP(KEY, void __VA_OPT__(,) __VA_ARGS__)
