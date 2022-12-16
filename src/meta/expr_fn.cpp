#include "expr_fn_def.hpp"
#include "meta/term.hpp"
#include "logging/global.hpp"
#include "strings/formatting/pad.hpp"
#include "meta/attr_context.hpp"
#include "meta/cpp_codegen.hpp"
#include "strings/format.hpp"
#include "meta/expr.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::noncopyable_vector<j::meta::expr_fn>);

namespace j::meta {
  namespace s = strings;
  namespace {
    [[nodiscard]] inline bool check_arg_type(arg_settings s, const attr_value & arg) noexcept {
      const attr_type_mask arg_mask = to_attr_mask(arg.type);
      if (s.accepts_single() && (s.types & arg_mask)) {
        return true;
      }
      if (!s.accepts_array()) {
        return false;
      }

      if (arg.type == attr_tuple) {
        for (auto & v : *arg.tuple) {
          if (!(s.types & to_attr_mask(v.type))) {
            return false;
          }
        }
        return true;
      }
      return s.types & node_set_attr_mask(arg.type);
    }
  }

  J_A(ND,NODISC) trivial_array_copyable<attr_value> arg_parser<span<const attr_value>>::parse(expr_scope & s, const attr_value & v) {
    if (v.is_tuple()) {
      return *v.tuple;
    }
    J_REQUIRE(v.is_node_set(), "Expected tuple or node set.");
    trivial_array_copyable<attr_value> result{containers::uninitialized, v.node_set.size()};
    for (auto & t : v.as_node_set().iterate(s.root->mod)) {
      result.initialize_element(&t);
    }
    return result;
  }

  J_A(ND,NODISC) codegen_template arg_parser<codegen_template>::parse(expr_scope & scope, const attr_value & v) {
    switch (v.type) {
    case attr_tpl:
      return v.as_tpl();
    case attr_str:
      return codegen_template(v.string);
    case attr_int:
      return codegen_template(strings::format("{}", v.integer));
    case attr_enum:
      return codegen_template(strings::format("0x{:X}U", v.enum_val.value));
    case attr_bool:
      return codegen_template(v.integer ? "true" : "false");
    default:
      scope.throw_expr_error(s::format("Unexpected type {} for string argument.", v.type));
    }
  }

  J_A(ND,NODISC) strings::string arg_parser<strings::string>::parse(expr_scope & scope, const attr_value & v) {
    switch (v.type) {
    case attr_str:
      return v.as_str();
    case attr_int:
      return strings::format("{}", v.as_int());
    default:
      scope.throw_expr_error(s::format("Unexpected type {} for string argument.", v.type));
    }
  }

  J_A(ND,NODISC) lisp::id_name arg_parser<lisp::id_name>::parse(expr_scope & scope, const attr_value & v) {
    if (v.type != attr_type::attr_id) {
      scope.throw_expr_error(s::format("Unexpected type {} for id argument.", v.type));
    }
    return v.id.name;
  }

  J_A(ND,NODISC) lisp::id_name_ref arg_parser<lisp::id_name_ref>::parse(expr_scope & scope, const attr_value & v) {
    if (v.type != attr_type::attr_id) {
      scope.throw_expr_error(s::format("Unexpected type {} for id argument.", v.type));
    }
    return v.id.name;
  }

  [[nodiscard]] bool expr_fn::can_invoke(const attr_value * args, u32_t nargs) const {
    if (nargs < num_required_args) {
      return false;
    }

    for (u32_t i = 0U, max = j::min(num_args, nargs); i < max; ++i, ++args, --nargs) {
      const arg_settings s = arg_settings_at(i);
      if (s.is_rest()) {
        for (; nargs; ++args, --nargs) {
          if (!check_arg_type(s, *args)) {
            return false;
          }
        }
        return true;
      }
      if (!check_arg_type(s, *args)) {
        return false;
      }
    }
    return !nargs;
  }


  [[nodiscard]] const expr_fn * fn_collection::maybe_get_fn(strings::const_string_view name, const attr_value * args, u32_t nargs) {
    if (auto fns = functions.maybe_at(name)) {
      for (auto & fn : *fns) {
        if (fn.can_invoke(args, nargs)) {
          return &fn;
        }
      }
    }
    return nullptr;
  }

  fn_collection & fn_collection::add_fn(strings::const_string_view name, expr_fn value) noexcept {
    J_ASSERT(name);
    functions[name].push_back(value);
    return *this;
  }

  namespace {
    template<bool IsMax>
    attr_value minmax(span<const i64_t> vs) {
      attr_value res{};
      for (auto v : vs) {
        res = attr_value(res ? (IsMax ? j::max(v, res.integer) : j::min(v, res.integer)) : v);
      }
      return res;
    }

    i64_t bit(i64_t index) {
      return 1L << index;
    }

    bool gt(i64_t lhs, i64_t rhs) {
      return lhs > rhs;
    }

    bool gte(i64_t lhs, i64_t rhs) {
      return lhs >= rhs;
    }

    bool lt(i64_t lhs, i64_t rhs) {
      return lhs < rhs;
    }

    bool lte(i64_t lhs, i64_t rhs) {
      return lhs <= rhs;
    }

