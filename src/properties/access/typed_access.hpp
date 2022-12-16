#pragma once

#include "attributes/attribute_map.hpp"
#include "properties/property_type.hpp"
#include "properties/access/registry.hpp"
#include "properties/conversions/conversion_collection.hpp"
#include "properties/detail/value.hpp"

namespace j::properties::classes {
  class object_access;
  class object_access_definition;
}

namespace j::properties::wrappers {
  class wrapper;
}

namespace j::properties::access {
  class typed_access;

  /// Base class for typed access definitions.
  class typed_access_definition : public properties::detail::value_definition {
    friend class typed_access;
  public:
    using create_default_t = void *(*)();

    PROPERTY_TYPE type = PROPERTY_TYPE::EMPTY;
  private:

    create_default_t m_create_default = nullptr;
    conversions::conversion_collection m_conversions;
    const wrappers::wrapper_definition * m_reference_wrapper = nullptr;
    type_id::type_id m_type_id;
    const char * m_name = nullptr;
    attributes::attribute_map m_metadata;

  public:
    typed_access_definition() noexcept;

    typed_access_definition(const typed_access_definition & rhs) = delete;
    typed_access_definition & operator=(const typed_access_definition & rhs) = delete;
    typed_access_definition(typed_access_definition && rhs) = delete;
    typed_access_definition & operator=(typed_access_definition && rhs) = delete;

    void initialize(
      const properties::detail::value_definition & value_definition,
      PROPERTY_TYPE type,
      const type_id::type_id & type_id,
      const char * name,
      const wrappers::wrapper_definition * reference_wrapper,
      create_default_t create_default);

    void initialize(
      const properties::detail::value_definition & value_definition,
      PROPERTY_TYPE type,
      const std::type_info & type_id,
      const char * name,
      const wrappers::wrapper_definition * reference_wrapper,
      create_default_t create_default);

    ~typed_access_definition();

    J_INLINE_GETTER bool empty() const noexcept {
      return type == PROPERTY_TYPE::EMPTY;
    }

    J_INLINE_GETTER const char * get_name() const noexcept {
      return m_name;
    }

    const type_id::type_id & type_id() const noexcept;

    bool is_string() const noexcept;

    bool is_int() const noexcept;

    bool is_bool() const noexcept;

    bool is_float() const noexcept;

    bool is_map() const noexcept;

    bool is_set() const noexcept;

    bool is_list() const noexcept;

    bool is_enum() const noexcept;

    bool is_nil() const noexcept;

    bool is_object() const noexcept;

    const class string_access_definition & as_string() const;

    const class int_access_definition & as_int() const;

    const class bool_access_definition & as_bool() const;

    const class float_access_definition & as_float() const;

    const class map_access_definition & as_map() const;

    const class set_access_definition & as_set() const;

    const class list_access_definition & as_list() const;

    const class enum_access_definition & as_enum() const;

    const class nil_access_definition & as_nil() const;

    const classes::object_access_definition & as_object() const;

    wrappers::wrapper create_default() const;

    wrappers::wrapper create_from(const typed_access & rhs) const;

    bool can_create_from(const typed_access_definition & rhs) const noexcept;

    bool can_create_from(const typed_access & rhs) const noexcept;

    J_INLINE_GETTER conversions::conversion_collection & conversions() noexcept {
      return m_conversions;
    }

    J_INLINE_GETTER const attributes::attribute_map & metadata() const noexcept {
      return m_metadata;
    }

    J_INLINE_GETTER attributes::attribute_map & metadata() noexcept {
      return m_metadata;
    }
  };

  namespace detail {
    template<typename T>
    J_NO_DEBUG J_HIDDEN static T & declref() noexcept;

    [[maybe_unused]] J_NO_DEBUG J_HIDDEN static void disable_if_value_base(const properties::detail::value_base *) noexcept;
    [[maybe_unused]] J_NO_DEBUG J_HIDDEN static bool disable_if_value_base(const void *) noexcept;
  }

  class typed_access : public properties::detail::value_base {
  public:
    typed_access() noexcept = default;

    template<typename Tag, typename Ptr>
    typed_access(const Tag & tag, const typed_access_definition & definition, Ptr * v)
      : properties::detail::value_base(tag, static_cast<const properties::detail::value_definition &>(definition), v)
    {
    }

    typed_access(const typed_access_definition & definition, void * property) noexcept
      : properties::detail::value_base(
        properties::detail::access_tag,
        static_cast<const properties::detail::value_definition &>(definition),
        property)
    {
      // Note that property may be null, e.g. with nullptr_t variants the pointer to the
      // object is not passed, but an actual null pointer.
    }

    template<typename Type, decltype(detail::disable_if_value_base(&detail::declref<Type>())) = true>
    J_ALWAYS_INLINE typed_access(Type && rhs)
      : properties::detail::value_base(
        properties::detail::access_tag,
        static_cast<const properties::detail::value_definition &>(*registry::get_typed_access_definition<Type>()),
        (void*)&rhs)
    {
    }

    template<typename Type, sz_t I>
    J_ALWAYS_INLINE typed_access(Type (&& rhs)[I])
      : properties::detail::value_base(
        properties::detail::access_tag,
        static_cast<const properties::detail::value_definition &>(*registry::get_typed_access_definition<Type*>()),
        (void*)&rhs[0])
    {
    }

    J_INLINE_GETTER const typed_access_definition & definition() const noexcept {
      return static_cast<const typed_access_definition &>(value_definition());
    }

    J_INLINE_GETTER PROPERTY_TYPE type() const noexcept {
      return definition().type;
    }

    J_INLINE_GETTER bool is_string() const noexcept {
      return type() == PROPERTY_TYPE::STRING;
    }

    J_INLINE_GETTER bool is_int() const noexcept {
      return type() == PROPERTY_TYPE::INT;
    }

    J_INLINE_GETTER bool is_bool() const noexcept {
      return type() == PROPERTY_TYPE::BOOL;
    }

    J_INLINE_GETTER bool is_float() const noexcept {
      return type() == PROPERTY_TYPE::FLOAT;
    }

    J_INLINE_GETTER bool is_map() const noexcept {
      return type() == PROPERTY_TYPE::MAP;
    }

    J_INLINE_GETTER bool is_set() const noexcept {
      return type() == PROPERTY_TYPE::SET;
    }

    J_INLINE_GETTER bool is_list() const noexcept {
      return type() == PROPERTY_TYPE::LIST;
    }

    J_INLINE_GETTER bool is_enum() const noexcept {
      return type() == PROPERTY_TYPE::ENUM;
    }

    J_INLINE_GETTER bool is_nil() const noexcept {
      return type() == PROPERTY_TYPE::NIL;
    }

    J_INLINE_GETTER bool is_object() const noexcept {
      return type() == PROPERTY_TYPE::OBJECT;
    }

    class string_access & as_string();

    class int_access & as_int();

    class bool_access & as_bool();

    class float_access & as_float();

    class map_access & as_map();

    class set_access & as_set();

    class list_access & as_list();

    class enum_access & as_enum();

    class nil_access & as_nil();

    classes::object_access & as_object();

    const type_id::type_id & type_id() const noexcept;

    void assign_from(const typed_access & rhs);
  };
}
