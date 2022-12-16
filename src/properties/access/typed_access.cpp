#include "properties/access/typed_access.hpp"
#include "properties/exception.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/nil_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/set_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/enum_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/classes/object_access.hpp"
#include "containers/trivial_array.hpp"
#include "properties/access/typed_access_registration.hpp"

J_DEFINE_EXTERN_TRIVIAL_ARRAY(j::type_id::type_id);

namespace j::properties::access {
  typed_access_definition::typed_access_definition() noexcept {
  }

  void typed_access_definition::initialize(
    const properties::detail::value_definition & value_definition,

    PROPERTY_TYPE type,
    const type_id::type_id & type_id,
    const char * name,
    const wrappers::wrapper_definition * reference_wrapper,

    create_default_t create_default
  ) {
    properties::detail::value_definition::initialize(value_definition);
    this->type = type;
    m_create_default = create_default;
    m_reference_wrapper = reference_wrapper;
    m_type_id = type_id;
    m_name = name;

    J_ASSERT_NOT_NULL(m_name, m_name[0], m_reference_wrapper);
  }

  void typed_access_definition::initialize(
    const properties::detail::value_definition & value_definition,

    PROPERTY_TYPE type,
    const std::type_info & type_id,
    const char * name,
    const wrappers::wrapper_definition * reference_wrapper,

    create_default_t create_default
  ) {
    initialize(value_definition, type, type_id::type_id(type_id), name, reference_wrapper, create_default);
  }

  typed_access_definition::~typed_access_definition() {
  }

  const type_id::type_id & typed_access_definition::type_id() const noexcept {
    return m_type_id;
  }

  bool typed_access_definition::is_string() const noexcept {
    return type == PROPERTY_TYPE::STRING;
  }

  bool typed_access_definition::is_int() const noexcept {
    return type == PROPERTY_TYPE::INT;
  }

  bool typed_access_definition::is_bool() const noexcept {
    return type == PROPERTY_TYPE::BOOL;
  }

  bool typed_access_definition::is_float() const noexcept {
    return type == PROPERTY_TYPE::FLOAT;
  }

  bool typed_access_definition::is_map() const noexcept {
    return type == PROPERTY_TYPE::MAP;
  }

  bool typed_access_definition::is_set() const noexcept {
    return type == PROPERTY_TYPE::SET;
  }

  bool typed_access_definition::is_list() const noexcept {
    return type == PROPERTY_TYPE::LIST;
  }

  bool typed_access_definition::is_enum() const noexcept {
    return type == PROPERTY_TYPE::ENUM;
  }

  bool typed_access_definition::is_nil() const noexcept {
    return type == PROPERTY_TYPE::NIL;
  }

  bool typed_access_definition::is_object() const noexcept {
    return type == PROPERTY_TYPE::OBJECT;
  }

  const string_access_definition & typed_access_definition::as_string() const {
    J_REQUIRE(type == PROPERTY_TYPE::STRING,
              "Tried to access a non-string typed_access_definition as a string.");
    return *static_cast<const string_access_definition *>(this);
  }

  const int_access_definition & typed_access_definition::as_int() const {
    J_REQUIRE(type == PROPERTY_TYPE::INT,
              "Tried to access a non-int typed_access_definition as an integer.");
    return *static_cast<const int_access_definition *>(this);
  }

  const bool_access_definition & typed_access_definition::as_bool() const {
    J_REQUIRE(type == PROPERTY_TYPE::BOOL,
              "Tried to access a non-boolean typed_access_definition as a boolean.");
    return *static_cast<const bool_access_definition *>(this);
  }

  const float_access_definition & typed_access_definition::as_float() const {
    J_REQUIRE(type == PROPERTY_TYPE::FLOAT,
              "Tried to access a non-float typed_access_definition as a float.");
    return *static_cast<const float_access_definition *>(this);
  }

  const map_access_definition & typed_access_definition::as_map() const {
    J_REQUIRE(type == PROPERTY_TYPE::MAP,
              "Tried to access a non-map typed_access_definition as a map.");
    return *static_cast<const map_access_definition *>(this);
  }

  const set_access_definition & typed_access_definition::as_set() const {
    J_REQUIRE(type == PROPERTY_TYPE::SET,
              "Tried to access a non-set typed_access_definition as a set.");
    return *static_cast<const set_access_definition *>(this);
  }

  const list_access_definition & typed_access_definition::as_list() const {
    J_REQUIRE(type == PROPERTY_TYPE::LIST,
              "Tried to access a non-list typed_access_definition as a list.");
    return *static_cast<const list_access_definition *>(this);
  }

