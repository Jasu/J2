#pragma once

#include "meta/attr_value.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "meta/expr_fn.hpp"

namespace j::meta {
  struct expr_scope;

  using prop_hasser_t J_NO_DEBUG_TYPE = bool(*)(const expr_scope &, const attr_value &) noexcept;
  using prop_getter_t J_NO_DEBUG_TYPE = attr_value(*)(const expr_scope &, const attr_value &) noexcept;
  using prop_setter_t J_NO_DEBUG_TYPE = void (*)(expr_scope &, attr_value &, attr_value);

  struct property final {
    strings::const_string_view name{};
    prop_getter_t get = nullptr;
    prop_hasser_t hasser = nullptr;
    prop_setter_t setter = nullptr;

    void set(expr_scope & s, attr_value & dst, attr_value val) const;

    J_A(AI,NODISC) inline bool has(const expr_scope & s, const attr_value & v) const noexcept {
      return !hasser || hasser(s, v);
    }
  };

  using dyn_hasser_t = bool(*)(const expr_scope &, const attr_value &, strings::const_string_view) noexcept;
  using dyn_getter_t = attr_value(*)(const expr_scope &, const attr_value &, strings::const_string_view);
  using dyn_setter_t = void (*)(expr_scope &, attr_value &, strings::const_string_view, attr_value);

  struct attr_value_interface final {
    attr_value_interface(span<property> props, span<pair<strings::const_string_view, expr_fn>> methods, const attr_value_interface * parent = nullptr,
                         dyn_hasser_t dyn_has = nullptr,
                         dyn_getter_t dyn_get = nullptr,
                         dyn_setter_t dyn_set = nullptr);

    [[nodiscard]] bool has_prop(const expr_scope & s, const attr_value & v, strings::const_string_view name) const noexcept;
    [[nodiscard]] attr_value get_prop(const expr_scope & s, const attr_value & v, strings::const_string_view name) const;
    void set_prop(expr_scope & s, attr_value & v, strings::const_string_view name, attr_value value) const;

    [[nodiscard]] bool can_call(strings::const_string_view name, span<const attr_value> args) const noexcept;
    [[nodiscard]] attr_value call(expr_scope & s, strings::const_string_view name, span<attr_value> args) const;


    trivial_array<property> props;
    trivial_array<pair<strings::const_string_view, expr_fn>> methods;

    dyn_hasser_t dyn_has = nullptr;
    dyn_getter_t dyn_get = nullptr;
    dyn_setter_t dyn_set = nullptr;
    const attr_value_interface * parent = nullptr;

  private:
    [[nodiscard]] const property * find_prop(strings::const_string_view name) const noexcept;
    [[nodiscard]] const expr_fn * find_method(strings::const_string_view name, span<const attr_value> args) const noexcept;
  };
}
