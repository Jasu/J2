#include "properties/classes/object_access.hpp"
#include "properties/classes/object_property_registration.hpp"
#include "containers/unsorted_string_map.hpp"

J_DEFINE_EXTERN_UNSORTED_STRING_MAP(j::properties::classes::object_property_getter);
J_DEFINE_EXTERN_UNSORTED_STRING_MAP(j::properties::classes::object_property_definition);

namespace j::properties::classes {
  void object_access_definition::on_property_wrapper_available(object_property_definition* prop, const wrappers::wrapper_definition * def) {
    J_ASSERT_NOT_NULL(prop, def);
    prop->set_definition(def);
    m_property_accessors.at(prop->name()).definition = def;
  }

  void object_access_definition::initialize_definition(object_property_definition && def) {
    m_properties[def.name()] = static_cast<object_property_definition &&>(def);
  }

  void object_access_definition::initialize_object_access_done() {
    m_property_accessors.reserve(m_properties.size());
    for (auto prop : m_properties) {
      m_property_accessors.emplace(*prop.first, object_property_getter(prop.second->getter()));
    }
  }

  const object_property_definition & object_access_definition::get_property(const strings::string & str) const {
    return m_properties.at(str);
  }

  bool object_property_definition::operator==(const object_property_definition & rhs) const noexcept {
    // TODO: Compare metadata.
    return m_getter == rhs.m_getter && m_definition == rhs.m_definition && m_type_id == rhs.m_type_id && m_name == m_name;
  }

  bool object_property_definition::operator!=(const object_property_definition & rhs) const noexcept {
    return !operator==(rhs);
  }

  object_property_definition::object_property_definition(
    const char * name,
    get_object_property_t getter,
    const std::type_info & type_id,
    attributes::attribute_map && metadata
  ) : m_getter(getter),
      m_name(name),
      m_type_id(type_id),
      m_metadata(static_cast<attributes::attribute_map &&>(metadata))
  {
    J_ASSERT_NOT_NULL(getter);
    J_ASSERT(!m_name.empty(), "Cannot create an object property with an empty name.");
  }

  object_access_definition::object_access_definition() noexcept {
  }

  object_access_definition::~object_access_definition() {
  }

  object_iterator object_access::begin() noexcept {
    return { as_void_star(), static_cast<const object_access_definition &>(value_definition()).m_property_accessors.begin() };
  }

  object_iterator object_access::end() noexcept {
    return { as_void_star(), static_cast<const object_access_definition &>(value_definition()).m_property_accessors.end() };
  }

  object_property_definition::object_property_definition() noexcept {
  }

  object_property_definition::~object_property_definition() {
  }

  object_property_definition::object_property_definition(object_property_definition && rhs) noexcept
    : m_getter(rhs.m_getter),
      m_name(static_cast<strings::string &&>(rhs.m_name)),
      m_definition(rhs.m_definition),
      m_type_id(rhs.m_type_id),
      m_metadata(static_cast<attributes::attribute_map &&>(rhs.m_metadata))
  {
  }

  object_property_definition & object_property_definition::operator=(object_property_definition && rhs) noexcept {
    if (this != &rhs) {
      m_getter = rhs.m_getter;
      m_name = static_cast<strings::string &&>(rhs.m_name);
      m_definition = rhs.m_definition;
      m_type_id = rhs.m_type_id;
      m_metadata = static_cast<attributes::attribute_map &&>(rhs.m_metadata);
    }
    return *this;
  }
}
