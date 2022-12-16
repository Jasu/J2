#include "properties/visiting/visit_path.hpp"
#include "exceptions/assert_lite.hpp"
#include "strings/string.hpp"

namespace j::properties::visiting {
  visit_path::visit_path() noexcept
    : m_type(COMPONENT_TYPE::EMPTY),
      m_parent(nullptr)
  {
  }

  const visit_path visit_path::root{};

  [[noreturn]] void visit_path::throw_not_map_key() const {
    J_THROW("Expected property path component to be a map key.");
  }

  [[noreturn]] void visit_path::throw_not_property_name() const {
    J_THROW("Expected property path component to be a property name.");
  }

  [[noreturn]] void visit_path::throw_not_array_index() const {
    J_THROW("Expected property path component to be an array index.");
  }

  path visit_path::to_path(u32_t index) const {
    path p;
    if (m_parent) {
      p = m_parent->to_path(index + 1);
    } else {
      p.reserve(index);
      return p;
    }
    switch (m_type) {
    case COMPONENT_TYPE::MAP_KEY:
      p.emplace_back(map_key_tag,
                     strings::string(*reinterpret_cast<const strings::const_string_view *>(m_contents.name)));
      break;
    case COMPONENT_TYPE::PROPERTY_NAME:
      p.emplace_back(property_name_tag,
                     strings::string(*reinterpret_cast<const strings::const_string_view *>(m_contents.name)));
      break;
    case COMPONENT_TYPE::ARRAY_INDEX:
      p.emplace_back(array_index_tag, m_contents.array_index);
      break;
    case COMPONENT_TYPE::EMPTY:
      J_THROW("Empty path with a parent.");
    }
    return p;
  }

  visit_path::operator path() const {
    return to_path(0);
  }
}
