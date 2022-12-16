#include "properties/path.hpp"
#include "containers/vector.hpp"
#include "exceptions/exceptions.hpp"
#include "hzd/mem.hpp"

J_DEFINE_EXTERN_VECTOR(j::properties::path_component);

namespace j::properties {
  static_assert(sizeof(uptr_t) >= sizeof(sz_t));
  static_assert(sizeof(uptr_t) >= sizeof(strings::string));

  void path_component::initialize_string(const strings::string & str) {
    ::new (&string_ref()) strings::string(str);
  }

  void path_component::initialize_string(strings::string && str) noexcept {
    ::new (&string_ref()) strings::string(static_cast<strings::string &&>(str));
  }

  [[noreturn]] void path_component::throw_not_map_key() const {
    J_THROW(exception() << message("Expected property path component to be a map key."));
  }

  [[noreturn]] void path_component::throw_not_property_name() const {
    J_THROW(exception() << message("Expected property path component to be a property name."));
  }

  [[noreturn]] void path_component::throw_not_array_index() const {
    J_THROW(exception() << message("Expected property path component to be an array index."));
  }

  path_component::path_component(const detail::property_name_tag_t &, const strings::string & rhs)
    : m_type(COMPONENT_TYPE::PROPERTY_NAME)
  {
    initialize_string(rhs);
  }

  path_component::path_component(const detail::property_name_tag_t &, strings::string && rhs) noexcept
    : m_type(COMPONENT_TYPE::PROPERTY_NAME)
  {
    initialize_string(static_cast<strings::string &&>(rhs));
  }

  path_component::path_component(const detail::map_key_tag_t &, const strings::string & rhs)
    : m_type(COMPONENT_TYPE::MAP_KEY)
  {
    initialize_string(rhs);
  }

  path_component::path_component(const detail::map_key_tag_t &, strings::string && rhs) noexcept
    : m_type(COMPONENT_TYPE::MAP_KEY)
  {
    initialize_string(static_cast<strings::string &&>(rhs));
  }
  path_component::path_component(const path_component & rhs)
    : m_type(rhs.m_type)
  {
    if (is_string()) {
      initialize_string(rhs.string_ref());
    } else {
      m_data = rhs.m_data;
    }
  }

  path_component & path_component::operator=(const path_component & rhs) {
    if (this != &rhs) {
      if (is_string()) {
        if (rhs.is_string()) {
          string_ref() = rhs.string_ref();
        } else {
          string_ref().~string();
          m_data = rhs.m_data;
        }
      } else if (rhs.is_string()) {
        initialize_string(rhs.string_ref());
      } else {
        m_data = rhs.m_data;
      }
      m_type = rhs.m_type;
    }
    return *this;
  }

  bool path_component::operator<(const path_component & rhs) const noexcept {
    if (m_type != rhs.m_type) {
      return m_type < rhs.m_type;
    }
    switch (m_type) {
    case COMPONENT_TYPE::MAP_KEY:
    case COMPONENT_TYPE::PROPERTY_NAME:
      return string_ref() < rhs.string_ref();
    case COMPONENT_TYPE::ARRAY_INDEX:
      return m_data < rhs.m_data;
    case COMPONENT_TYPE::EMPTY:
      return false;
    }
  }

  bool path_component::operator>=(const path_component & rhs) const noexcept {
    return !(rhs < *this);
  }

  bool path_component::operator<=(const path_component & rhs) const noexcept {
    return *this < rhs || *this == rhs;
  }

  bool path_component::operator>(const path_component & rhs) const noexcept {
    return !(*this < rhs || *this == rhs);
  }

  void path::clear() noexcept {
    m_data.clear();
  }

  void path::reserve(u32_t size) {
    m_data.reserve(size);
  }

  bool path::operator<(const path & rhs) const noexcept {
    u32_t sz = m_data.size();
    if (sz < rhs.m_data.size()) {
      return true;
    } else if (sz > rhs.m_data.size()) {
      return false;
    }
    while (sz--) {
      if (m_data[sz] < rhs.m_data[sz]) {
        return true;
      }
    }
    return false;
  }

  bool path::operator<=(const path & rhs) const noexcept {
    return *this == rhs || *this < rhs;
  }

  bool path::operator>(const path & rhs) const noexcept {
    return !(*this == rhs) && !(*this < rhs);
  }

  bool path::operator>=(const path & rhs) const noexcept {
    return *this < rhs;
  }

  bool path::is_ancestor_of(const path & rhs) const noexcept {
    u32_t sz = m_data.size();
    if (sz >= rhs.m_data.size()) {
      return false;
    }
    while (sz--) {
      if (m_data[sz] != rhs.m_data[sz]) {
        return false;
      }
    }
    return true;
  }

  bool path::is_ancestor_of_or_same(const path & rhs) const noexcept {
    u32_t sz = m_data.size();
    if (sz > rhs.m_data.size()) {
      return false;
    }
    while (sz--) {
      if (m_data[sz] != rhs.m_data[sz]) {
        return false;
      }
    }
    return true;
  }
}
