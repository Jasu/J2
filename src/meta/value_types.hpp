#pragma once

#include "strings/string_map_fwd.hpp"
#include "meta/attr_context.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/node.hpp"
#include "containers/vector.hpp"

namespace j::meta::inline rules {
  struct te_fn_set;
  struct te_function;
  struct te_function_arg;
  struct te_type;
  struct te_value_source;
  struct term_expr;
}
namespace j::meta {
  struct val_type;

  enum class operator_type : u8_t {
    none,
    assign,
    assign_add,
    assign_sub,
    assign_mul,
    assign_div,
    /// `+=$` - Add terms including value
    add_value,
    /// `+=!` - Add terms for FX only
    add_fx,
    op_assign_max = add_fx,

    /// `!+$` - Side-FX + value
    prepend_fx,
    /// `$+!` - Value + Side-FX
    append_fx,

    add,
    sub,
    mul,
    div,

    eq,
    neq,

    lt,
    le,
    gt,
    ge,

    lor,
    land,
    lxor,

    fn_call,
    prop_get,

    index,
  };

  J_A(ND) constexpr inline i32_t num_operator_types_v = (i32_t)operator_type::index + 1;

  J_A(AI,NODISC) inline bool is_assign_operator(operator_type op) noexcept {
    return op != operator_type::none && op <= operator_type::op_assign_max;
  }

  J_A(AI,NODISC) inline bool is_non_assign_operator(operator_type op) noexcept {
    return op > operator_type::op_assign_max;
  }

  struct val_type_operators final {
    te_fn_set * operators[num_operator_types_v] = {nullptr};
    J_A(AI,NODISC) inline bool has_operator(operator_type type) const noexcept {
      return operators[(u8_t)type];
    }

    [[nodiscard]] const te_function & get_operator(operator_type type, span<te_type> arg_types) const;
    [[nodiscard]] const te_function & get_operator(operator_type type, span<te_value_source> args) const;
    [[nodiscard]] const te_function * maybe_get_operator(operator_type type, span<te_type> arg_types) const;
    [[nodiscard]] const te_function * maybe_get_operator(operator_type type, span<te_value_source> args) const;

    te_function & add(operator_type op, te_function && fn) noexcept;
  };

  struct val_type_property final {
    J_A(AI) inline val_type_property() noexcept = default;
    J_A(AI) inline val_type_property(codegen_template && tpl, val_type * J_NOT_NULL result, i32_t precedence) noexcept
      : tpl(static_cast<codegen_template &&>(tpl)),
        result(result),
        precedence(precedence)
    { }
codegen_template tpl{};
    val_type * result = nullptr;
    i32_t precedence = 100;
  };

  struct term_stack_info final {
    strings::string name;
    strings::string debug_name;
    term_expr * extra_arg = nullptr;
    val_type * allocation_type;
    i32_t tracking_index;
  };

  struct val_type final : node {
    J_BOILERPLATE(val_type, CTOR_NE_ND)

    explicit val_type(strings::const_string_view name) noexcept;

    val_type(val_region allowed_regions, strings::const_string_view name,
             doc_comment && comment) noexcept;

    J_A(AI,NODISC,ND) inline bool has_dynamic_size() const noexcept {
      return !size;
    }

    void initialize();

    val_region allowed_regions = region_none;
    u8_t align = 0U;
    u8_t size = 0U;
    bool is_scalar = false;

    strings::string type_name;
    strings::string ctor_type_name;
    strings::string getter_type_name;
    strings::string const_getter_type_name;
    attr_value ctor_stmt;
    attr_value size_expr;
    attr_value ctor_size_expr;
    attr_value getter_expr;
    attr_value const_getter_expr;

    attr_value format_expr;
    strings::string default_value_expr;
    strings::string default_name_expr;
    noncopyable_vector<pair<val_type*, codegen_template>> conversions;
    noncopyable_vector<pair<attr_type_def, codegen_template>> attr_conversions;

    const codegen_template * maybe_get_conversion(const val_type * J_NOT_NULL t) const noexcept;
    const codegen_template * maybe_get_attr_conversion(const attr_type_def & t) const noexcept;

    strings::string_map<te_fn_set> fns;
    strings::string_map<val_type_property> props;

    noncopyable_vector<term_stack_info*> term_stacks;

    attr_type_def parse_as;

    val_type_operators operators{};

    strings::string release;
    strings::string allocate;

    bool is_trs_term_type = false;
    bool is_trs_terms_type = false;

    J_A(AI) inline bool has_lifetime_tracking() const noexcept {
      return (bool)release;
    }

    void define_fn(strings::const_string_view name, te_function && fn);
    void define_prop(strings::const_string_view name, codegen_template && tpl, val_type * J_NOT_NULL result, i32_t precedence);

    J_A(AI,ND,HIDDEN) inline te_function & define_operator(operator_type op, te_function && fn) noexcept {
      return operators.add(op, static_cast<te_function &&>(fn));
    }
  };
}

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::pair<j::meta::val_type*, j::meta::codegen_template>);
J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(j::pair<j::meta::attr_type_def, j::meta::codegen_template>);
J_DECLARE_EXTERN_STRING_MAP(j::meta::te_fn_set);
J_DECLARE_EXTERN_STRING_MAP(j::meta::val_type_property);
