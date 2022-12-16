#pragma once

#include "meta/expr_scope.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/expr.hpp"
#include "containers/trivial_array.hpp"

namespace j::meta {
  template<typename T>
  struct arg_parser;

  template<>
  struct J_TYPE_HIDDEN arg_parser<attr_value> final {
    constexpr inline static attr_type_mask types J_A(ND) = attr_mask_any;
    using type = attr_value;
    J_A(AI,ND,NODISC) static inline const attr_value & parse(expr_scope &, const attr_value & v) noexcept {
      return v;
    }
  };

  template<ActuallyIntegral T>
  struct J_TYPE_HIDDEN arg_parser<T> final {
    constexpr inline static attr_type_mask types J_A(ND) = attr_mask_int_or_enum;
    using type = T;
    J_A(AI,ND,NODISC) static inline T parse(expr_scope &, const attr_value & v) {
      return v.is_enum() ? (T)v.enum_val.value : (T)v.integer;
    }
  };

  template<Node T>
  struct J_TYPE_HIDDEN arg_parser<T> final {
    constexpr inline static attr_type_mask types J_A(ND) = mask_v<T>;
    using type = const T *;
    J_A(AI,ND,NODISC) static inline const T * parse(expr_scope &, const attr_value & v) {
      return reinterpret_cast<const T*>(v.node);
    }
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<wrapped_node_set> final {
    constexpr inline static attr_type_mask types J_A(ND) = attr_mask_node_set;
    using type = wrapped_node_set;
    J_A(AI,ND,NODISC) static inline wrapped_node_set parse(expr_scope &, const attr_value & v) {
      return v.as_node_set();
    }
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<span<const attr_value>> final {
    constexpr inline static attr_type_mask types J_A(ND) = attr_mask_collection;
    using type = trivial_array_copyable<attr_value>;
    J_A(ND,NODISC) static trivial_array_copyable<attr_value> parse(expr_scope &, const attr_value & v);
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<codegen_template> final {
    constexpr inline static attr_type_mask types J_A(ND) = to_attr_mask(attr_tpl, attr_str, attr_bool, attr_int);
    using type = codegen_template;
    J_A(ND,NODISC) static codegen_template parse(expr_scope &, const attr_value & v);
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<strings::string> {
    constexpr inline static attr_type_mask types J_A(ND) = to_attr_mask(attr_str, attr_int);
    using type = strings::string;
    J_A(ND,NODISC) static strings::string parse(expr_scope &, const attr_value & v);
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<lisp::id_name> {
    constexpr inline static attr_type_mask types J_A(ND) = to_attr_mask(attr_id);
    using type = lisp::id_name;
    J_A(ND,NODISC) static lisp::id_name parse(expr_scope &, const attr_value & v);
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<lisp::id_name_ref> {
    constexpr inline static attr_type_mask types J_A(ND) = to_attr_mask(attr_id);
    using type = lisp::id_name_ref;
    J_A(ND,NODISC) static lisp::id_name_ref parse(expr_scope &, const attr_value & v);
  };

  template<>
  struct J_TYPE_HIDDEN arg_parser<bool> final {
    constexpr inline static attr_type_mask types J_A(ND) = attr_mask_any;
    using type = bool;
    J_A(AI,ND,NODISC) static inline bool parse(expr_scope &, const attr_value & v) {
      return !v.value_empty();
    }
  };

  template<typename T> struct J_TYPE_HIDDEN arg_parser_map { using type J_NO_DEBUG_TYPE = arg_parser<T>; };
  template<> struct J_TYPE_HIDDEN arg_parser_map<strings::const_string_view> { using type J_NO_DEBUG_TYPE = arg_parser<strings::string>; };

  template<typename T> using arg_parser_t = typename arg_parser_map<remove_ptr_t<remove_cref_t<T>>>::type;
  template<typename T>
  struct arg_wrapper final {
    using parser_t J_NO_DEBUG_TYPE = arg_parser_t<T>;

    J_A(AI) constexpr inline static arg_init make_arg_init(arg_init_flags f) noexcept {
      return arg_init{parser_t::types, false, f};
    }

    J_A(NODISC,ND,AI) inline static decltype(auto) get_arg(const arg_settings &, expr_scope & scope, attr_value * J_NOT_NULL args, u32_t) {
      return parser_t::parse(scope, *args);
    }
  };

  template<typename T>
  struct arg_wrapper<span<T>> final {
    using parser_t J_NO_DEBUG_TYPE = arg_parser_t<T>;
    using elem_type = typename parser_t::type;

    J_A(AI) constexpr inline static arg_init make_arg_init(arg_init_flags f) noexcept {
      return {parser_t::types, true, f};
    }

    J_A(NODISC) inline static trivial_array_copyable<elem_type> get_arg(arg_settings s, expr_scope & scope, attr_value * J_AA(NOALIAS) args, u32_t nargs) {
      if (s.is_optional() && (!nargs || !*args)) {
        return {};
      }
      constexpr bool accepts_node_set_v J_A(ND) = is_one_of_v<elem_type, attr_value, term, rule, val_type, node>;

      u32_t sz = 0U;
      u32_t max = max = s.is_rest() ? nargs : 1U;
      if (s.accepts_array()) {
        for (u32_t i = 0; i < max; ++i) {
          if (args[i].type == attr_tuple) {
            sz += args[i].tuple->size();
            continue;
          }
          if constexpr (accepts_node_set_v) {
            if (s.types & node_set_attr_mask(args[i].type)) {
              sz += args[i].node_set.size();
              continue;
            }
          }
          ++sz;
        }
      } else {
        sz = max;
      }

      trivial_array_copyable<elem_type> result{containers::uninitialized, sz};

      for (u32_t i = 0U; i < max; ++i) {
        if (s.accepts_array()) {
          if (args[i].type == attr_tuple) {
            for (auto & v : *args[i].tuple) {
              result.initialize_element(parser_t::parse(scope, v));
            }
            continue;
          }

          if constexpr (accepts_node_set_v) {
            if (s.types & node_set_attr_mask(args[i].type)) {
              for (auto & v : args[i].as_node_set().iterate(scope.root->mod)) {
                result.initialize_element(&v);
              }
              continue;
            }
          }
        }
        result.initialize_element(parser_t::parse(scope, args[i]));
      }

      return result;
    }
  };

  template<typename Fn>
  struct expr_function_wrapper;

  template<typename Result, typename... Args>
  struct J_TYPE_HIDDEN expr_function_wrapper<Result (*)(expr_scope &, Args...)> final {
    inline static attr_value call(const expr_fn & fn, expr_scope & scope, attr_value * args, u32_t nargs) {
      return do_call(fn, scope, args, nargs, make_index_sequence<sizeof...(Args), u32_t>());
    }

  private:
    template<u32_t... Is>
      J_A(AI,ND,NODISC,HIDDEN) static inline attr_value do_call(const expr_fn & f, expr_scope & scope, attr_value * J_AA(NOALIAS) args, u32_t nargs, integer_sequence<u32_t, Is...>) {
      if constexpr (is_same_v<Result, attr_value>) {
        return ((Result (*)(expr_scope &, Args...))f.userdata)(
          scope,
          arg_wrapper<remove_cref_t<Args>>::get_arg(f.arg_settings_at(Is), scope, args + Is, nargs - Is)...);
      } else {
        return attr_value{((Result (*)(expr_scope &, Args...))f.userdata)(
            scope,
            arg_wrapper<remove_cref_t<Args>>::get_arg(f.arg_settings_at(Is), scope, args + Is, nargs - Is)...)};
      }
    }
  };

  template<typename Result, typename Arg>
  struct J_TYPE_HIDDEN expr_function_wrapper<Result (*)(expr_scope &, Arg)> final {
    inline static attr_value call(const expr_fn & fn, expr_scope & scope, attr_value * args, u32_t nargs) {
      if constexpr (is_same_v<Result, attr_value>) {
        return ((Result (*)(expr_scope &, Arg))fn.userdata)(
          scope,
          arg_wrapper<remove_cref_t<Arg>>::get_arg(fn.arg_settings_at(0), scope, args, nargs));
      } else {
        return attr_value(((Result (*)(expr_scope &, Arg))fn.userdata)(
          scope,
          arg_wrapper<remove_cref_t<Arg>>::get_arg(fn.arg_settings_at(0), scope, args, nargs)));
      }
    }
  };

  template<typename Result, typename... Args>
  struct J_TYPE_HIDDEN expr_function_wrapper<Result (*)(Args...)> final {
    inline static attr_value call(const expr_fn & fn, expr_scope & scope, attr_value * args, u32_t nargs) {
      return do_call(fn, scope, args, nargs, make_index_sequence<sizeof...(Args), u32_t>());
    }

  private:
    template<u32_t... Is>
    J_A(AI,ND,NODISC,HIDDEN) static inline attr_value do_call(const expr_fn & f, expr_scope & scope, attr_value * args, u32_t nargs, integer_sequence<u32_t, Is...>) {
      if constexpr (is_same_v<Result, attr_value>) {
        return ((Result (*)(Args...))f.userdata)(
            arg_wrapper<remove_cref_t<Args>>::get_arg(f.arg_settings_at(Is), scope, args + Is, nargs - Is)...);
      } else {
        return attr_value{((Result (*)(Args...))f.userdata)(
            arg_wrapper<remove_cref_t<Args>>::get_arg(f.arg_settings_at(Is), scope, args + Is, nargs - Is)...)
        };
      }
    }
  };

  template<typename Result, typename Arg>
  struct J_TYPE_HIDDEN expr_function_wrapper<Result (*)(Arg)> final {
    J_A(AI,ND,NODISC,HIDDEN) static inline attr_value call(const expr_fn & f, expr_scope & scope, attr_value * args, u32_t nargs) {
      if constexpr (is_same_v<Result, attr_value>) {
        return ((Result (*)(Arg))f.userdata)(
            arg_wrapper<remove_cref_t<Arg>>::get_arg(f.arg_settings_at(0), scope, args, nargs));
      } else {
        return attr_value{((Result (*)(Arg))f.userdata)(
            arg_wrapper<remove_cref_t<Arg>>::get_arg(f.arg_settings_at(0), scope, args, nargs))
        };
      }
    }
  };

  template<typename FnPtr>
  struct J_TYPE_HIDDEN lambda_traits;

  template<typename Result, typename Class, typename... Args>
  struct J_TYPE_HIDDEN lambda_traits<Result (Class::*)(Args...) const> final {
    using fn_t J_NO_DEBUG_TYPE = Result (*)(Args...);
    constexpr inline static u8_t nargs_v = sizeof...(Args);
  };

  template<typename Result, typename Class, typename... Args>
  struct J_TYPE_HIDDEN lambda_traits<Result (Class::*)(Args...) const noexcept> final {
    using fn_t = J_NO_DEBUG_TYPE Result (*)(Args...);
    constexpr inline static u8_t nargs_v = sizeof...(Args);
  };

  template<typename Result, typename... Args, u32_t... Is>
  constexpr expr_fn do_make_expr_fn(integer_sequence<u32_t, Is...>, Result (*fn)(Args...), arg_init_flags overrides[sizeof...(Args)]) noexcept {
    return expr_fn{
      {arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).types...},
      {arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).flags...},
      sizeof...(Args),
      (u8_t)(0 + ... + arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).is_required()),
      &expr_function_wrapper<Result(*)(Args...)>::call,
      (void*)fn
    };
  }

  template<typename Result, typename... Args, u32_t... Is>
  constexpr expr_fn do_make_expr_fn(integer_sequence<u32_t, Is...>, Result (*fn)(expr_scope &, Args...), arg_init_flags overrides[sizeof...(Args)]) noexcept {
    return expr_fn{
      {arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).types...},
      {arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).flags...},
      sizeof...(Args),
      (u8_t)(0 + ... + arg_wrapper<remove_cref_t<Args>>::make_arg_init(overrides[Is]).is_required()),
      &expr_function_wrapper<Result(*)(expr_scope &, Args...)>::call,
      (void*)fn
    };
  }

  template<typename Result, typename... Args, typename... Overrides>
  J_A(AI,ND,NODISC) inline constexpr expr_fn make_expr_fn(Result (*fn)(Args...), const Overrides & ... ors) noexcept {
    return do_make_expr_fn(make_index_sequence<sizeof...(Args), u32_t>{}, fn, (arg_init_flags[sizeof...(Args)]){ors...});
  }

  template<typename Result, typename... Args, typename... Overrides>
  J_A(AI,ND,NODISC) inline constexpr expr_fn make_expr_fn(Result (*fn)(expr_scope &, Args...), const Overrides & ... ors) noexcept {
    return do_make_expr_fn(make_index_sequence<sizeof...(Args), u32_t>{}, fn, (arg_init_flags[sizeof...(Args)]){ors...});
  }

  template<typename Fn, typename... Overrides>
  constexpr expr_fn make_expr_fn(Fn && lambda, const Overrides & ... ors) noexcept {
    using lt = lambda_traits<decltype(&remove_cref_t<Fn>::operator())>;
    return do_make_expr_fn(make_index_sequence<lt::nargs_v, u32_t>{}, static_cast<typename lt::fn_t>(lambda), (arg_init_flags[lt::nargs_v]){ors...});
  }
}
