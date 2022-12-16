#pragma once

#include "rendering/data_types/data_type.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::data_types {
  struct struct_field {
    data_type type = data_type::none;
    u16_t offset = 0U;
  };

  class struct_description {
  public:
    static constexpr inline u8_t max_size_v = 16U;

    struct_description() noexcept = default;

    template<u8_t Sz>
    struct_description(const struct_field (& fields)[Sz]) noexcept {
      static_assert(Sz <= max_size_v);

      u8_t i = 0;
      for (; i < Sz; ++i) {
        m_fields[i] = fields[i];
      }
    }

    u8_t size() const noexcept {
      u8_t i = 0U;
      for (; i < max_size_v && m_fields[i].type != data_type::none; ++i) { }
      return i;
    }

    const struct_field & operator[](u8_t i) const {
      J_ASSERT(i < max_size_v && m_fields[i].type != data_type::none, "Index out of bounds.");
      return m_fields[i];
    }

    const struct_field * begin() const noexcept {
      return m_fields;
    }
    struct_field * begin() noexcept {
      return m_fields;
    }

    const struct_field * end() const noexcept {
      return m_fields + size();
    }
    struct_field * end() noexcept {
      return m_fields + size();
    }
  private:
    struct_field m_fields[max_size_v];
  };
}
