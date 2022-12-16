#include "properties/property_variant.hpp"

#include "exceptions/assert_lite.hpp"
#include "properties/visit_variant.hpp"
#include "containers/unsorted_string_map.hpp"
#include "containers/vector.hpp"
#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"

J_DEFINE_EXTERN_VECTOR(j::properties::property_variant);
J_DEFINE_EXTERN_UNSORTED_STRING_MAP_COPYABLE(j::properties::property_variant);

namespace j::properties {
  namespace {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc99-designator"
    static const char * invalid_type_errors[] =  {
      [(int)PROPERTY_TYPE::EMPTY] = "Empty",
      [(int)PROPERTY_TYPE::NIL] = "Expected variant to contain nil.",
      [(int)PROPERTY_TYPE::STRING] = "Expected variant to contain a string.",
      [(int)PROPERTY_TYPE::INT] = "Expected variant to contain an integer.",
      [(int)PROPERTY_TYPE::FLOAT] = "Expected variant to contain a float.",
      [(int)PROPERTY_TYPE::LIST] = "Expected variant to contain a list.",
      [(int)PROPERTY_TYPE::MAP] = "Expected variant to contain a map.",
      [(int)PROPERTY_TYPE::OBJECT] = "Expected variant to contain an object, but variants may not contain objects.",
    };
#pragma clang diagnostic pop
  }

  void property_variant::throw_empty() const {
    J_THROW("Tried to access an empty variant.");
  }

  void property_variant::throw_incorrect_type(PROPERTY_TYPE type) const {
    J_THROW(invalid_type_errors[(int)type]);
  }

  template<> void property_variant::initialize(const string_t & rhs) {
    m_type = PROPERTY_TYPE::STRING;
    new (m_data) string_t(rhs);
  }

  template<> void property_variant::initialize(string_t && rhs) {
    m_type = PROPERTY_TYPE::STRING;
    new (m_data) string_t(static_cast<string_t &&>(rhs));
  }

  template<> void property_variant::initialize(const map_t & rhs) {
    m_type = PROPERTY_TYPE::MAP;
    new (m_data) map_t(rhs);
  }

  template<> void property_variant::initialize(map_t && rhs) {
    m_type = PROPERTY_TYPE::MAP;
    new (m_data) map_t(static_cast<map_t &&>(rhs));
  }

  template<> void property_variant::initialize(const list_t & rhs) {
    m_type = PROPERTY_TYPE::LIST;
    new (m_data) list_t(rhs);
  }

  template<> void property_variant::initialize(list_t && rhs) {
    m_type = PROPERTY_TYPE::LIST;
    new (m_data) list_t(static_cast<list_t &&>(rhs));
  }

  void property_variant::free_string_content() noexcept {
    raw_reference<string_t>().~string_t();
  }

  void property_variant::free_map_content() noexcept {
    raw_reference<map_t>().~map_t();
  }

  void property_variant::free_list_content() noexcept {
    raw_reference<list_t>().~list_t();
  }

  property_variant::property_variant(const property_variant & rhs) {
    if (rhs.empty()) {
      m_type = PROPERTY_TYPE::EMPTY;
    } else {
      visit_variant(rhs, [this](auto && v) { initialize(v); });
    }
  }

  property_variant::property_variant(property_variant && rhs) noexcept {
    if (rhs.empty()) {
      m_type = PROPERTY_TYPE::EMPTY;
    } else {
      visit_variant(static_cast<property_variant &&>(rhs),
                    [this](auto && v) -> void {
                      initialize(static_cast<j::remove_cref_t<decltype(v)> &&>(v));
                    });
      rhs.reset();
    }
  }

  property_variant & property_variant::operator=(const property_variant & rhs) {
    if (&rhs != this) {
      if (rhs.empty()) {
        reset();
      } else {
        visit_variant(rhs, [this](auto && v) { *this = v; });
      }
    }
    return *this;
  }

  property_variant & property_variant::operator=(property_variant && rhs) noexcept {
    if (&rhs != this) {
      if (rhs.empty()) {
        reset();
      } else {
        visit_variant(static_cast<property_variant &&>(rhs),
                      [this](auto && v) { *this = static_cast<j::remove_cref_t<decltype(v)> &&>(v); });
        rhs.reset();
      }
    }
    return *this;
  }

  bool property_variant::operator==(const property_variant & rhs) const noexcept {
    return m_type == rhs.m_type
      && (m_type == PROPERTY_TYPE::EMPTY
          || visit_variant(rhs, [this] (auto && v) { return as_same_as(v) == v; }));
  }
}
