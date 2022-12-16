#include "properties/wrappers/variant_wrapper.hpp"

#include "properties/property_variant.hpp"

#include "properties/access/typed_access.hpp"

#include "properties/access/int_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/nil_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/bool_access.hpp"

#include "properties/access/default_accesses.hpp"

#include "properties/exception.hpp"
#include "properties/wrappers/wrapper_registration.hpp"
#include "properties/detail/value_definition_impl.hpp"

namespace j::properties::wrappers::detail {
  PROPERTY_TYPE get_variant_type(const void * variant);
  void set_variant_value(void * variant, const access::typed_access & typed_access);
  access::typed_access get_variant_value(void * variant);
  wrapper create_variant_wrapper(const wrapper_definition & def, access::typed_access & a);

  const wrapper_definition variant_wrapper_definition{
    properties::detail::make_value_definition<property_variant>(),
    get_variant_type,
    set_variant_value,
    get_variant_value,
    create_variant_wrapper,
    trivial_array<const access::typed_access_definition *>(
      containers::move,
      {
        access::typed_access_definition_v<null_t>,
        access::typed_access_definition_v<strings::string>,
        access::typed_access_definition_v<i64_t>,
        access::typed_access_definition_v<bool>,
        access::typed_access_definition_v<double>,
        access::variant_list_access_definition,
        access::variant_map_access_definition
      })
  };

  static const wrapper_registration<property_variant> variant_wrapper_registration(&variant_wrapper_definition);

  PROPERTY_TYPE get_variant_type(const void * variant) {
    J_ASSERT_NOT_NULL(variant);
    return reinterpret_cast<const property_variant *>(variant)->type();
  }

  void set_variant_value(void * variant, const access::typed_access & typed_access_) {
    J_ASSERT_NOT_NULL(variant);
    auto & v = *reinterpret_cast<property_variant *>(variant);
    if (v.type() == typed_access_.type()) {
      get_variant_value(variant).assign_from(typed_access_);
      return;
    }
    access::typed_access typed_access = typed_access_;
    switch (typed_access.type()) {
    case PROPERTY_TYPE::STRING:
      v = typed_access.as_string().as_j_string();
      break;
    case PROPERTY_TYPE::INT:
      v = typed_access.as_int().get_value();
      break;
    case PROPERTY_TYPE::LIST:
      if (&typed_access.definition() == access::variant_list_access_definition) {
        v = *reinterpret_cast<list_t*>(typed_access.as_void_star());
      } else {
        list_t l;
        access::list_access la{*access::variant_list_access_definition, &l};
        la.assign_from(typed_access);
        v = static_cast<list_t &&>(l);
      }
      break;
    case PROPERTY_TYPE::MAP:
      if (&typed_access.definition() == access::variant_map_access_definition) {
        v = *reinterpret_cast<map_t*>(typed_access.as_void_star());
      } else {
        map_t m;
        access::map_access ma{*access::variant_map_access_definition, &m};
        ma.assign_from(typed_access);
        v = static_cast<map_t &&>(m);
      }
      break;
    case PROPERTY_TYPE::FLOAT:
      v = typed_access.as_float().get_value();
      break;
    case PROPERTY_TYPE::BOOL:
      v = typed_access.as_bool().get_value();
      break;
    case PROPERTY_TYPE::NIL:
      v = nullptr;
      break;
    case PROPERTY_TYPE::ENUM:
      J_THROW(empty_property_access_exception()
              << message("Tried to assign an enumeration value to a variant."));
    case PROPERTY_TYPE::SET:
      J_THROW(empty_property_access_exception()
              << message("Tried to assign a set value to a variant."));
    case PROPERTY_TYPE::EMPTY:
      J_THROW(empty_property_access_exception()
              << message("Tried to assign an empty value to a variant."));
    case PROPERTY_TYPE::OBJECT:
      J_THROW(empty_property_access_exception()
              << message("Tried to assign an object to a variant."));
    }
  }

  access::typed_access get_variant_value(void * variant) {
    J_ASSERT_NOT_NULL(variant);
    auto & v = *reinterpret_cast<property_variant *>(variant);
    switch (v.type()) {
    case PROPERTY_TYPE::EMPTY:
      J_THROW(empty_property_access_exception()
              << message("Tried to get an empty value from a variant."));
    case PROPERTY_TYPE::ENUM:
      J_THROW(empty_property_access_exception()
              << message("Tried to get an enumeration value from a variant - variant in an invalid state."));
    case PROPERTY_TYPE::SET:
      J_THROW(empty_property_access_exception()
              << message("Tried to get a set value from a variant - variant in an invalid state."));
    case PROPERTY_TYPE::NIL:
      return access::typed_access{*access::typed_access_definition_v<null_t>, &v.as_nil()};
    case PROPERTY_TYPE::STRING:
      return access::string_access{*access::typed_access_definition_v<string_t>, &v.as<string_t>()};
    case PROPERTY_TYPE::INT:
      return access::int_access{*access::typed_access_definition_v<int_t>, &v.as<int_t>()};
    case PROPERTY_TYPE::BOOL:
      return access::bool_access{*access::typed_access_definition_v<bool_t>, &v.as<bool_t>()};
    case PROPERTY_TYPE::FLOAT:
      return access::float_access{*access::typed_access_definition_v<float_t>, &v.as<float_t>()};
    case PROPERTY_TYPE::LIST:
      return access::list_access{*access::variant_list_access_definition, &v.as<list_t>()};
    case PROPERTY_TYPE::MAP:
      return access::map_access{*access::variant_map_access_definition, &v.as<map_t>()};
    case PROPERTY_TYPE::OBJECT:
      J_THROW(
        invalid_type_property_access_exception()
        << message("Tried to get an object from a variant. Variants cannot hold objects."));
    }
  }

  wrapper create_variant_wrapper(const wrapper_definition & def, access::typed_access & a) {
    if (&a.definition() == access::typed_access_definition_v<null_t>) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(nullptr)};
    } else if (&a.definition() == access::typed_access_definition_v<strings::string>) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<strings::string*>(a.as_void_star()))};
    } else if (&a.definition() == access::typed_access_definition_v<i64_t>) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<i64_t*>(a.as_void_star()))};
    } else if (&a.definition() == access::typed_access_definition_v<bool>) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<bool*>(a.as_void_star()))};
    } else if (&a.definition() == access::typed_access_definition_v<double>) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<double*>(a.as_void_star()))};
    } else if (&a.definition() == access::variant_list_access_definition) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<list_t*>(a.as_void_star()))};
    } else if (&a.definition() == access::variant_map_access_definition) {
      return {properties::detail::wrapper_take_ownership_tag_t{}, def, new property_variant(*reinterpret_cast<map_t*>(a.as_void_star()))};
    } else {
      J_THROW(invalid_type_property_access_exception()
              << message("Tried to create a variant wrapper from an invalid type."));
    }
  }

  void delete_variant_wrapper(void * wrapper) {
    J_ASSERT_NOT_NULL(wrapper);
    delete reinterpret_cast<property_variant*>(wrapper);
  }
}