  const enum_access_definition & typed_access_definition::as_enum() const {
    J_REQUIRE(type == PROPERTY_TYPE::ENUM,
              "Tried to access a non-list typed_access_definition as an enum.");
    return *static_cast<const enum_access_definition *>(this);
  }

  const nil_access_definition & typed_access_definition::as_nil() const {
    J_REQUIRE(type == PROPERTY_TYPE::NIL,
              "Tried to access a non-nil typed_access_definition as nil.");
    return *static_cast<const nil_access_definition *>(this);
  }

  const classes::object_access_definition & typed_access_definition::as_object() const {
    J_REQUIRE(type == PROPERTY_TYPE::OBJECT,
              "Tried to access a non-object typed_access_definition as an object.");
    return *static_cast<const classes::object_access_definition *>(this);
  }

  // wrappers::wrapper typed_access_definition::create_default() const {
  //   return {
  //     properties::detail::wrapper_take_ownership_tag_t{},
  //     *this,
  //     m_create_default()
  //   };
  // }

  wrappers::wrapper typed_access_definition::create_default() const {
    return { properties::detail::wrapper_take_ownership_tag_t{}, *m_reference_wrapper, m_create_default() };
  }

  wrappers::wrapper typed_access_definition::create_from(const typed_access & rhs) const {
    if (&rhs.value_definition() == static_cast<const properties::detail::value_definition*>(this)) {
      return { properties::detail::wrapper_copy_tag_t{}, *m_reference_wrapper, rhs.as_void_star() };
    }
    return m_conversions.create_from(rhs, *m_reference_wrapper);
  }

  bool typed_access_definition::can_create_from(const typed_access_definition & rhs) const noexcept {
    return m_conversions.can_convert_from(rhs) || &rhs == this;
  }

  bool typed_access_definition::can_create_from(const typed_access & rhs) const noexcept {
    return can_create_from(rhs.definition());
  }

  void typed_access::assign_from(const typed_access & rhs) {
    if (&rhs.value_definition() == &value_definition()) {
      *(properties::detail::value_base*)this = rhs;
      return;
    }
    definition().m_conversions.assign_from(*this, rhs);
  }

  string_access & typed_access::as_string() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::STRING,
              "Tried to access non-string typed access as a string.");
    return static_cast<string_access&>(*this);
  }

  int_access & typed_access::as_int() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::INT,
              "Tried to access non-int typed access as an integer.");
    return static_cast<int_access&>(*this);
  }

  bool_access & typed_access::as_bool() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::BOOL,
              "Tried to access non-boolean typed access as a boolean.");
    return static_cast<bool_access&>(*this);
  }

  float_access & typed_access::as_float() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::FLOAT,
              "Tried to access non-float typed access as a float.");
    return static_cast<float_access&>(*this);
  }

  map_access & typed_access::as_map() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::MAP,
              "Tried to access non-map typed access as a map.");
    return static_cast<map_access&>(*this);
  }

  set_access & typed_access::as_set() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::SET,
              "Tried to access non-map typed access as a set.");
    return static_cast<set_access&>(*this);
  }

  list_access & typed_access::as_list() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::LIST,
              "Tried to access non-list typed access as a list.");
    return static_cast<list_access&>(*this);
  }

  enum_access & typed_access::as_enum() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::ENUM,
              "Tried to access non-nil typed access as enum.");
    return static_cast<enum_access&>(*this);
  }

  nil_access & typed_access::as_nil() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::NIL,
              "Tried to access non-nil typed access as nil.");
    return static_cast<nil_access&>(*this);
  }

  classes::object_access & typed_access::as_object() {
    J_REQUIRE(!empty() && definition().type == PROPERTY_TYPE::OBJECT,
              "Tried to access non-object typed access as an object.");
    return static_cast<classes::object_access&>(*this);
  }

  const type_id::type_id & typed_access::type_id() const noexcept {
    return definition().m_type_id;
  }

  typed_access_registration::typed_access_registration() noexcept {
  }

  void typed_access_registration::add_to_registry(const std::type_info & type, typed_access_definition * def, const wrappers::wrapper_definition * wrapper_def) {
    m_type = type;
    J_ASSERT(!m_is_registered, "Tried to register an already-registered type.");
    registry::add_type(m_type, def, wrapper_def);
    m_is_registered = true;
  }

  typed_access_registration::~typed_access_registration() {
    if (m_is_registered) {
      m_is_registered = false;
      registry::remove_type(m_type, m_conversions_to);
    }
  }

  void typed_access_registration::set_num_conversions(sz_t num) {
    m_conversions_to = trivial_array<type_id::type_id>(containers::uninitialized, num);
  }

  void typed_access_registration::add_conversion_to(const conversions::conversion_definition & conversion) {
    m_conversions_to.initialize_element(registry::add_conversion_to(conversion));
  }
}
