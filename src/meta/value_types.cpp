#include "value_types.hpp"
#include "meta/rules/term_expr.hpp"
#include "exceptions/assert.hpp"
#include "logging/global.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::pair<j::meta::attr_type_def, j::meta::codegen_template>);
J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(j::pair<j::meta::val_type*, j::meta::codegen_template>);
J_DEFINE_EXTERN_STRING_MAP(j::meta::te_fn_set);
J_DEFINE_EXTERN_STRING_MAP(j::meta::val_type_property);

namespace j::meta {
  namespace s = strings;
  val_type::val_type(strings::const_string_view name) noexcept
    : node(node_value_type, name)
  {
    J_ASSERT(name);
  }

  val_type::val_type(val_region allowed_regions, strings::const_string_view name,
                     doc_comment && comment) noexcept
    : node(node_value_type, name, static_cast<doc_comment &&>(comment)),
      allowed_regions(allowed_regions)
  {
    J_ASSERT(name);
  }

  const codegen_template * val_type::maybe_get_conversion(const val_type * J_NOT_NULL t) const noexcept {
    for (auto & conv : conversions) {
      if (conv.first == t) {
        return &conv.second;
      }
    }
    return nullptr;
  }

  const codegen_template * val_type::maybe_get_attr_conversion(const attr_type_def & t) const noexcept {
    for (auto & conv : attr_conversions) {
      if (conv.first == t) {
        return &conv.second;
      }
    }
    return nullptr;
  }

  void val_type::initialize() {
    J_ASSERT(name);
    J_REQUIRE(type_name, "Value type {} has no Type.", name);
    if (!const_getter_type_name) {
      const_getter_type_name = getter_type_name
        ? static_cast<s::string &&>(getter_type_name)
        : type_name;
      const_getter_expr = static_cast<attr_value &&>(getter_expr);
    }
    if (!ctor_type_name) {
      ctor_type_name = type_name;
    }
    if (!size) {
      J_REQUIRE(align, "Type {} has dynamic size but no align.", name);
      J_REQUIRE(size_expr, "Type has no size expression.", name);
      J_REQUIRE(ctor_size_expr, "CtorType has no size expression.", name);
    } else {
      align = align ? align : size;
      size_expr = attr_value((i64_t)size);
      ctor_size_expr = attr_value((i64_t)size);
    }
  }

  void val_type::define_fn(strings::const_string_view name, te_function && fn) {
    J_ASSERT(name);
    fns[name].add(static_cast<te_function &&>(fn));
  }

  void val_type::define_prop(strings::const_string_view name, codegen_template && tpl, val_type * J_NOT_NULL result, i32_t precedence) {
    auto p = props.emplace(name, static_cast<codegen_template &&>(tpl), result, precedence);
    J_REQUIRE(p.second, "Duplicate function {} in {}.", p.first->first, this->name);
  }

  [[nodiscard]] const te_function & val_type_operators::get_operator(operator_type op, span<te_type> arg_types) const {
    J_ASSERT(operators[(i32_t)op], "Operator \"{}\" not found.", operator_info[(i32_t)op].name);
    return operators[(i32_t)op]->find_overload(arg_types);
  }

  [[nodiscard]] const te_function & val_type_operators::get_operator(operator_type op, span<te_value_source> args) const {
    J_ASSERT(operators[(i32_t)op], "Operator \"{}\" not found.", operator_info[(i32_t)op].name);
    return operators[(i32_t)op]->find_overload(args);
  }

  [[nodiscard]] const te_function * val_type_operators::maybe_get_operator(operator_type op, span<te_type> arg_types) const {
    return operators[(i32_t)op] ? operators[(i32_t)op]->maybe_find_overload(arg_types) : nullptr;
  }

  [[nodiscard]] const te_function * val_type_operators::maybe_get_operator(operator_type op, span<te_value_source> args) const {
    return operators[(i32_t)op] ? operators[(i32_t)op]->maybe_find_overload(args) : nullptr;
  }

  te_function & val_type_operators::add(operator_type op, te_function && fn) noexcept {
    if (!operators[(i32_t)op]) {
      operators[(i32_t)op] = ::new te_fn_set{};
    }
    return operators[(i32_t)op]->add(static_cast<te_function &&>(fn));
  }
}
