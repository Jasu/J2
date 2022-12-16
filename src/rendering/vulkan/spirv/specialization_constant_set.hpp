#pragma once

#include "strings/string.hpp"
#include "rendering/attribute_key.hpp"
#include "rendering/data_types/data_type.hpp"

namespace j::rendering::vulkan::inline spirv {
  struct specialization_constant_set final {
    static constexpr inline u8_t max_size_v = 8U;

    void set(u8_t index, data_types::data_type type, const strings::string & name) {
      J_ASSERT(index < max_size_v && m_values[index].type == data_types::data_type::none);
      m_values[index] = { type, name };
    }

    u8_t index_of(strings::const_string_view name) const noexcept {
      for (u8_t i = 0U; i < max_size_v; ++i) {
        if (m_values[i].name == name) {
          J_ASSERT_NOT_NULL(m_values[i].type);
          return i;
        }
      }
      J_FAIL("Specialization constant not found.");
    }

    u8_t index_of(const attribute_key & key) const noexcept {
      return key.is_index() ? key.index() : index_of(key.name());
    }

    const data_types::data_type & operator[](const attribute_key & key) const noexcept {
      const u8_t idx = index_of(key);
      J_ASSERT(m_values[idx].type);
      return m_values[idx].type;
    }

    const data_types::data_type & operator[](u8_t index) const noexcept {
      J_ASSERT(index < max_size_v && (bool)m_values[index].type);
      return m_values[index].type;
    }

  private:
    struct value final {
      data_types::data_type type = data_types::data_type::none;
      strings::string name;
    };
    value m_values[max_size_v];
  };
}
