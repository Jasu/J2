#include "properties/conversions/conversion_collection.hpp"
#include "properties/conversions/conversion_definition.hpp"
#include "type_id/type_hash.hpp"
#include "properties/conversions/conversion.hpp"
#include "properties/wrappers/wrapper.hpp"
#include "properties/classes/object_access.hpp"
#include "properties/access/typed_access.hpp"
#include "properties/access/enum_access.hpp"
#include "properties/access/list_access.hpp"
#include "properties/access/map_access.hpp"
#include "properties/access/string_access.hpp"
#include "properties/access/bool_access.hpp"
#include "properties/access/float_access.hpp"
#include "properties/access/int_access.hpp"
#include "properties/access/set_access.hpp"
#include "containers/hash_map.hpp"
#include "strings/string.hpp"

namespace j::properties::conversions {
  using conversion_map_t = hash_map<type_id::type_id, conversion, type_id::hash>;

  conversion_collection::conversion_collection() noexcept {
    static_assert(sizeof(conversion_collection::m_map) == sizeof(conversion_map_t));
    new (&m_map) conversion_map_t;
  }

  conversion_collection::conversion_collection(conversion_collection && rhs) noexcept {
    new (&m_map) conversion_map_t(static_cast<conversion_map_t &&>(*reinterpret_cast<conversion_map_t*>(&rhs.m_map)));
  }

  conversion_collection::conversion_collection(const conversion_collection & rhs) {
    new (&m_map) conversion_map_t(*reinterpret_cast<const conversion_map_t*>(&rhs.m_map));
  }

  conversion_collection & conversion_collection::operator=(conversion_collection && rhs) noexcept {
    *reinterpret_cast<conversion_map_t*>(&m_map) = static_cast<conversion_map_t &&>(*reinterpret_cast<conversion_map_t*>(&rhs.m_map));
    return *this;
  }

  conversion_collection & conversion_collection::operator=(const conversion_collection & rhs) {
    *reinterpret_cast<conversion_map_t*>(&m_map) = *reinterpret_cast<const conversion_map_t*>(&rhs.m_map);
    return *this;
  }

  conversion_collection::~conversion_collection() {
    reinterpret_cast<conversion_map_t*>(&m_map)->~conversion_map_t();
  }

  void conversion_collection::reserve([[maybe_unused]] sz_t sz) {
    // reinterpret_cast<conversion_map_t*>(&m_map)->reserve(sz);
  }

  bool conversion_collection::can_convert_from(const access::typed_access_definition & from) const noexcept {
    auto & map = *reinterpret_cast<const conversion_map_t*>(&m_map);
    if (map.contains(from.type_id())) {
      return true;
    }
    switch (from.type) {
    case PROPERTY_TYPE::EMPTY: return false;
    case PROPERTY_TYPE::NIL:
      return map.contains(type_id::type_id(typeid(std::nullptr_t)));
    case PROPERTY_TYPE::STRING:
      return map.contains(type_id::type_id(typeid(strings::const_string_view)));
    case PROPERTY_TYPE::INT:
      return map.contains(type_id::type_id(typeid(i64_t)));
    case PROPERTY_TYPE::BOOL:
      return map.contains(type_id::type_id(typeid(bool)));
    case PROPERTY_TYPE::FLOAT:
      return map.contains(type_id::type_id(typeid(double)));
    case PROPERTY_TYPE::LIST:
      return map.contains(type_id::type_id(typeid(access::list_access)));
    case PROPERTY_TYPE::MAP:
      return map.contains(type_id::type_id(typeid(access::map_access)));
    case PROPERTY_TYPE::SET:
      return map.contains(type_id::type_id(typeid(access::set_access)));
    case PROPERTY_TYPE::OBJECT:
      return map.contains(type_id::type_id(typeid(classes::object_access)));
    case PROPERTY_TYPE::ENUM:
      return map.contains(type_id::type_id(typeid(access::enum_access)));
    }
  }

