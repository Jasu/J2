#include "attr_value_interface.hpp"
#include "meta/expr_scope.hpp"
#include "containers/trivial_array.hpp"
#include "exceptions/assert.hpp"

namespace j::meta {

  void property::set(expr_scope & s, attr_value & dst, attr_value val) const {
    J_REQUIRE(setter, "Property {} is not settable.", name);
    setter(s, dst, static_cast<attr_value &&>(val));
  }

  attr_value_interface::attr_value_interface(span<property> props,
                                             span<pair<strings::const_string_view, expr_fn>> methods,
                                             const attr_value_interface * parent,
                                             dyn_hasser_t dyn_has,
                                             dyn_getter_t dyn_get,
                                             dyn_setter_t dyn_set)
    : props(containers::move, props),
      methods(containers::move, methods),
      dyn_has(dyn_has),
      dyn_get(dyn_get),
      dyn_set(dyn_set),
      parent(parent)
  {
    J_ASSERT(!dyn_has == !dyn_get && (!dyn_set || dyn_get));
  }


  [[nodiscard]] bool attr_value_interface::has_prop(const expr_scope & s, const attr_value & v, strings::const_string_view name) const noexcept {
    if (auto p = find_prop(name)) {
      return p->has(s, v);
    }
    if (dyn_has && dyn_has(s, v, name)) {
      return true;
    }
    return parent ? parent->has_prop(s, v, name) : false;
  }

  [[nodiscard]] attr_value attr_value_interface::get_prop(const expr_scope & s, const attr_value & v, strings::const_string_view name) const {
    if (auto p = find_prop(name)) {
      return p->get(s, v);
    }
    if (dyn_has && (!parent || dyn_has(s, v, name))) {
      return dyn_get(s, v, name);
    }
    if (!parent) {
      s.throw_prop_not_found_error(name, v);
    }
    return parent->get_prop(s, v, name);
  }

  void attr_value_interface::set_prop(expr_scope & s, attr_value & v, strings::const_string_view name, attr_value value) const {
    if (auto p = find_prop(name)) {
      p->set(s, v, static_cast<attr_value &&>(value));
      return;
    }
    if (dyn_set) {
      dyn_set(s, v, name, static_cast<attr_value &&>(value));
      return;
    }
    if (!parent) {
      s.throw_prop_not_found_error(name, v);
    }
    return parent->set_prop(s, v, name, static_cast<attr_value &&>(value));
  }

  [[nodiscard]] bool attr_value_interface::can_call(strings::const_string_view name, span<const attr_value> args) const noexcept {
    if (auto m = find_method(name, args)) {
      return m->can_invoke(args.begin(), args.size());
    }
    return parent ? parent->can_call(name, args) : false;
  }

  [[nodiscard]] attr_value attr_value_interface::call(expr_scope & s, strings::const_string_view name, span<attr_value> args) const {
    if (auto m = find_method(name, args)) {
      J_REQUIRE(m->can_invoke(args.begin(), args.size()), "Method {} is not callable with the arguments given in {}.", name, args[0].type);
      return (*m)(s, args.begin(), args.size());
    }
    J_REQUIRE(parent, "Method {} not found in {}.", name, args[0].type);
    return parent->call(s, name, args);
  }

  [[nodiscard]] const property * attr_value_interface::find_prop(strings::const_string_view name) const noexcept {
    for (auto & v : props) {
      if (v.name == name) {
        return &v;
      }
    }
    return nullptr;
  }

  [[nodiscard]] const expr_fn * attr_value_interface::find_method(strings::const_string_view name, span<const attr_value> args) const noexcept {
    for (auto & v : methods) {
      if (v.first == name && v.second.can_invoke(args.begin(), args.size())) {
        return &v.second;
      }
    }
    return nullptr;
  }
}
