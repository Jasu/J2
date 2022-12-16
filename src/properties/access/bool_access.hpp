#pragma once

#include "properties/access/typed_access.hpp"

namespace j::properties::access {
  class bool_access_definition final : public typed_access_definition {
    friend class bool_access;
  public:
    using get_value_t = bool (*)(const void*) noexcept;
    using set_value_t = void (*)(void*, bool value);
  private:
    get_value_t m_get_value = nullptr;
    set_value_t m_set_value = nullptr;
  public:
    bool_access_definition() noexcept;

    ~bool_access_definition();

    void initialize_bool_access(get_value_t get_value, set_value_t set_value);
  };

  class bool_access final : public typed_access {
  public:
    using typed_access::typed_access;

    bool get_value() const noexcept {
      return static_cast<const bool_access_definition &>(value_definition()).m_get_value(as_void_star());
    }

    void set_value(bool value) {
      return static_cast<const bool_access_definition &>(value_definition()).m_set_value(as_void_star(), value);
    }
  };
}
