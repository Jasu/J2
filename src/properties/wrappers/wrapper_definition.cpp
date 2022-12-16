#include "properties/wrappers/wrapper_definition.hpp"

#include "exceptions/assert_lite.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/access/typed_access.hpp"
#include "containers/trivial_array.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(const j::properties::access::typed_access_definition *);

namespace j::properties::wrappers {
  namespace {
    template<PROPERTY_TYPE PropertyType> PROPERTY_TYPE get_property_type(const void *) noexcept;

    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::EMPTY>(const void *) noexcept {
      return PROPERTY_TYPE::EMPTY;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::NIL>(const void *) noexcept {
      return PROPERTY_TYPE::NIL;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::STRING>(const void *) noexcept {
      return PROPERTY_TYPE::STRING;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::INT>(const void *) noexcept {
      return PROPERTY_TYPE::INT;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::BOOL>(const void *) noexcept {
      return PROPERTY_TYPE::BOOL;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::FLOAT>(const void *) noexcept {
      return PROPERTY_TYPE::FLOAT;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::LIST>(const void *) noexcept {
      return PROPERTY_TYPE::LIST;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::MAP>(const void *) noexcept {
      return PROPERTY_TYPE::MAP;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::SET>(const void *) noexcept {
      return PROPERTY_TYPE::SET;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::OBJECT>(const void *) noexcept {
      return PROPERTY_TYPE::OBJECT;
    }
    template<> PROPERTY_TYPE get_property_type<PROPERTY_TYPE::ENUM>(const void *) noexcept {
      return PROPERTY_TYPE::ENUM;
    }
  }
  wrapper_definition::wrapper_definition() noexcept {
  }

  wrapper_definition::wrapper_definition(properties::detail::value_definition && value_definition,
                                         get_current_type_t get_current_type,
                                         set_value_t set_value,
                                         get_value_t get_value,
                                         create_t create,
                                         trivial_array<const access::typed_access_definition *> && possible_contents)
    : properties::detail::value_definition(static_cast<properties::detail::value_definition &&>(value_definition)),
      m_get_current_type(get_current_type),
      m_set_value(set_value),
      m_get_value(get_value),
      m_create(create),
      m_possible_contents(static_cast<trivial_array<const access::typed_access_definition *> &&>(possible_contents))
  {
    J_ASSERT_NOT_NULL(m_get_current_type, m_get_value);
  }

  wrapper_definition::wrapper_definition(properties::detail::value_definition && value_definition,
                                         PROPERTY_TYPE property_type,
                                         set_value_t set_value,
                                         get_value_t get_value,
                                         create_t create,
                                         const access::typed_access_definition * type)
    : properties::detail::value_definition(static_cast<properties::detail::value_definition &&>(value_definition)),
      m_set_value(set_value),
      m_get_value(get_value),
      m_create(create),
      m_possible_contents(containers::move, {type})
  {
    switch (property_type) {
    case PROPERTY_TYPE::EMPTY:  m_get_current_type = &get_property_type<PROPERTY_TYPE::EMPTY>;  break;
    case PROPERTY_TYPE::NIL:    m_get_current_type = &get_property_type<PROPERTY_TYPE::NIL>;    break;
    case PROPERTY_TYPE::STRING: m_get_current_type = &get_property_type<PROPERTY_TYPE::STRING>; break;
    case PROPERTY_TYPE::INT:    m_get_current_type = &get_property_type<PROPERTY_TYPE::INT>;    break;
    case PROPERTY_TYPE::BOOL:   m_get_current_type = &get_property_type<PROPERTY_TYPE::BOOL>;   break;
    case PROPERTY_TYPE::FLOAT:  m_get_current_type = &get_property_type<PROPERTY_TYPE::FLOAT>;  break;
    case PROPERTY_TYPE::LIST:   m_get_current_type = &get_property_type<PROPERTY_TYPE::LIST>;   break;
    case PROPERTY_TYPE::MAP:    m_get_current_type = &get_property_type<PROPERTY_TYPE::MAP>;    break;
    case PROPERTY_TYPE::SET:    m_get_current_type = &get_property_type<PROPERTY_TYPE::SET>;    break;
    case PROPERTY_TYPE::OBJECT: m_get_current_type = &get_property_type<PROPERTY_TYPE::OBJECT>; break;
    case PROPERTY_TYPE::ENUM:   m_get_current_type = &get_property_type<PROPERTY_TYPE::ENUM>;   break;
    }
    J_ASSERT_NOT_NULL(m_get_value);
  }

  wrapper_definition::~wrapper_definition() {
  }

  wrapper wrapper_definition::create(const access::typed_access & access) const {
    J_ASSERT_NOT_NULL(m_create);
    return m_create(*this, const_cast<access::typed_access &>(access));
  }

  wrappers::wrapper wrapper_definition::convert_from(const access::typed_access & item) const {
    const access::typed_access_definition * match = nullptr;
    auto & item_def = item.definition();
    for (auto & type : possible_contents()) {
      if (type->can_create_from(item_def)) {
        match = type;
        if (type->type == item_def.type) {
          return type->create_from(item);
        }
      }
    }
    if (J_UNLIKELY(!match)) {
      J_THROW("No conversion found.");
    }
    return match->create_from(item);
  }

  bool wrapper_definition::can_contain(const access::typed_access_definition * def) const noexcept {
    for (auto & d : m_possible_contents) {
      if (d == def) {
        return true;
      }
    }
    return false;
  }
}
