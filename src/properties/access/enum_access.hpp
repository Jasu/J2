#pragma once

#include "properties/access/typed_access.hpp"

namespace j::properties::access {
  class enum_access_definition final : public typed_access_definition {
    friend class enum_access;
  public:
    using get_value_t = u64_t (*)(const void*) noexcept;
    using set_value_t = void (*)(void*, u64_t value);
  private:
    get_value_t m_get_value = nullptr;
    set_value_t m_set_value = nullptr;
  public:
    void initialize_enum_access(get_value_t get_value, set_value_t set_value);
  };

  class enum_access final : public typed_access {
  public:
    using typed_access::typed_access;

    u64_t value() const noexcept {
      return static_cast<const enum_access_definition &>(value_definition()).m_get_value(as_void_star());
    }

    template<typename T>
    J_HIDDEN J_ALWAYS_INLINE T value() const noexcept {
      return static_cast<T>(static_cast<const enum_access_definition &>(value_definition()).m_get_value(as_void_star()));
    }

    template<typename T>
    J_HIDDEN J_ALWAYS_INLINE void set_value(T value) {
      return static_cast<const enum_access_definition &>(value_definition()).m_set_value(as_void_star(), (u64_t)value);
    }
  };
}
