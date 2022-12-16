#pragma once

#include "strings/string.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "exceptions/assert_lite.hpp"
#include "attributes/attribute_map.hpp"

namespace j::properties::classes {
  using get_object_property_t = void * (*)(void *);

  struct object_property_getter {
    const wrappers::wrapper_definition * definition = nullptr;
    get_object_property_t getter;
    object_property_getter() noexcept = default;

    explicit object_property_getter(get_object_property_t getter);

    wrappers::wrapper get(void * object) const {
      J_ASSERT_NOT_NULL(object, definition);
      return wrappers::wrapper(*definition, getter(object));
    }
  };

  class object_property_definition {
    get_object_property_t m_getter;
    strings::string m_name;
    const wrappers::wrapper_definition * m_definition = nullptr;
    type_id::type_id m_type_id;
    attributes::attribute_map m_metadata;

  public:
    object_property_definition() noexcept;

    object_property_definition(object_property_definition && rhs) noexcept;

    object_property_definition & operator=(object_property_definition && rhs) noexcept;

    ~object_property_definition();

    object_property_definition(
      const char * name,
      get_object_property_t getter,
      const std::type_info & type_id,
      attributes::attribute_map && metadata);

    const strings::string & name() const noexcept {
      return m_name;
    }

    const type_id::type_id & type_id() const noexcept {
      return m_type_id;
    }

    get_object_property_t getter() const noexcept {
      return m_getter;
    }

    const wrappers::wrapper_definition * definition() const {
      J_ASSERT_NOT_NULL(m_definition);
      return m_definition;
    }

    const attributes::attribute_map & metadata() const noexcept {
      return m_metadata;
    }

    void set_definition(const wrappers::wrapper_definition * definition);

    bool operator==(const object_property_definition & rhs) const noexcept;

    bool operator!=(const object_property_definition & rhs) const noexcept;
  };
}