    s::string join(strings::const_string_view sep, span<const s::string> vs) {
      s::string res;
      bool is_first = true;
      for (auto & v : vs) {
        if (!is_first) {
          res += sep;
        }
        is_first = false;
        res += v;
      }
      return res;
    }

    template<bool IsAnd>
    s::string oxford(span<const s::string> vs) {
      s::string res;
      if (const u32_t num = vs.size()) {
        for (u32_t i = 0; i < num - 1U; ++i) {
          if (i) {
            res += ", ";
          }
          res += vs[i];
        }
        if (num > 1) {
          if constexpr (IsAnd) {
            res += num > 2 ? ", and " : " and ";
          } else {
            res += num > 2 ? ", or " : " or ";
          }
        }
        res += vs[num - 1];
      }
      return res;
    }

    template<bool IsHex, bool IsUnsigned>
    [[nodiscard]] s::string format_num(i64_t v) {
      const char * prefix;
      if constexpr (IsUnsigned) {
        if constexpr (IsHex) {
          prefix = "0x";
        } else {
          prefix = "0b";
        }
      } else {
        if constexpr (IsHex) {
          prefix = v < 0 ? "-0x" : "0x";
        } else {
          prefix = v < 0 ? "-0b" : "0b";
        }
        v = j::abs(v);
      }
      return s::format(IsHex ? (IsUnsigned ? "{}{:X}U" : "{}{:X}") : (IsUnsigned ? "{}{:b}U" : "{}{:b}"), prefix, v);
    }

    template<bool IsHex, bool IsUnsigned>
    [[nodiscard]] s::string format_num_width(i64_t v, i64_t width) {
      const char * prefix;
      if constexpr (IsUnsigned) {
        if constexpr (IsHex) {
          prefix = "0x";
        } else {
          prefix = "0b";
        }
      } else {
        if constexpr (IsHex) {
          prefix = v < 0 ? "-0x" : "0x";
        } else {
          prefix = v < 0 ? "-0b" : "0b";
        }
        v = j::abs(v);
      }
      s::string str = s::format(IsHex ? (IsUnsigned ? "{:X}U" : "{:X}") : (IsUnsigned ? "{:b}U" : "{:b}"), v);
      const i64_t pad = width - str.size();
      return prefix + (pad > 0 ? s::get_zeroes(pad) + str : str);
    }
  }

  [[nodiscard]] fn_collection make_root_fn_collection() noexcept {
    fn_collection result;
    result.add_fn("Assert", make_expr_fn([](const attr_value & v, s::string s) {
      J_REQUIRE(!v.value_empty(), "Assertion \"{}\" failed.", s);
      return v;
    }));

    result.add_fn("Abs", make_expr_fn(static_cast<i64_t (*)(i64_t)>(j::abs)));
    result.add_fn("Tuple", make_expr_fn([](span<attr_value> vs) noexcept { return attr_value(vs); }, rest));
    result.add_fn("Join", make_expr_fn(&join, required, lax_rest));
    result.add_fn("OxfordAnd", make_expr_fn(oxford<true>, lax_rest));
    result.add_fn("OxfordOr", make_expr_fn(oxford<false>, lax_rest));
    result.add_fn("FormatBin", make_expr_fn(&format_num<false, false>));
    result.add_fn("FormatHex", make_expr_fn(&format_num<true, false>));
    result.add_fn("FormatBin", make_expr_fn(&format_num_width<false, false>));
    result.add_fn("FormatHex", make_expr_fn(&format_num_width<true, false>));
    result.add_fn("Format", make_expr_fn([](s::const_string_view s, span<const attr_value> vs) {
      strings::formatting::format_value fmts[vs.size()];
      u32_t i = 0U;
      for (auto & v : vs) {
        switch (v.type) {
        case attr_int:
          fmts[i++] = s::format_value(v.integer);
          break;
        case attr_bool:
          fmts[i++] = s::format_value((bool)v.integer);
          break;
        case attr_enum:
          if (v.enum_val.def->is_flag_enum) {
            fmts[i++] = s::format_value(v.enum_val.value);
          } else {
            fmts[i++] = s::format_value(v.enum_val.def->get_unqualified_name(v.enum_val.value));
          }
          break;
        case attr_term:
          fmts[i++] = s::format_value(v.node->name);
          break;
        case attr_str:
          fmts[i++] = s::format_value(v.string);
          break;
        case attr_id:
          fmts[i++] = s::format_value(v.id.resolved);
          break;
        default:
          J_THROW("Unsupported attr for format.");
        }
      }
      return s::detail::format(s, i, fmts);
    }, required, rest));
    result.add_fn("Min", make_expr_fn(&minmax<false>, lax_rest));
    result.add_fn("Max", make_expr_fn(&minmax<true>, lax_rest));
    result.add_fn("Bit", make_expr_fn(&bit));
    result.add_fn("Gt", make_expr_fn(&gt));
    result.add_fn("Gte", make_expr_fn(&gte));
    result.add_fn("Lt", make_expr_fn(&lt));
    result.add_fn("Lte", make_expr_fn(&lte));

    return result;
  }
}
