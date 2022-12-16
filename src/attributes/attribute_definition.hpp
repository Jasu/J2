#pragma once

#include "attributes/attribute.hpp"
#include "attributes/map_detail.hpp"
#include "attributes/common.hpp"
#include "attributes/value_handler.hpp"
#include "attributes/keyed_attributes.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::attributes {
  /// Definition of an attribute.
  ///
  /// Assigning to an attribute_definition creates an attribute object, wrapping the value
  /// being assigned.
  template<typename ValueType, typename Tag, typename Policy = single, typename Key = void>
  struct attribute_definition final : detail::value_handler<ValueType, Tag, Policy>
  {
    inline constexpr static bool J_NO_DEBUG is_any_v = false;
    using tag_t J_NO_DEBUG_TYPE = Tag;
    using value_t J_NO_DEBUG_TYPE = ValueType;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept { }

    template<typename K>
    J_INLINE_GETTER constexpr auto operator[](K && rhs) const noexcept(is_nothrow_copy_constructible_v<K &&>) {
      return detail::assign_wrapper<Key, ValueType, attribute_definition>{Key(static_cast<K &&>(rhs))};
    }
  };


  template<typename Tag>
  struct J_TYPE_HIDDEN attribute_definition<bool, Tag, flag, void> final {
    using tag_t = Tag;
    using value_t = bool;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept { }

    inline constexpr static bool J_NO_DEBUG is_multiple_v = false;
    inline constexpr static bool J_NO_DEBUG is_any_v = false;

    inline constexpr static bool J_NO_DEBUG m_value = true;

    consteval const attribute_definition & operator=(bool) const noexcept {
      return *this;
    }

    template<typename... Args>
    consteval static bool get(const Args & ...) noexcept {
      return (detail::matches_v<Args &, Tag> || ...);
    }
  };


  /// Specialization for attributes that can contain any value.
  template<typename Tag, typename Policy>
  struct J_TYPE_HIDDEN attribute_definition<mem::any, Tag, Policy, void> final
    : detail::value_handler<mem::any, Tag, Policy> {
    J_NO_DEBUG inline constexpr static bool is_any_v = true;
    using tag_t J_NO_DEBUG_TYPE = Tag;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept { }

    template<typename Arg>
    J_INLINE_GETTER_NO_DEBUG constexpr auto operator=(Arg && rhs) const noexcept(noexcept(consume(declval<Arg>()))) {
      return attribute<attribute_definition, decay_t<Arg>> {
        static_cast<Arg &&>(rhs)
      };
    }
  };

  /// Specialization for attributes that contain a type value.
  template<typename Tag, typename Policy>
  struct J_TYPE_HIDDEN attribute_definition<type_value, Tag, Policy, void> final {
    J_NO_DEBUG inline constexpr static bool is_any_v = true;
    using tag_t J_NO_DEBUG_TYPE = Tag;
    using value_t J_NO_DEBUG_TYPE = void;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept {
    }

    template<typename T>
    J_INLINE_GETTER_NO_DEBUG constexpr attribute<attribute_definition, type_tag<T>> operator=(const type_tag<T> &) const noexcept {
      return {};
    }

    template<typename... Args>
    J_NO_DEBUG static constexpr typename decltype(identity(detail::get_first<Tag>(declref<Args>()...)))::type get() noexcept;
  };

  /// Specialization for non-key-value, non-flag attributes with a specified type.
  template<typename ValueType, typename Tag, typename Policy>
  struct J_TYPE_HIDDEN attribute_definition<ValueType, Tag, Policy, void> final
    : detail::value_handler<ValueType, Tag, Policy>
  {
    inline constexpr static bool is_any_v J_NO_DEBUG = false;
    using tag_t J_NO_DEBUG_TYPE = Tag;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept {
    }

    J_INLINE_GETTER_NO_DEBUG constexpr attribute<attribute_definition, ValueType> operator=(ValueType && value) const noexcept {
      return {static_cast<ValueType &&>(value)};
    }

    J_INLINE_GETTER_NO_DEBUG constexpr attribute<attribute_definition, ValueType> operator=(const ValueType & value) const noexcept(is_nothrow_copy_constructible_v<ValueType>) {
      return {value};
    }
  };

  /// Specialization for key-value attributes that return an attribute pair.
  template<typename Tag, typename Policy, typename KeyType>
  struct J_TYPE_HIDDEN attribute_definition<mem::any, Tag, Policy, KeyType> final
    : detail::value_handler<mem::any, Tag, Policy>
  {
    using tag_t J_NO_DEBUG_TYPE = Tag;
    inline constexpr static bool is_any_v J_NO_DEBUG = true;

    template<typename... Args>
    explicit consteval attribute_definition(Args && ...) noexcept { }

    J_INLINE_GETTER_NO_DEBUG constexpr detail::assign_wrapper_any<KeyType, attribute_definition> operator[](KeyType && key) const noexcept {
      return detail::assign_wrapper_any<KeyType, attribute_definition>{static_cast<KeyType &&>(key)};
    }

    J_INLINE_GETTER_NO_DEBUG constexpr detail::assign_wrapper_any<KeyType, attribute_definition> operator[](const KeyType & key) const noexcept(is_nothrow_copy_constructible_v<KeyType>) {
      return detail::assign_wrapper_any<KeyType, attribute_definition>{key};
    }
  };

}

