#pragma once

#include "properties/path.hpp"
#include "strings/string_view.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::properties::visiting {
  class visit_path {
    COMPONENT_TYPE m_type;
    union {
      u8_t name[sizeof(strings::const_string_view)];
      sz_t array_index;
    } m_contents;
    const visit_path * m_parent;

    [[noreturn]] void throw_not_map_key() const;
    [[noreturn]] void throw_not_property_name() const;
    [[noreturn]] void throw_not_array_index() const;

    path to_path(u32_t index = 0) const;

    visit_path() noexcept;

  public:
    static const visit_path root;

    visit_path(COMPONENT_TYPE type, const visit_path & parent)
      : m_type(type),
        m_parent(&parent)
    {
      J_ASSERT(type != COMPONENT_TYPE::EMPTY, "Cannot access root with parent.");
    }

    void set_string(const strings::const_string_view & rhs) noexcept {
      *reinterpret_cast<strings::const_string_view *>(&m_contents.name[0]) = rhs;
    }

    void set_array_index(sz_t index) noexcept {
      m_contents.array_index = index;
    }

    bool empty() const noexcept {
      return m_type == COMPONENT_TYPE::EMPTY;
    }

    COMPONENT_TYPE type() const noexcept {
      return m_type;
    }

    bool is_root() const noexcept {
      return m_type == COMPONENT_TYPE::EMPTY;
    }

    bool is_map_key() const noexcept {
      return m_type == COMPONENT_TYPE::MAP_KEY;
    }

    bool is_property_name() const noexcept {
      return m_type == COMPONENT_TYPE::PROPERTY_NAME;
    }

    bool is_array_index() const noexcept {
      return m_type == COMPONENT_TYPE::ARRAY_INDEX;
    }

    const strings::const_string_view & as_property_name_noexcept() const noexcept {
      return *reinterpret_cast<const strings::const_string_view *>(m_contents.name);
    }

    const strings::const_string_view & as_property_name() const {
      if (m_type != COMPONENT_TYPE::PROPERTY_NAME) {
        throw_not_property_name();
      }
      return *reinterpret_cast<const strings::const_string_view *>(m_contents.name);
    }

    const strings::const_string_view & as_map_key_noexcept() const noexcept {
      return *reinterpret_cast<const strings::const_string_view *>(m_contents.name);
    }

    const strings::const_string_view & as_map_key() const {
      if (m_type != COMPONENT_TYPE::MAP_KEY) {
        throw_not_map_key();
      }
      return *reinterpret_cast<const strings::const_string_view *>(m_contents.name);
    }

    sz_t as_array_index() const {
      if (m_type != COMPONENT_TYPE::ARRAY_INDEX) {
        throw_not_array_index();
      }
      return m_contents.array_index;
    }

    sz_t as_array_index_noexcept() const noexcept {
      return m_contents.array_index;
    }

    const visit_path * parent() const noexcept {
      return m_parent;
    }

    explicit operator path() const;
  };

  inline bool operator==(const visit_path & lhs, const path & rhs) noexcept {
    sz_t sz = rhs.size();
    const visit_path * p = &lhs;
    for (; p && sz; p = p->parent()) {
      --sz;
      if (p->type() != rhs[sz].type()) {
        return false;
      }
      switch (p->type()) {
      case COMPONENT_TYPE::MAP_KEY:
        if (p->as_map_key_noexcept() != rhs[sz].as_map_key_noexcept()) {
          return false;
        }
        break;
      case COMPONENT_TYPE::PROPERTY_NAME:
        if (p->as_property_name_noexcept() != rhs[sz].as_property_name_noexcept()) {
          return false;
        }
        break;
      case COMPONENT_TYPE::ARRAY_INDEX:
        if (p->as_array_index_noexcept() != rhs[sz].as_array_index_noexcept()) {
          return false;
        }
        break;
      case COMPONENT_TYPE::EMPTY:
        // Path ended before
        return false;
      }
    }
    return p && p->type() == COMPONENT_TYPE::EMPTY;
  }

  inline bool operator==(const visit_path & lhs, const visit_path & rhs) noexcept {
    if (lhs.type() != rhs.type()) {
      return false;
    }
    switch (lhs.type()) {
    case COMPONENT_TYPE::MAP_KEY:
      if (lhs.as_map_key_noexcept() != rhs.as_map_key_noexcept()) {
        return false;
      }
      break;
    case COMPONENT_TYPE::PROPERTY_NAME:
      if (lhs.as_property_name_noexcept() != rhs.as_property_name_noexcept()) {
        return false;
      }
      break;
    case COMPONENT_TYPE::ARRAY_INDEX:
      if (lhs.as_array_index_noexcept() != rhs.as_array_index_noexcept()) {
        return false;
      }
      break;
    case COMPONENT_TYPE::EMPTY:
      return true;
    }
    return *lhs.parent() == *rhs.parent();
  }

  inline bool operator==(const path & lhs, const visit_path & rhs) noexcept {
    return operator==(rhs, lhs);
  }

  inline bool operator!=(const visit_path & lhs, const visit_path & rhs) noexcept {
    return !operator==(lhs, rhs);
  }

  inline bool operator!=(const visit_path & lhs, const path & rhs) noexcept {
    return !operator==(lhs, rhs);
  }

  inline bool operator!=(const path & lhs, const visit_path & rhs) noexcept {
    return !operator==(lhs, rhs);
  }
}
