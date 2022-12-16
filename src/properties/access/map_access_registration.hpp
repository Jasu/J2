#pragma once

#include "properties/access/map_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/default_map_iterator_functions.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "exceptions/assert_lite.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/metadata.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  /// Default access for containers satisfying the AssociativeContainer concept.
  template<typename Container, typename IteratorFunctions = default_map_iterator_functions<Container>>
  struct J_TYPE_HIDDEN associative_container_access_wrapper : IteratorFunctions {
    using key_type J_NO_DEBUG_TYPE = typename Container::key_type;
    using value_type J_NO_DEBUG_TYPE = typename Container::mapped_type;

    inline static const typed_access_definition * m_key_access_definition = nullptr;
    inline static const wrappers::wrapper_definition * m_value_wrapper_definition = nullptr;

    J_HIDDEN J_ALWAYS_INLINE static sz_t size(const Container & c) noexcept {
      return c.size();
    }

    J_HIDDEN J_ALWAYS_INLINE static bool empty(const Container & c) noexcept {
      return c.empty();
    }

    J_HIDDEN J_ALWAYS_INLINE static void clear(Container & c) {
      c.clear();
    }

    J_HIDDEN J_ALWAYS_INLINE static decltype(auto) at(Container & c, const key_type & key) {
      return c.at(key);
    }

    J_HIDDEN J_ALWAYS_INLINE static pair<iterator_data, bool> insert(
      Container & c,
      const key_type & key,
      const value_type & value)
    {
      auto && result = c.emplace(key, value);
      return {
        IteratorFunctions::wrap_iterator(static_cast<typename Container::iterator &&>(result.first)),
        result.second};
    }

    J_HIDDEN J_ALWAYS_INLINE static iterator_data replace(Container & c, const key_type & key, const value_type & value) {
      auto result = c.emplace(key, value);
      if (!result.second) {
        result.first->second = value;
      }
      return IteratorFunctions::wrap_iterator(static_cast<typename Container::iterator &&>(result.first));
    }

    J_HIDDEN J_ALWAYS_INLINE static sz_t erase_by_key(Container & c, const key_type & key) {
      return c.erase(key);
    }
  };

  template<typename Type,
           typename Access = associative_container_access_wrapper<Type>,
           typename KeyType = typename Type::key_type,
           typename ValueType = typename Type::mapped_type>
  class J_TYPE_HIDDEN map_access_registration {
  public:
    inline static map_access_definition definition;
    inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::MAP> wrapper;
  private:
    typed_access_registration m_registration;

    template<auto Fn, typename... Args>
    static decltype(auto) with_map(void* c, Args... args) {
      return Fn(*reinterpret_cast<Type *>(c), args...);
    }

    static sz_t size(const void * m) noexcept {
      return Access::size(*reinterpret_cast<const Type *>(m));
    }

    static bool empty(const void * m) noexcept {
      return Access::empty(*reinterpret_cast<const Type *>(m));
    }

    template<auto Fn, typename... Args>
    J_HIDDEN static decltype(auto) with_key(void * c, const typed_access & key, Args... args) {
      J_ASSERT_NOT_NULL(c);
      auto & key_def = definition.key_definition();
      if (&key.definition() == &key_def) {
        return Fn(*reinterpret_cast<Type*>(c), *reinterpret_cast<const KeyType*>(key.as_void_star()), args...);
      } else {
        auto converted_key{key_def.create_from(key)};
        return Fn(*reinterpret_cast<Type*>(c), *reinterpret_cast<const KeyType*>(converted_key.as_void_star()), args...);
      }
    }

    template<auto Fn, auto FnWithKey = &with_key<Fn, const ValueType &>>
    J_HIDDEN static decltype(auto) wrap_insert(void * c, const typed_access & key, const typed_access & value) {
      auto & value_def = definition.value_definition();
      if (value_def.can_contain(&value.definition())) {
        auto ptr = value_def.create(value);
        return FnWithKey(c, key, *reinterpret_cast<const ValueType*>(ptr.as_void_star()));
      } else {
        auto converted_value{value_def.convert_from(value)};
        auto val = converted_value.get_value();
        auto ptr = value_def.create(val);
        return FnWithKey(c, key, *reinterpret_cast<const ValueType*>(ptr.as_void_star()));
      }
    }

    template<auto Fn, typename... Args>
    J_HIDDEN static wrappers::wrapper with_value_result(Args... args) {
      if constexpr (j::is_same_v<wrappers::wrapper &, decltype(Fn(declval<Args>()...)) &>) {
        return Fn(static_cast<Args &&>(args)...);
      } else {
        return wrappers::wrapper{definition.value_definition(), &Fn(static_cast<Args &&>(args)...)};
      }
    }

    J_HIDDEN static typed_access get_iterator_key(iterator_data & it) {
      return typed_access(
        definition.key_definition(),
        const_cast<void*>(reinterpret_cast<const void *>(&Access::get_iterator_key(it))));
    }

    J_HIDDEN static void map_key_available(const typed_access_definition * key_access, void*) {
      definition.set_key_definition(key_access);
      Access::m_key_access_definition = key_access;

    }

    J_HIDDEN static void map_value_available(const wrappers::wrapper_definition * wrapper, void*) {
      definition.set_value_definition(wrapper);
      Access::m_value_wrapper_definition = wrapper;
    }

  public:
    template<typename... Metadata>
    J_HIDDEN explicit map_access_registration(const char * name, Metadata && ... metadata) {
      if constexpr (sizeof...(Metadata) != 0) {
        convertible_from.maybe_apply([](const auto & ... convs) {
          auto & conversions = definition.conversions();
          (conversions.add_conversion_from(convs.from, convs.conversion), ...);
        }, static_cast<Metadata &&>(metadata)...);
        if constexpr (j::attributes::has<Metadata...>(key_metadata)) {
          definition.key_metadata() = static_cast<attributes::attribute_map &&>(
            key_metadata.get(static_cast<Metadata &&>(metadata)...));
        }
        if constexpr (j::attributes::has<Metadata...>(value_metadata)) {
          definition.value_metadata() = static_cast<attributes::attribute_map &&>(
            value_metadata.get(static_cast<Metadata &&>(metadata)...));
        }
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from, key_metadata, value_metadata),
          static_cast<Metadata &&>(metadata)...
        );
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::MAP, &wrapper, name);

      definition.initialize_map_access(
        &size,
        &empty,
        &with_value_result<&with_key<&Access::at>>,

        &with_map<&Access::begin>,
        &with_map<&Access::end>,
        &with_key<&Access::find>,

        &get_iterator_key,
        &with_value_result<&Access::get_iterator_value>,

        Access::release_iterator,
        Access::advance_iterator,
        Access::copy_iterator,
        Access::iterator_equals,

        &wrap_insert<&Access::insert>,
        &wrap_insert<&Access::replace>,

        &with_key<&Access::erase_by_key>,
        &with_map<&Access::erase_by_iterator, const iterator_data &>,
        &with_map<&Access::clear>);

      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (sizeof...(Metadata) != 0) {
        convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
      }

      registry::wait_for_wrapper(type_id::type_id(typeid(Type)), type_id::type_id(typeid(ValueType)), &map_value_available, nullptr);
      registry::wait_for_access(type_id::type_id(typeid(Type)), type_id::type_id(typeid(KeyType)), &map_key_available, nullptr);
    }
  };
}