namespace j::attributes {
  namespace detail {
    using tag_attribute_definition_t J_NO_DEBUG_TYPE = attribute_definition<type_value, attribute_tag_tag>;
    using key_attribute_definition_t J_NO_DEBUG_TYPE = attribute_definition<type_value, attribute_key_tag>;
    using value_attribute_definition_t J_NO_DEBUG_TYPE = attribute_definition<type_value, attribute_value_type_tag>;
    using multiple_attribute_definition_t J_NO_DEBUG_TYPE = attribute_definition<bool, multiple, flag>;
    using flag_attribute_definition_t J_NO_DEBUG_TYPE = attribute_definition<bool, flag, flag>;
  }

  [[maybe_unused]] inline constexpr detail::tag_attribute_definition_t tag{};
  [[maybe_unused]] inline constexpr detail::key_attribute_definition_t key_type{};
  [[maybe_unused]] inline constexpr detail::value_attribute_definition_t value_type{};
  [[maybe_unused]] inline constexpr detail::multiple_attribute_definition_t is_multiple{};
  [[maybe_unused]] inline constexpr detail::flag_attribute_definition_t is_flag{};

  namespace detail {
    template<typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer;

    template<>
    struct J_TYPE_HIDDEN attribute_definition_deducer<> {
      using value_t J_NO_DEBUG_TYPE = mem::any;
      using policy_t J_NO_DEBUG_TYPE = single;
      using key_t J_NO_DEBUG_TYPE = void;
    };

    template<typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer<multiple_attribute_definition_t, Tail...>
      : attribute_definition_deducer<Tail...> {
      using policy_t J_NO_DEBUG_TYPE = multiple;
    };

    template<typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer<flag_attribute_definition_t, Tail...>
      : attribute_definition_deducer<Tail...> {
      using value_t J_NO_DEBUG_TYPE = bool;
      using policy_t J_NO_DEBUG_TYPE = flag;
    };

    template<typename NewValueType, typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer<attribute<value_attribute_definition_t, type_tag<NewValueType>>, Tail...>
      : attribute_definition_deducer<Tail...> {
      using value_t J_NO_DEBUG_TYPE = NewValueType;
    };

    template<typename NewKeyType, typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer<attribute<key_attribute_definition_t, type_tag<NewKeyType>>, Tail...>
      : attribute_definition_deducer<Tail...> {
      using key_t J_NO_DEBUG_TYPE = NewKeyType;
    };

    template<typename NewTagType, typename... Tail>
    struct J_TYPE_HIDDEN attribute_definition_deducer<attribute<tag_attribute_definition_t, type_tag<NewTagType>>, Tail...>
      : attribute_definition_deducer<Tail...> {
      using tag_t J_NO_DEBUG_TYPE = NewTagType;
    };
  }

  template<typename Tag, typename Value>
  explicit attribute_definition(attribute<detail::tag_attribute_definition_t, type_tag<Tag>>, attribute<detail::value_attribute_definition_t, type_tag<Value>>) -> attribute_definition<Value, Tag, single, void>;

  template<typename Tag, typename Value>
  explicit attribute_definition(attribute<detail::value_attribute_definition_t, type_tag<Value>>, attribute<detail::tag_attribute_definition_t, type_tag<Tag>>) -> attribute_definition<Value, Tag, single, void>;

  template<typename... Attributes, typename Deducer = detail::attribute_definition_deducer<Attributes...>>
  explicit attribute_definition(Attributes...) -> attribute_definition<
    typename Deducer::value_t,
    typename Deducer::tag_t,
    typename Deducer::policy_t,
    typename Deducer::key_t>;
}
