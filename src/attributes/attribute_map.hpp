#pragma once

#include "mem/any.hpp"

#include "attributes/map_detail.hpp"
#include "containers/hash_map_fwd.hpp"
#include "containers/span.hpp"
#include "containers/vector.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "type_id/type_hash.hpp"

J_DECLARE_EXTERN_HASH_MAP(j::type_id::type_id, j::noncopyable_vector<j::mem::any>, HASH(j::type_id::hash));

namespace j::attributes {
  namespace detail {
    template<typename Withouts>
    struct J_TYPE_HIDDEN attr_inserter;
  }

  template<typename, typename> class attribute;

  class attribute_map;

  template<typename... AttributeDefinition>
  J_A(AI,ND,NODISC) inline constexpr auto without(const AttributeDefinition & ...) noexcept {
    return detail::without<typename AttributeDefinition::tag_t...>{};
  }

  /// Dynamic container for named arguments.
  class attribute_map final {
  public:
    J_A(AI) attribute_map() noexcept;

    template<typename... Attrs>
    J_A(AI,ND,HIDDEN) attribute_map(Attrs && ... attrs) {
      using ins J_NO_DEBUG_TYPE = detail::attr_inserter<detail::get_withouts_t<Attrs &...>>;
      (ins::insert(this, static_cast<Attrs &&>(attrs)), ...);
    }

    attribute_map(const attribute_map &) = delete;
    attribute_map(attribute_map &) = delete;
    attribute_map(attribute_map &&) noexcept;
    attribute_map & operator=(const attribute_map &) = delete;
    attribute_map & operator=(attribute_map &&) noexcept;

    ~attribute_map();

    bool empty() const noexcept;

    u32_t size() const noexcept;

    auto begin() noexcept;

    auto end() noexcept;

    auto begin() const noexcept;

    auto end() const noexcept;

    void emplace(const std::type_info & type, mem::any && value);

    bool contains_type(const std::type_info & type) const noexcept;
    template<typename T>
    J_A(AI,ND,NODISC) inline bool contains() const noexcept {
      return contains_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline bool contains(const T &) const noexcept {
      return contains_type(typeid(T));
    }

    u32_t count_type(const std::type_info & type) const noexcept;
    template<typename T>
    J_A(AI,ND,NODISC) inline u32_t count() const noexcept {
      return count_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline u32_t count(const T &) const noexcept {
      return count_type(typeid(T));
    }

    [[nodiscard]] mem::any * maybe_first_type(const std::type_info & type) noexcept;
    [[nodiscard]] mem::any const * maybe_first_type(const std::type_info & type) const noexcept;

    template<typename T>
    J_A(AI,ND,NODISC) inline mem::any * maybe_first() noexcept {
      return maybe_first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline mem::any * maybe_first(const T &) noexcept {
      return maybe_first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline const mem::any * maybe_first() const noexcept {
      return maybe_first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline const mem::any * maybe_first(const T &) const noexcept {
      return maybe_first_type(typeid(T));
    }

    [[nodiscard]] mem::any & first_type(const std::type_info & type) noexcept;
    [[nodiscard]] mem::any const & first_type(const std::type_info & type) const noexcept;

    template<typename T>
    J_A(AI,ND,NODISC) inline mem::any & first() noexcept {
      return first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline mem::any & first(const T &) noexcept {
      return first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline const mem::any & first() const noexcept {
      return first_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline const mem::any & first(const T &) const noexcept {
      return first_type(typeid(T));
    }

    span<mem::any> maybe_at_type(const std::type_info & type) noexcept;
    span<const mem::any> maybe_at_type(const std::type_info & type) const noexcept;

    template<typename T>
    J_A(AI,ND,NODISC) inline auto maybe_at() noexcept {
      return maybe_at_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline auto maybe_at(const T &) noexcept {
      return maybe_at_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline auto maybe_at() const noexcept {
      return maybe_at_type(typeid(T));
    }
    template<typename T>
    J_A(AI,ND,NODISC) inline auto maybe_at(const T &) const noexcept {
      return maybe_at_type(typeid(T));
    }

    template<typename T>
    J_INLINE_GETTER decltype(auto) get() const {
      const std::type_info & tid = typeid(T);
      if constexpr (T::is_multiple_v) {
        span<const mem::any> from = maybe_at_type(tid);
        trivial_array<typename T::value_t> result{containers::uninitialized, from.size()};
        for (auto & f : from) {
          if constexpr (T::is_any_v) {
            result.initialize_element(f);
          } else {
            result.initialize_element(f.get_unsafe<typename T::value_t>());
          }
        }
        return result;
      } else {
        const mem::any & v = first_type(tid);
        if constexpr (T::is_any_v) {
          return v;
        } else {
          return v.get<typename T::value_t>();
        }
      }
    }

    template<typename T>
    J_A(AI,ND,NODISC) inline decltype(auto) get(const T &) const {
      return get<T>();
    }

  private:
    hash_map<type_id::type_id, noncopyable_vector<mem::any>, type_id::hash> m_map;
  };

  namespace detail {
    template<typename Withouts>
    struct J_TYPE_HIDDEN attr_inserter;

    template<typename... Withouts>
    struct J_TYPE_HIDDEN attr_inserter<type_list<Withouts...>> {
      template<typename T>
      J_ALWAYS_INLINE_NO_DEBUG static void insert(void *, const T &) noexcept { }

      template<typename AttrDef, typename Value>
      J_A(AI,ND) inline static void insert(attribute_map * J_NOT_NULL map, attribute<AttrDef, Value> && attr) {
        if constexpr ((!__is_same_as(Withouts, typename AttrDef::tag_t) && ...)) {
          map->emplace(typeid(AttrDef), mem::any::create<Value>(static_cast<Value &&>(attr.m_value)));
        }
      }
    };
  }
}
