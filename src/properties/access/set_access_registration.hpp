#pragma once

#include "properties/access/set_access.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/default_set_iterator_functions.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "exceptions/assert_lite.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/metadata.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "containers/pair.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  template<typename Container, typename IteratorFunctions = default_set_iterator_functions<Container>>
  struct J_TYPE_HIDDEN set_access_wrapper : IteratorFunctions {
    using item_type J_NO_DEBUG_TYPE = typename Container::key_type;

    J_HIDDEN J_NO_DEBUG inline static const typed_access_definition * m_key_access_definition = nullptr;
    J_HIDDEN J_NO_DEBUG inline static const wrappers::wrapper_definition * m_value_wrapper_definition = nullptr;

    J_HIDDEN J_ALWAYS_INLINE static sz_t size(const Container & c) noexcept;

    J_HIDDEN J_ALWAYS_INLINE static bool empty(const Container & c) noexcept;

    J_HIDDEN J_ALWAYS_INLINE static void clear(Container & c);

    J_HIDDEN J_ALWAYS_INLINE static bool contains(Container & c, const item_type & item);

    J_HIDDEN J_ALWAYS_INLINE static pair<iterator_data, bool> insert(Container & c, const item_type & item);

    J_HIDDEN J_ALWAYS_INLINE static sz_t erase_by_item(Container & c, const item_type & item);
  };

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE sz_t set_access_wrapper<Container, IteratorFunctions>::size(const Container & c) noexcept {
    return c.size();
  }

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE bool set_access_wrapper<Container, IteratorFunctions>::empty(const Container & c) noexcept {
    return c.empty();
  }

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE void set_access_wrapper<Container, IteratorFunctions>::clear(Container & c) {
    c.clear();
  }

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE bool set_access_wrapper<Container, IteratorFunctions>::contains(Container & c, const item_type & item) {
    return c.find(item) != c.end();
  }

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE pair<iterator_data, bool> set_access_wrapper<Container, IteratorFunctions>::insert(Container & c, const item_type & item) {
    auto && result = c.emplace(item);
    return {
      IteratorFunctions::wrap_iterator(static_cast<typename Container::iterator &&>(result.first)),
      result.second
    };
  }

  template<typename Container, typename IteratorFunctions>
  J_HIDDEN J_ALWAYS_INLINE sz_t set_access_wrapper<Container, IteratorFunctions>::erase_by_item(Container & c, const item_type & item) {
    return c.erase(item);
  }

  template<typename Type,
           typename Access = set_access_wrapper<Type>,
           typename ItemType = typename Type::key_type>
  class J_TYPE_HIDDEN set_access_registration {
  public:
    J_HIDDEN J_NO_DEBUG inline static set_access_definition definition;
    J_HIDDEN J_NO_DEBUG inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::SET> wrapper;
  private:
    typed_access_registration m_registration;

    template<auto Fn, typename... Args>
    J_HIDDEN static decltype(auto) with_set(void* c, Args... args);

    J_HIDDEN static sz_t size(const void * set) noexcept {
      return Access::size(*reinterpret_cast<const Type*>(set));
    }

    J_HIDDEN static bool empty(const void * set) noexcept {
      return Access::empty(*reinterpret_cast<const Type*>(set));
    }

    template<auto Fn>
    J_HIDDEN static decltype(auto) with_item(void * c, const typed_access & item);

    template<auto Fn, typename... Args>
    J_HIDDEN static wrappers::wrapper with_item_result(Args... args);

    J_HIDDEN static void set_item_available(const wrappers::wrapper_definition * wrapper, void*);

  public:
    template<typename... Metadata>
    J_HIDDEN explicit set_access_registration(const char * name, Metadata && ... metadata);
  };

  template<typename Type, typename Access, typename ItemType>
  template<auto Fn, typename... Args>
  J_HIDDEN decltype(auto) set_access_registration<Type, Access, ItemType>::with_set(void* c, Args... args) {
    return Fn(*reinterpret_cast<Type *>(c), args...);
  }

  template<typename Type, typename Access, typename ItemType>
  template<auto Fn>
  J_HIDDEN decltype(auto) set_access_registration<Type, Access, ItemType>::with_item(void * c, const typed_access & item) {
    J_ASSERT_NOT_NULL(c);
    Type & container = *reinterpret_cast<Type*>(c);
    auto & item_def = definition.item_definition();
    auto & input_item_def = item.definition();
    if (item_def.can_contain(&input_item_def)) {
      return Fn(
        container,
        *reinterpret_cast<const ItemType*>(item.as_void_star()));
    } else {
      auto converted_value{item_def.convert_from(item)};
      return Fn(
        container,
        *reinterpret_cast<const ItemType*>(converted_value.as_void_star()));
    }
  }

  template<typename Type, typename Access, typename ItemType>
  template<auto Fn, typename... Args>
  J_HIDDEN wrappers::wrapper set_access_registration<Type, Access, ItemType>::with_item_result(Args... args) {
    if constexpr (j::is_ref_v<decltype(Fn(args...))>) {
      return {definition.item_definition(), const_cast<ItemType*>(&Fn(args...))};
    } else {
      ItemType item = Fn(args...);
      return {properties::detail::wrapper_copy_tag_t{}, definition.item_definition(), &item};
    }
  }

  template<typename Type, typename Access, typename ItemType>
  J_HIDDEN void set_access_registration<Type, Access, ItemType>::set_item_available(const wrappers::wrapper_definition * wrapper, void*) {
    definition.set_item_definition(wrapper);
  }

  template<typename Type, typename Access, typename ItemType>
  template<typename... Metadata>
  J_HIDDEN set_access_registration<Type, Access, ItemType>::set_access_registration(
    const char * name,
    Metadata && ... metadata
  ) {
    constexpr sz_t num_conversions = attributes::count<Metadata...>(convertible_from, convertible_to);
    if constexpr (sizeof...(Metadata) != 0) {
      if constexpr (num_conversions != 0) {
        convertible_from.maybe_apply([](const auto & ... convs) {
          auto & conversions = definition.conversions();
          (conversions.add_conversion_from(convs.from, convs.conversion), ...);
        }, static_cast<Metadata &&>(metadata)...);
      }
      constexpr bool has_value_metadata = j::attributes::has<Metadata...>(value_metadata);
      if constexpr (has_value_metadata) {
        definition.item_metadata() = static_cast<attributes::attribute_map &&>(
          value_metadata.get(static_cast<Metadata &&>(metadata)...));
      }
      if (num_conversions + (has_value_metadata ? 1 : 0) != sizeof...(Metadata)) {
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from, value_metadata),
        static_cast<Metadata &&>(metadata)...
      );
    }
  }
  initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::SET, &wrapper, name);

  definition.initialize_set_access(
    &size,
    &empty,
    &with_item<&Access::contains>,

    &with_set<&Access::begin>,
    &with_set<&Access::end>,
    &with_item<&Access::find>,

    &with_item_result<&Access::get_iterator_item, iterator_data &>,

      Access::release_iterator,
      Access::advance_iterator,
      Access::copy_iterator,
      Access::iterator_equals,

      &with_item<&Access::insert>,

      &with_item<&Access::erase_by_item>,
      &with_set<&Access::erase_by_iterator, const iterator_data &>,
      &with_set<&Access::clear>);

    m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
    if constexpr (num_conversions != 0) {
      convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
    }

    registry::wait_for_wrapper(type_id::type_id(typeid(Type)), type_id::type_id(typeid(ItemType)), &set_item_available, nullptr);
  }
}
