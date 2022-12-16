#pragma once

#include "properties/access/list_access_definition.hpp"
#include "properties/access/get_typed_access_definition.hpp"
#include "properties/access/typed_access_registration.hpp"
#include "exceptions/assert_lite.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/metadata.hpp"
#include "properties/access/lists/list_iterator.hpp"
#include "properties/detail/value_definition_impl.hpp"
#include "properties/wrappers/reference_wrapper_registration.hpp"
#include "attributes/basic_operations.hpp"

namespace j::properties::access {
  /// Wrapper for accesses to types satisfying the SequenceContainer concept.
  template<typename Type>
  struct J_TYPE_HIDDEN sequence_container_access_wrapper {
    using value_t J_NO_DEBUG_TYPE = typename Type::value_type;
    using iterator_t J_NO_DEBUG_TYPE = typename Type::iterator;

    static_assert(j::is_nicely_copyable_v<iterator_t>);

    J_NO_DEBUG inline static const list_access_definition * m_list_definition = nullptr;
    J_NO_DEBUG inline static const wrappers::wrapper_definition * m_item_wrapper_definition = nullptr;

    static list_iterator wrap_iterator(iterator_t it) {
      return list_iterator(m_list_definition, iterator_data(static_cast<iterator_t &&>(it)));
    }

    static sz_t get_size(const Type & c) noexcept {
      return c.size();
    }

    static bool empty(const Type & c) noexcept {
      return c.empty();
    }

    static wrappers::wrapper at(Type & c, sz_t index) {
      J_ASSERT_NOT_NULL(m_item_wrapper_definition);
      return {*m_item_wrapper_definition, &c.at(index)};
    }

    static void push_back(Type & c, const value_t & value) {
      c.emplace_back(value);
    }

    static list_iterator insert(Type & c, const list_iterator & it, const value_t & value) {
      return wrap_iterator(c.insert(it.data().as<iterator_t>(), value));
    }

    static void erase_by_index(Type & c, sz_t index) {
      J_REQUIRE(index < c.size(), "Erased out of bounds.");
      c.erase(c.begin() + index);
    }

    static list_iterator erase_by_iterator(Type & c, const list_iterator & it) {
      return wrap_iterator(c.erase(it.data().as<iterator_t>()));
    }

    static list_iterator begin(Type & c) {
      return wrap_iterator(c.begin());
    }

    static list_iterator end(Type & c) {
      return wrap_iterator(c.end());
    }

    static void clear(Type & c) {
      return c.clear();
    }

    static void advance_iterator(iterator_data & it) noexcept {
      ++it.as<iterator_t>();
    }

    static bool iterator_equals(const iterator_data & lhs, const iterator_data & rhs) noexcept {
      return lhs.as<const iterator_t>() == rhs.as<const iterator_t>();
    }

    static wrappers::wrapper get_iterator_value(const iterator_data & it) noexcept {
      return wrappers::wrapper(*m_item_wrapper_definition, static_cast<void*>(&*it.as<const iterator_t>()));
    }

    J_ALWAYS_INLINE J_HIDDEN static void initialize(const wrappers::wrapper_definition * item_definition) {
      J_ASSERT(!m_item_wrapper_definition, "list access definition was initialized twice.");
      m_item_wrapper_definition = item_definition;
      J_ASSERT_NOT_NULL(m_item_wrapper_definition);
    }
  };

  template<
    typename Type,
    typename Access = sequence_container_access_wrapper<Type>,
    typename ValueType = typename Type::value_type
  >
  class J_TYPE_HIDDEN list_access_registration {
  public:
    inline static list_access_definition definition;
    J_NO_DEBUG inline static wrappers::wrapper_definition list_iterator_wrapper_definition;
    J_NO_DEBUG inline static wrappers::reference_wrapper_definition<Type, &definition, PROPERTY_TYPE::LIST> wrapper;
  private:
    J_HIDDEN typed_access_registration m_registration;

    template<auto Fn, typename... Args>
    J_HIDDEN static decltype(auto) with_list(void * list, Args... args) {
      return Fn(*reinterpret_cast<Type*>(list), static_cast<Args &&>(args)...);
    }

    J_HIDDEN static sz_t size(const void * list) noexcept {
      return Access::get_size(*reinterpret_cast<const Type*>(list));
    }

    J_HIDDEN static bool empty(const void * list) noexcept {
      return Access::empty(*reinterpret_cast<const Type*>(list));
    }

    template<auto Fn, typename... Args>
    J_HIDDEN static decltype(auto) wrap_insert(void * c, Args... args, const typed_access & value) {
      auto item_def = definition.item_definition();
      if (item_def->can_contain(&value.definition())) {
        auto ptr = item_def->create(value);
        return Fn(
          *reinterpret_cast<Type*>(c),
          static_cast<Args &&>(args)...,
          *reinterpret_cast<const ValueType*>(ptr.as_void_star()));
      } else {
        auto converted_value{item_def->convert_from(value)};
        auto val = converted_value.get_value();
        auto ptr = item_def->create(val);
        return Fn(
          *reinterpret_cast<Type*>(c),
          static_cast<Args &&>(args)...,
          *reinterpret_cast<const ValueType*>(ptr.as_void_star()));
      }
    }

    J_HIDDEN static void item_definition_available(const wrappers::wrapper_definition * def, void*) {
      definition.set_item_definition(def);
      Access::initialize(def);
    }
  public:
    template<typename... Metadata>
    J_HIDDEN explicit list_access_registration(const char * name, Metadata && ... metadata) {
      Access::m_list_definition = &definition;
      if constexpr (sizeof...(Metadata) != 0) {
        convertible_from.maybe_apply([](const auto & ... convs) {
          auto & conversions = definition.conversions();
          (conversions.add_conversion_from(convs.from, convs.conversion), ...);
        }, static_cast<Metadata &&>(metadata)...);
        definition.metadata() = attributes::attribute_map(
          attributes::without(convertible_to, convertible_from, value_metadata),
          static_cast<Metadata &&>(metadata)...
        );
      }
      initialize_typed_access<Type, Access>(definition, PROPERTY_TYPE::LIST, &wrapper, name);
      definition.initialize_list_access(
        &size,
        &empty,
        &with_list<&Access::at, sz_t>,

        &with_list<&Access::begin>,
        &with_list<&Access::end>,

        &wrap_insert<&Access::push_back>,
        &wrap_insert<&Access::insert, const list_iterator &>,

        &with_list<&Access::erase_by_iterator, const list_iterator &>,
        &with_list<&Access::erase_by_index, sz_t>,
        &with_list<&Access::clear>,

        &Access::advance_iterator,
        &Access::iterator_equals,
        &Access::get_iterator_value);

      m_registration.add_to_registry(typeid(Type), &definition, &wrapper);
      if constexpr (sizeof...(Metadata) != 0) {
        if constexpr (j::attributes::has<Metadata...>(value_metadata)) {
          definition.item_metadata() = static_cast<attributes::attribute_map &&>(value_metadata.get(static_cast<Metadata &&>(metadata)...));
        }
        convertible_to.maybe_apply(m_registration, static_cast<Metadata &&>(metadata)...);
      }
      registry::wait_for_wrapper(type_id::type_id(typeid(Type)), type_id::type_id(typeid(ValueType)), &item_definition_available, nullptr);
    }
  };
}
