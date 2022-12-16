#pragma once

#include "rendering/data_types/data_type.hpp"
#include "rendering/attribute_key.hpp"

namespace j::rendering::vertex_data {
  struct vertex_input_attribute final {
    data_types::data_type type = data_types::data_type::none;
    strings::string name;
  };

  class vertex_input_info final {
  public:
    static constexpr inline u8_t max_locations_v = 16U;

    vertex_input_attribute * begin() noexcept { return m_attributes; }
    const vertex_input_attribute * begin() const noexcept { return m_attributes; }
    vertex_input_attribute * end() noexcept { return m_attributes + max_locations_v; }
    const vertex_input_attribute * end() const noexcept { return m_attributes + max_locations_v; }

    vertex_input_attribute & operator[](u8_t location) noexcept {
      J_ASSERT(location < max_locations_v, "Out of bounds");
      return m_attributes[location];
    }

    const vertex_input_attribute & operator[](u8_t location) const noexcept {
      J_ASSERT(location < max_locations_v, "Out of bounds");
      return m_attributes[location];
    }

    vertex_input_attribute & operator[](const strings::const_string_view & str) noexcept {
      for (u8_t i = 0; i < max_locations_v; ++i) {
        if (m_attributes[i].name == str) {
          return m_attributes[i];
        }
      }
      J_THROW("Out of bounds");
    }

    const vertex_input_attribute & operator[](const strings::const_string_view & str) const noexcept {
      return const_cast<vertex_input_info *>(this)->operator[](str);
    }

    vertex_input_attribute & operator[](const attribute_key & key) noexcept {
      if (key.is_name()) {
        return operator[](key.name());
      } else {
        return operator[](key.index());
      }
    }

    u8_t location_of(const attribute_key & key) const noexcept {
      return &operator[](key) - m_attributes;
    }

    const vertex_input_attribute & operator[](const attribute_key & key) const noexcept {
      return const_cast<vertex_input_info*>(this)->operator[](key);
    }

  private:
    vertex_input_attribute m_attributes[max_locations_v];
  };
}
