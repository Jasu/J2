#pragma once

#include "properties/access/typed_access.hpp"

namespace j::properties::access {
  class int_access_definition final : public typed_access_definition {
    friend class int_access;
  public:
    using get_value_t = i64_t (*)(const void*) noexcept;
    using set_value_t = void (*)(void*, i64_t value) noexcept;
  private:
    get_value_t m_get_value = nullptr;
    set_value_t m_set_value = nullptr;
  public:
    int_access_definition() noexcept;
    ~int_access_definition();

    void initialize_int_access(get_value_t get_value, set_value_t set_value);
  };

  class int_access final : public typed_access {
  public:
    using typed_access::typed_access;

    i64_t get_value() const noexcept {
      return static_cast<const int_access_definition &>(value_definition()).m_get_value(as_void_star());
    }

    void set_value(i64_t value) noexcept {
      static_cast<const int_access_definition &>(value_definition()).m_set_value(as_void_star(), value);
    }
  };
}
