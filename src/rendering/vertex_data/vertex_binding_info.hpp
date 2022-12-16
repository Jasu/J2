#pragma once

#include "rendering/attribute_key.hpp"
#include "rendering/data_types/get_data_type.hpp"

namespace j::rendering::vertex_data {
  enum class scaling : u8_t {
    normalized = 0U,
    integer    = 1U,
    scaled     = 2U,
  };

  class vertex_input_info;

  struct vertex_attribute_target final {
    u16_t offset = 0U;
    data_types::data_type type = data_types::data_type::none;
    enum scaling scaling = scaling::normalized;

    J_ALWAYS_INLINE constexpr vertex_attribute_target() noexcept = default;

    J_ALWAYS_INLINE constexpr vertex_attribute_target(u16_t offset,
                                                      data_types::data_type type,
                                                      enum scaling s) noexcept
      : offset(offset), type(type), scaling(s)
    { }

    template<typename T, typename U>
    J_ALWAYS_INLINE constexpr explicit vertex_attribute_target(T U::* ptr) noexcept
      : offset((u16_t)__builtin_bit_cast(ssz_t, reinterpret_cast<char*>(&(static_cast<U*>(nullptr)->*ptr)))),
        type(data_types::data_type_v<T>)
    {
    }
  };


  struct vertex_attribute_binding final {
    attribute_key key;
    vertex_attribute_target target;

    J_ALWAYS_INLINE constexpr vertex_attribute_binding() noexcept = default;

    template<typename Key, typename Target>
    J_ALWAYS_INLINE vertex_attribute_binding(Key && k, Target && t)
      : key(static_cast<Key &&>(k)),
        target(static_cast<Target &&>(t))
    {
    }
  };

  class vertex_binding_info final {
  public:
    static constexpr inline u8_t max_locations_v = 16U;

    J_ALWAYS_INLINE vertex_binding_info() noexcept = default;

    template<u8_t N>
    J_ALWAYS_INLINE constexpr vertex_binding_info(const vertex_attribute_binding (& attributes)[N], u16_t stride = 0U) noexcept
      : m_size(N),
        m_stride(stride)
    {
      static_assert(N < max_locations_v);
      for (u8_t i = 0; i < N; ++i) {
        m_bindings[i] = attributes[i];
      }

      if (!m_stride) {
        for (const auto & b : *this) {
          m_stride = ::j::max(m_stride, b.target.offset + data_types::packed_size(b.target.type));
        }
      }
    }

    J_INLINE_GETTER u16_t size() const noexcept
    { return m_size; }

    J_INLINE_GETTER u16_t stride() const noexcept
    { return m_stride; }

    J_INLINE_GETTER_NONNULL vertex_attribute_binding * begin() noexcept
    { return m_bindings; }
    J_INLINE_GETTER_NONNULL const vertex_attribute_binding * begin() const noexcept
    { return m_bindings; }
    J_INLINE_GETTER_NONNULL vertex_attribute_binding * end() noexcept
    { return m_bindings + m_size; }
    J_INLINE_GETTER_NONNULL const vertex_attribute_binding * end() const noexcept
    { return m_bindings + m_size; }
  private:
    vertex_attribute_binding m_bindings[max_locations_v];
    u16_t m_size = 0U;
    u16_t m_stride = 0U;
  };

  class matched_vertex_binding_info;

  struct matched_vertex_binding_info_iterator final {
    const matched_vertex_binding_info * m_ptr = nullptr;
    u8_t index = 0U;

    J_INLINE_GETTER bool operator==(const matched_vertex_binding_info_iterator & rhs) const {
      J_ASSUME(m_ptr == rhs.m_ptr);
      return index == rhs.index;
    }

    J_ALWAYS_INLINE matched_vertex_binding_info_iterator & operator++() noexcept {
      ++index;
      fast_forward();
      return *this;
    }

    J_ALWAYS_INLINE matched_vertex_binding_info_iterator operator++(int) noexcept {
      const auto result(*this);
      return operator++(), result;
    }

    J_INLINE_GETTER_NONNULL const vertex_attribute_target * operator->() const noexcept;

    J_INLINE_GETTER const vertex_attribute_target & operator*() const noexcept {
      return *operator->();
    }

    void fast_forward() noexcept;
  };

  class matched_vertex_binding_info final {
  public:
    static constexpr inline u8_t max_locations_v = 16U;
    matched_vertex_binding_info(const vertex_binding_info & binding_info,
                                const vertex_input_info & input_info);

    J_INLINE_GETTER matched_vertex_binding_info_iterator begin() const noexcept {
      matched_vertex_binding_info_iterator it{this, 0};
      it.fast_forward();
      return it;
    }
    matched_vertex_binding_info_iterator end() const noexcept {
      return matched_vertex_binding_info_iterator{this, max_locations_v};
    }
  private:
    vertex_attribute_target m_bindings[max_locations_v];
    u16_t m_stride = 0U;
    friend struct matched_vertex_binding_info_iterator;
  };

  inline const vertex_attribute_target * matched_vertex_binding_info_iterator::operator->() const noexcept {
    J_ASSUME(index < matched_vertex_binding_info::max_locations_v);
    return m_ptr->m_bindings + index;
  }

  inline void matched_vertex_binding_info_iterator::fast_forward() noexcept {
    for (; index < matched_vertex_binding_info::max_locations_v
           && m_ptr->m_bindings[index].type == data_types::data_type::none;
         ++index) { }
  }
}
