#pragma once

#include "meta/expr_fn_def.hpp"
#include "hzd/callable_traits.hpp"
#include "meta/attr_value_interface.hpp"

namespace j::meta {
  template<typename To>
  J_A(ND,HIDDEN) inline decltype(auto) attr_value_to(const attr_value & val) {
    return val.as<To>();
  }

  template<ActuallyIntegral To>
  J_A(AI,ND,HIDDEN) inline To attr_value_to(const attr_value & val) {
    return (To)val.as<i64_t>();
  }

  template<>
  J_A(AI,ND,HIDDEN) inline decltype(auto) attr_value_to<const attr_value &>(const attr_value & val) {
    return val;
  }


  template<auto MemPtr>
  J_A(ND,HIDDEN,NODISC) inline attr_value member_prop_getter(const expr_scope &, const attr_value & obj) noexcept {
    using t J_NO_DEBUG_TYPE = member_type_t<decltype(MemPtr)>;
    if constexpr (is_same_v<t, attr_value>) {
      return obj.as<const member_class_t<decltype(MemPtr)> &>().*MemPtr;
    } else {
      return attr_value(obj.as<const member_class_t<decltype(MemPtr)> &>().*MemPtr);
    }
  }

  template<auto MemFn>
  J_A(ND,HIDDEN,NODISC) inline attr_value member_fn_getter(const expr_scope &, const attr_value & obj) noexcept {
    using result_t J_NO_DEBUG_TYPE = remove_cref_t<callable_result_t<decltype(MemFn)>>;
    if constexpr (is_member_function_pointer_v<decltype(MemFn)>) {
      if constexpr (is_same_v<result_t, attr_value>) {
        return (attr_value_to<const memfn_class_t<decltype(MemFn)> &>(obj).*MemFn)();
      } else {
        return attr_value((attr_value_to<const memfn_class_t<decltype(MemFn)> &>(obj).*MemFn)());
      }
    } else if constexpr (is_same_v<result_t, attr_value>) {
      return MemFn(attr_value_to<const callable_arg_t<decltype(MemFn), 0U> &>(obj));
    } else {
      return attr_value(MemFn(attr_value_to<const callable_arg_t<decltype(MemFn), 0U> &>(obj)));
    }
  }

  template<auto Fn>
  J_A(ND,HIDDEN,NODISC) inline bool fn_hasser(const expr_scope &, const attr_value & obj) noexcept {
    if constexpr (is_member_function_pointer_v<decltype(Fn)>) {
      return (attr_value_to<const memfn_class_t<decltype(Fn)> &>(obj).*Fn)();
    } else {
      return Fn(attr_value_to<callable_arg_t<decltype(Fn), 0U>>(obj));
    }
  }

  template<auto MemPtr>
  J_A(ND,HIDDEN) inline void member_prop_setter(expr_scope &, attr_value & obj, attr_value val) noexcept {
    using t J_NO_DEBUG_TYPE = member_type_t<decltype(MemPtr)>;
    if constexpr (is_same_v<t, attr_value>) {
      obj.as<member_class_t<decltype(MemPtr)> &>().*MemPtr = static_cast<attr_value &&>(val);
    } else {
      obj.as<member_class_t<decltype(MemPtr)> &>().*MemPtr = attr_value_to<member_type_t<decltype(MemPtr)>>(val);
    }
  }

  template<auto MemPtr>
  consteval property make_prop(const char * J_NOT_NULL name) {
    return {
      name,
      &member_prop_getter<MemPtr>,
      nullptr,
      &member_prop_setter<MemPtr>
    };
  }

  template<auto MemPtr>
  consteval property make_prop_ro(const char * J_NOT_NULL name) {
    return {name, &member_prop_getter<MemPtr>};
  }

  template<auto MemFn, auto HasFn = nullptr>
  consteval property make_getter(const char * J_NOT_NULL name) {
    if constexpr (HasFn != nullptr) {
      return property{name, &member_fn_getter<MemFn>, &fn_hasser<HasFn>};
    } else {
      return property{name, &member_fn_getter<MemFn>};
    }
  }

  template<typename Fn, typename... Overrides>
  inline consteval pair<strings::const_string_view, expr_fn> make_method(const char * J_NOT_NULL name, Fn fn, const Overrides & ... ors) noexcept {
    return pair{strings::const_string_view(name), make_expr_fn(fn, ors...)};
  }
}
