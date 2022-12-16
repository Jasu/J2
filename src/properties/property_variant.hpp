#pragma once

#include "properties/property_defs.hpp"
#include "hzd/type_traits.hpp"
#include "hzd/mem.hpp"

#include "containers/vector.hpp"
#include "containers/unsorted_string_map_fwd.hpp"
#include "strings/string.hpp"

namespace j::properties {
  class property_variant;

  namespace detail {
    template<typename T, typename Enable = void>
    struct J_TYPE_HIDDEN property_variant_type { using type J_NO_DEBUG_TYPE = T; };

    template<>
    struct J_TYPE_HIDDEN property_variant_type<bool, void> { using type J_NO_DEBUG_TYPE = bool; };

    template<typename T>
    struct J_TYPE_HIDDEN property_variant_type<T, j::enable_if_t<j::is_integral_v<T>>> { using type J_NO_DEBUG_TYPE = int_t; };

    template<typename T>
    struct J_TYPE_HIDDEN property_variant_type<T, j::enable_if_t<j::is_floating_point_v<T>>> { using type J_NO_DEBUG_TYPE = float_t; };

    template<>
    struct J_TYPE_HIDDEN property_variant_type<const char *, void> { using type J_NO_DEBUG_TYPE = strings::string; };

    template<typename T>
    using property_variant_type_t J_NO_DEBUG_TYPE = typename property_variant_type<decay_t<T>>::type;

    template<typename Type>
    inline constexpr PROPERTY_TYPE property_enum_v = Type::dummy;

    template<> inline constexpr PROPERTY_TYPE property_enum_v<map_t> = PROPERTY_TYPE::MAP;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<list_t> = PROPERTY_TYPE::LIST;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<float_t> = PROPERTY_TYPE::FLOAT;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<bool_t> = PROPERTY_TYPE::BOOL;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<int_t> = PROPERTY_TYPE::INT;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<string_t> = PROPERTY_TYPE::STRING;
    template<> inline constexpr PROPERTY_TYPE property_enum_v<nil_t> = PROPERTY_TYPE::NIL;


    template<typename T>
    struct J_TYPE_HIDDEN disable_if_variant { using type = void; };
    template<>
    struct J_TYPE_HIDDEN disable_if_variant<property_variant &> { };
    template<>
    struct J_TYPE_HIDDEN disable_if_variant<const property_variant &> { };
  }


  /// Variant for storing properties in a normalized format.
  class property_variant {
    u8_t m_data[max_size_v<nil_t, string_t, int_t, bool_t, float_t, list_t, map_t>];
    PROPERTY_TYPE m_type;

    template<typename Type>
    J_INLINE_GETTER Type & raw_reference() {
      return *reinterpret_cast<Type*>(m_data);
    }

    template<typename Type>
    J_INLINE_GETTER const Type & raw_reference() const {
      return *reinterpret_cast<const Type*>(m_data);
    }

    [[noreturn]] void throw_empty() const;

    [[noreturn]] void throw_incorrect_type(PROPERTY_TYPE type) const;

    template<typename Type>
    void initialize(Type && rhs) {
      using type J_NO_DEBUG_TYPE = detail::property_variant_type_t<Type>;
      if constexpr (j::is_same_v<type, string_t>) {
        // const char *
        initialize(strings::string(static_cast<Type &&>(rhs)));
      } else {
        m_type = detail::property_enum_v<type>;
        raw_reference<type>() = static_cast<Type &&>(rhs);
      }
    }

    template<> void initialize(const string_t & rhs);
    template<> void initialize(string_t && rhs);
    template<> void initialize(string_t & rhs) {
      initialize(const_cast<const string_t &>(rhs));
    }

    template<> void initialize(const map_t & rhs);
    template<> void initialize(map_t & rhs) {
      initialize(const_cast<const map_t &>(rhs));
    }
    template<> void initialize(map_t && rhs);

    template<> void initialize(const list_t & rhs);
    template<> void initialize(list_t && rhs);
    template<> void initialize(list_t & rhs) {
      initialize(const_cast<const list_t &>(rhs));
    }

    void free_string_content() noexcept;
    void free_map_content() noexcept;
    void free_list_content() noexcept;

    void free_content() noexcept {
      switch (m_type) {
      case PROPERTY_TYPE::STRING:
        free_string_content();
        break;
      case PROPERTY_TYPE::MAP:
        free_map_content();
        break;
      case PROPERTY_TYPE::LIST:
        free_list_content();
        break;
      default:
        break;
      }
    }

  public:
    property_variant() noexcept : m_type(PROPERTY_TYPE::EMPTY) {
    }

    template<typename Type, typename = typename detail::disable_if_variant<Type &>::type>
    J_ALWAYS_INLINE property_variant(Type && rhs) noexcept {
      initialize(static_cast<Type &&>(rhs));
    }

