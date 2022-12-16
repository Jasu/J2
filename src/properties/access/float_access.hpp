#pragma once

#include "properties/access/typed_access.hpp"

namespace j::properties::access {
  class float_access_definition final : public typed_access_definition {
    friend class float_access;
  public:
    using get_value_t = double (*)(const void*) noexcept;
    using set_value_t = void (*)(void*, double value) noexcept;
  private:
    get_value_t m_get_value = nullptr;
    set_value_t m_set_value = nullptr;
  public:
    float_access_definition() noexcept;

    ~float_access_definition();

    void initialize_float_access(get_value_t get_value, set_value_t set_value);
  };

  class float_access final : public typed_access {
  public:
    using typed_access::typed_access;

    double get_value() const noexcept {
      return static_cast<const float_access_definition &>(value_definition()).m_get_value(as_void_star());
    }

    void set_value(double value) noexcept {
      static_cast<const float_access_definition &>(value_definition()).m_set_value(as_void_star(), value);
    }
  };
}
