#pragma once

#include "containers/vector.hpp"
#include "strings/string_map_fwd.hpp"
#include "meta/attr_value.hpp"
#include "meta/expr_fn.hpp"

J_DECLARE_EXTERN_STRING_MAP(j::meta::attr_value);

namespace j::meta::inline rules {
  struct term_expr;
}
namespace j::meta {
  struct tpl_part;
  struct expr;
  struct expr_scope_root;

  struct expr_scope_init final {
    noncopyable_vector<pair<strings::string, expr*>> vars;
    noncopyable_vector<fn*> fns;
    void set(strings::const_string_view name, expr * J_NOT_NULL e);
    void define_fn(fn * J_NOT_NULL f) noexcept;
  };

  struct expr_scope_guard;

  struct expr_scope {
    expr_scope * parent = nullptr;
    expr_scope_root * root = nullptr;
    strings::string_map<attr_value> vars;
    fn_collection fns{};
    const expr * current_expr = nullptr;
    const tpl_part * current_tpl = nullptr;
    const term_expr * current_te = nullptr;

    [[nodiscard]] const expr * get_expr() const noexcept;
    [[nodiscard]] const tpl_part * get_tpl() const noexcept;
    [[nodiscard]] const term_expr * get_te() const noexcept;

    explicit expr_scope(expr_scope_root * J_NOT_NULL root) noexcept;
    explicit expr_scope(expr_scope * J_NOT_NULL parent) noexcept;
    expr_scope(expr_scope * J_NOT_NULL parent, const expr_scope_init & init) noexcept;
    ~expr_scope();

    void clear() noexcept;

    [[nodiscard]] inline expr_scope_guard enter_expr(const expr * J_NOT_NULL e) noexcept;
    [[nodiscard]] inline expr_scope_guard enter_tpl(const tpl_part * J_NOT_NULL e) noexcept;
    [[nodiscard]] inline expr_scope_guard enter_term_expr(const term_expr * J_NOT_NULL e) noexcept;
    [[nodiscard]] strings::string format_loc() const noexcept;

    [[nodiscard]] attr_value * get_ptr(strings::const_string_view name);
    [[nodiscard]] attr_value get(strings::const_string_view name);
    [[nodiscard]] const expr_fn & get_fn(strings::const_string_view name, const attr_value * args, u32_t nargs);
    void assign(strings::const_string_view name, attr_value && value) noexcept;
    void set(strings::const_string_view name, attr_value && value) noexcept;
    [[nodiscard]] attr_value & operator[](strings::const_string_view name) noexcept;

    [[noreturn]] void throw_expr_error(strings::string && message) const;
    [[noreturn]] void throw_expr_error(const char * J_NOT_NULL message) const;
    [[noreturn]] void throw_expr_error(strings::const_string_view && message) const;
    [[noreturn]] void throw_prop_not_found_error(strings::const_string_view prop, const attr_value & v) const;
  };

  struct expr_scope_guard {
    J_BOILERPLATE(expr_scope_guard, COPY_DEL)
    expr_scope * scope = nullptr;
    const expr * e = nullptr;
    const tpl_part * tpl = nullptr;
    const term_expr * te = nullptr;

    inline expr_scope_guard(expr_scope * J_NOT_NULL scope, const expr * e, const tpl_part * tpl, const term_expr * te) noexcept
      : scope(scope),
        e(e),
        tpl(tpl),
        te(te)
    { }

    inline expr_scope_guard(expr_scope_guard && rhs) noexcept
      : scope(rhs.scope),
        e(rhs.e),
        tpl(rhs.tpl),
        te(rhs.te)
    { rhs.scope = nullptr; }

    inline ~expr_scope_guard() {
      if (scope) {
        scope->current_expr = e;
        scope->current_tpl = tpl;
        scope->current_te = te;
      }
    }
  };

  [[nodiscard]] inline expr_scope_guard expr_scope::enter_expr(const expr * J_NOT_NULL e) noexcept {
    expr_scope_guard result{this, current_expr, current_tpl, current_te};
    current_expr = e;
    return result;
  }

  [[nodiscard]] inline expr_scope_guard expr_scope::enter_tpl(const tpl_part * J_NOT_NULL tpl) noexcept {
    expr_scope_guard result{this, current_expr, current_tpl, current_te};
    current_tpl = tpl;
    return result;
  }

  [[nodiscard]] inline expr_scope_guard expr_scope::enter_term_expr(const term_expr * J_NOT_NULL te) noexcept {
    expr_scope_guard result{this, current_expr, current_tpl, current_te};
    current_te = te;
    return result;
  }

  struct expr_scope_root final : expr_scope {
    J_BOILERPLATE(expr_scope_root, COPY_DEL)
    module * mod = nullptr;

    expr_scope_root(module * J_NOT_NULL mod) noexcept;
    expr_scope_root(module * J_NOT_NULL mod, const expr_scope_init & init) noexcept;
  };
}