    property_variant(const property_variant & rhs);
    property_variant(property_variant && rhs) noexcept;
    property_variant & operator=(const property_variant & rhs);
    property_variant & operator=(property_variant && rhs) noexcept;

    template<typename Type, typename = typename detail::disable_if_variant<Type &>::type>
    J_ALWAYS_INLINE property_variant & operator=(Type && rhs) {
      using type = detail::property_variant_type_t<Type>;
      if (m_type == detail::property_enum_v<type>) {
        raw_reference<type>() = static_cast<Type &&>(rhs);
      } else {
        free_content();
        initialize(static_cast<Type &&>(rhs));
      }
      return *this;
    }

    bool operator==(const property_variant & rhs) const noexcept;

    ~property_variant() {
      free_content();
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return m_type == PROPERTY_TYPE::EMPTY;
    }

    J_INLINE_GETTER PROPERTY_TYPE type() const noexcept {
      return m_type;
    }

    J_INLINE_GETTER bool is(PROPERTY_TYPE type) const noexcept {
      return m_type == type;
    }

    template<typename Type>
    J_INLINE_GETTER bool is() const noexcept {
      return m_type == detail::property_enum_v<Type>;
    }

    J_INLINE_GETTER bool is_nil() const noexcept {
      return m_type == PROPERTY_TYPE::NIL;
    }

    J_INLINE_GETTER bool is_string() const noexcept {
      return m_type == PROPERTY_TYPE::STRING;
    }

    J_INLINE_GETTER bool is_int() const noexcept {
      return m_type == PROPERTY_TYPE::INT;
    }

    J_INLINE_GETTER bool is_bool() const noexcept {
      return m_type == PROPERTY_TYPE::BOOL;
    }

    J_INLINE_GETTER bool is_float() const noexcept {
      return m_type == PROPERTY_TYPE::FLOAT;
    }

    J_INLINE_GETTER bool is_list() const noexcept {
      return m_type == PROPERTY_TYPE::LIST;
    }

    J_INLINE_GETTER bool is_map() const noexcept {
      return m_type == PROPERTY_TYPE::MAP;
    }

    template<typename Type>
    void assert_is() const {
      if (!is<Type>()) {
        throw_incorrect_type(detail::property_enum_v<Type>);
      }
    }

    template<typename Type>
    J_INLINE_GETTER Type & as() & {
      assert_is<Type>();
      return raw_reference<Type>();
    }

    template<typename Type>
    J_INLINE_GETTER Type && as() && {
      assert_is<Type>();
      return static_cast<Type &&>(raw_reference<Type>());
    }

    template<typename Type>
    J_INLINE_GETTER const Type & as() const {
      assert_is<Type>();
      return raw_reference<Type>();
    }

    J_INLINE_GETTER nil_t as_nil() const {
      return as<nil_t>();
    }

    J_INLINE_GETTER nil_t & as_nil() {
      return as<nil_t>();
    }

    J_INLINE_GETTER const string_t & as_string() const {
      return as<string_t>();
    }

    J_INLINE_GETTER string_t & as_string() {
      return as<string_t>();
    }

    J_INLINE_GETTER int_t as_int() const {
      return as<int_t>();
    }

    J_INLINE_GETTER int_t & as_int() {
      return as<int_t>();
    }

    J_INLINE_GETTER float_t as_float() const {
      return as<float_t>();
    }

    J_INLINE_GETTER float_t & as_float() {
      return as<float_t>();
    }

    J_INLINE_GETTER const list_t & as_list() const {
      return as<list_t>();
    }

    J_INLINE_GETTER list_t & as_list() {
      return as<list_t>();
    }

    J_INLINE_GETTER const map_t & as_map() const {
      return as<map_t>();
    }

    J_INLINE_GETTER map_t & as_map() {
      return as<map_t>();
    }

    J_INLINE_GETTER bool as_bool() const {
      return as<bool_t>();
    }

    J_INLINE_GETTER bool_t & as_bool() {
      return as<bool_t>();
    }

    template<typename Type>
    const Type & as_same_as(const Type &) const {
      assert_is<Type>();
      return raw_reference<Type>();
    }

    template<typename Type>
    Type & as_same_as(const Type &) & {
      assert_is<Type>();
      return raw_reference<Type>();
    }

    template<typename Type>
    Type && as_same_as(const Type &) && {
      assert_is<Type>();
      return static_cast<Type &&>(raw_reference<Type>());
    }

    void reset() noexcept {
      free_content();
      m_type = PROPERTY_TYPE::EMPTY;
    }

    J_INLINE_GETTER void * as_raw_ptr() noexcept {
      return &m_data;
    }

    J_INLINE_GETTER const void * as_raw_ptr() const noexcept {
      return &m_data;
    }
  };
}

J_DECLARE_EXTERN_VECTOR(j::properties::property_variant);
J_DECLARE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::properties::property_variant);
