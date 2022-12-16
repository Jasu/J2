#pragma once

#include "rendering/data_types/get_data_type.hpp"
#include "hzd/mem.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::data_types {
  class typed_value final {
  public:
    constexpr typed_value() noexcept = default;

    template<typename T, typename = j::enable_if_t<!j::is_same_v<T, typed_value>>>
    explicit typed_value(const T & value) noexcept
      : m_data_type(data_type_v<T>)
    {
      constexpr u8_t sz = packed_size(data_type_v<T>);
      if constexpr (sz > sizeof(m_buffer)) {
        void * ptr = ::j::allocate(sz);
        ::j::memcpy(ptr, &value, sz);
        m_buffer[0] = reinterpret_cast<uptr_t>(ptr);
      } else {
        ::j::memcpy(m_buffer, &value, sz);
      }
    }

    template<typename T, data_type Type = data_type_v<T>>
    T & as() noexcept {
      J_ASSERT(Type == m_data_type, "Data type mismatch");
      if constexpr (packed_size(Type) > sizeof(m_buffer)) {
        return *reinterpret_cast<T*>(m_buffer[0]);
      } else {
        return *reinterpret_cast<T*>(m_buffer);
      }
    }

    template<typename T, data_type Type = data_type_v<T>>
    const T & as() const noexcept {
      return const_cast<typed_value*>(this)->as<T>();
    }

    typed_value(typed_value && rhs) noexcept
      : m_data_type(rhs.m_data_type),
        m_buffer{rhs.m_buffer[0], rhs.m_buffer[1]}
    { rhs.m_data_type = data_type::none; }

    typed_value(const typed_value & rhs)
    { initialize_from(rhs); }

    typed_value & operator=(const typed_value & rhs) {
      if (J_LIKELY(this != &rhs)) {
        release();
        initialize_from(rhs);
      }
      return *this;
    }

    typed_value & operator=(typed_value && rhs) noexcept {
      if (J_LIKELY(this != &rhs)) {
        release();
        m_data_type = rhs.m_data_type;
        m_buffer[0] = rhs.m_buffer[0];
        m_buffer[1] = rhs.m_buffer[1];
        rhs.m_data_type = data_type::none;
      }
      return *this;
    }

    ~typed_value() {
      release();
    }

    J_INLINE_GETTER data_type type() const noexcept
    { return m_data_type; }

    J_INLINE_GETTER u8_t size() const noexcept
    { return packed_size(m_data_type); }

    J_INLINE_GETTER void * data() noexcept {
      return size() > sizeof(m_buffer)
        ? reinterpret_cast<void*>(m_buffer[0])
        : reinterpret_cast<void*>(m_buffer);
    }

    J_INLINE_GETTER const void * data() const noexcept {
      return size() > sizeof(m_buffer)
        ? reinterpret_cast<const void*>(m_buffer[0])
        : reinterpret_cast<const void*>(m_buffer);
    }
  private:
    void initialize_from(const typed_value & rhs) {
      m_data_type = rhs.m_data_type;
      const u8_t sz = size();
      if (sz > sizeof(m_buffer)) {
        void * ptr = ::j::allocate(sz);
        ::j::memcpy(ptr, reinterpret_cast<const void *>(rhs.m_buffer[0]), sz);
        m_buffer[0] = reinterpret_cast<uptr_t>(ptr);
      } else {
        m_buffer[0] = rhs.m_buffer[0];
        m_buffer[1] = rhs.m_buffer[1];
      }
    }

    void release() noexcept {
      if (size() > sizeof(m_buffer)) {
        ::j::free(reinterpret_cast<void*>(m_buffer[0]));
        m_data_type = data_type::none;
      }
    }
    data_type m_data_type = data_type::none;
    uptr_t m_buffer[2] = { 0UL, 0UL };
  };
}