  wrappers::wrapper conversion_collection::create_from(
    const access::typed_access & from,
    const wrappers::wrapper_definition & def) const
  {
    auto & map = *reinterpret_cast<const conversion_map_t*>(&m_map);
    auto it = map.find(from.type_id());
    if (it != map.end()) {
      return it->second.create_from(def, from.as_void_star());
    }
    switch (from.type()) {
    case PROPERTY_TYPE::EMPTY:
      J_THROW("Cannot create from an empty value.");
    case PROPERTY_TYPE::NIL: {
      auto n = nullptr;
      return map.at(type_id::type_id(typeid(std::nullptr_t))).create_from(def, &n);
    }
    case PROPERTY_TYPE::STRING: {
      strings::const_string_view s = const_cast<access::typed_access &>(from).as_string().as_j_string_view();
      return map.at(type_id::type_id(typeid(strings::const_string_view))).create_from(def, &s);
    }
    case PROPERTY_TYPE::INT: {
      i64_t value = const_cast<access::typed_access &>(from).as_int().get_value();
      return map.at(type_id::type_id(typeid(i64_t))).create_from(def, &value);
    }
    case PROPERTY_TYPE::BOOL: {
      bool value = const_cast<access::typed_access &>(from).as_bool().get_value();
      return map.at(type_id::type_id(typeid(bool))).create_from(def, &value);
    }
    case PROPERTY_TYPE::FLOAT: {
      double value = const_cast<access::typed_access &>(from).as_float().get_value();
      return map.at(type_id::type_id(typeid(double))).create_from(def, &value);
    }
    case PROPERTY_TYPE::LIST: {
      return map.at(type_id::type_id(typeid(access::list_access)))
        .create_from(def, &const_cast<access::typed_access &>(from).as_list());
    }
    case PROPERTY_TYPE::MAP: {
      return map.at(type_id::type_id(typeid(access::map_access)))
        .create_from(def, &const_cast<access::typed_access &>(from).as_map());
    }
    case PROPERTY_TYPE::SET: {
      return map.at(type_id::type_id(typeid(access::set_access)))
        .create_from(def, &const_cast<access::typed_access &>(from).as_set());
    }
    case PROPERTY_TYPE::OBJECT: {
      return map.at(type_id::type_id(typeid(classes::object_access)))
        .create_from(def, &const_cast<access::typed_access &>(from).as_object());
    }
    case PROPERTY_TYPE::ENUM: {
      return map.at(type_id::type_id(typeid(access::enum_access)))
        .create_from(def, &const_cast<access::typed_access &>(from).as_enum());
    }
    }
  }

  void conversion_collection::assign_from(
    access::typed_access & to,
    const access::typed_access & from) const
  {
    auto & map = *reinterpret_cast<const conversion_map_t*>(&m_map);
    auto it = map.find(from.type_id());
    if (it != map.end()) {
      it->second.assign_from(to, from.as_void_star());
      return;
    }

    switch (from.type()) {
    case PROPERTY_TYPE::EMPTY:
      J_THROW("Cannot create from an empty value.");
    case PROPERTY_TYPE::NIL: {
      auto n = nullptr;
      map.at(type_id::type_id(typeid(std::nullptr_t))).assign_from(to, &n);
      break;
    }
    case PROPERTY_TYPE::STRING: {
      strings::const_string_view s = const_cast<access::typed_access &>(from).as_string().as_j_string_view();
      map.at(type_id::type_id(typeid(strings::const_string_view))).assign_from(to, &s);
      break;
    }
    case PROPERTY_TYPE::INT: {
      i64_t value = const_cast<access::typed_access &>(from).as_int().get_value();
      map.at(type_id::type_id(typeid(i64_t))).assign_from(to, &value);
      break;
    }
    case PROPERTY_TYPE::BOOL: {
      bool value = const_cast<access::typed_access &>(from).as_bool().get_value();
      map.at(type_id::type_id(typeid(bool))).assign_from(to, &value);
      break;
    }
    case PROPERTY_TYPE::FLOAT: {
      double value = const_cast<access::typed_access &>(from).as_float().get_value();
      map.at(type_id::type_id(typeid(double))).assign_from(to, &value);
      break;
    }
    case PROPERTY_TYPE::LIST: {
      map.at(type_id::type_id(typeid(access::list_access)))
        .assign_from(to, &const_cast<access::typed_access &>(from).as_list());
      break;
    }
    case PROPERTY_TYPE::MAP: {
      map.at(type_id::type_id(typeid(access::map_access)))
        .assign_from(to, &const_cast<access::typed_access &>(from).as_map());
      break;
    }
    case PROPERTY_TYPE::SET: {
      map.at(type_id::type_id(typeid(access::set_access)))
        .assign_from(to, &const_cast<access::typed_access &>(from).as_set());
      break;
    }
    case PROPERTY_TYPE::OBJECT: {
      map.at(type_id::type_id(typeid(classes::object_access)))
        .assign_from(to, &const_cast<access::typed_access &>(from).as_object());
      break;
    }
    case PROPERTY_TYPE::ENUM: {
      map.at(type_id::type_id(typeid(access::enum_access)))
        .assign_from(to, &const_cast<access::typed_access &>(from).as_enum());
      break;
    }
    }
  }

  void conversion_collection::add_conversion_from(
    const type_id::type_id & type,
    const conversion & conversion)
  {
    [[maybe_unused]] bool did_create = reinterpret_cast<conversion_map_t*>(&m_map)->emplace(type, conversion).second;
    J_ASSERT(did_create, "Tried to add a conversion twice.");
  }

  void conversion_collection::remove_conversion_from(const type_id::type_id & type) {
    auto map = reinterpret_cast<conversion_map_t*>(&m_map);
    map->erase(map->find(type));
  }

  void conversion_collection::add_conversion_from_(const conversion_definition & conversion) {
    [[maybe_unused]] bool did_create = reinterpret_cast<conversion_map_t*>(&m_map)->emplace(conversion.from, conversion.conversion).second;
    J_ASSERT(did_create, "Tried to add a conversion twice.");
  }
}
