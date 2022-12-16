#include "expr_scope.hpp"
#include "meta/rules/term_expr.hpp"
#include "meta/attr_context.hpp"
#include "meta/dump.hpp"
#include "meta/fn.hpp"
#include "strings/format.hpp"
#include "strings/string_map.hpp"
#include "meta/term.hpp"
#include "meta/errors.hpp"
#include "meta/module.hpp"
#include "meta/basic_node_set.hpp"
#include "meta/expr.hpp"

namespace j::meta {
  namespace s = strings;

  namespace {
    void init_expr_scope(expr_scope * J_NOT_NULL scope, const expr_scope_init & init) {
      for (auto & p : init.vars) {
        scope->set(p.first, eval_expr(*scope, p.second));
      }
      for (auto & f : init.fns) {
        scope->root->fns.add_fn(f->name, f->to_expr_fn());
      }
    }
  }

  expr_scope::expr_scope(expr_scope_root * J_NOT_NULL root) noexcept
    : root(root)
  { }


  expr_scope::expr_scope(expr_scope * J_NOT_NULL parent) noexcept
    : parent(parent),
      root(parent->root)
  { }

  expr_scope::expr_scope(expr_scope * J_NOT_NULL parent, const expr_scope_init & init) noexcept
    : parent(parent),
      root(parent->root)
  {
    init_expr_scope(this, init);
  }

  [[nodiscard]] const expr * expr_scope::get_expr() const noexcept {
    if (current_expr) {
      return current_expr;
    }
    return parent ? parent->get_expr() : nullptr;
  }

  [[nodiscard]] const tpl_part * expr_scope::get_tpl() const noexcept {
    if (current_tpl) {
      return current_tpl;
    }
    return parent ? parent->get_tpl() : nullptr;
  }

  [[nodiscard]] const term_expr * expr_scope::get_te() const noexcept {
    if (current_te) {
      return current_te;
    }
    return parent ? parent->get_te() : nullptr;
  }

  [[nodiscard]] s::string expr_scope::format_loc() const noexcept {
    s::string result;
    if (const expr * const e = get_expr()) {
      result = s::format("expression {}{}", dump_str(root->mod, *e), e->loc);
    }
    if (const tpl_part * const tpl = get_tpl()) {
      result = s::format(result ? "{} / template {}" : "{}template {}", static_cast<s::string &&>(result), tpl->loc);
    }
    if (const term_expr * const te = get_te()) {
      result = s::format(result ? "{} / term expr {}" : "{}term expr {}", static_cast<s::string &&>(result), te->loc);
    }
    return static_cast<s::string &&>(result);
  }

  [[noreturn]] void expr_scope::throw_expr_error(strings::string && message) const {
    if (s::string loc = format_loc()) {
      J_THROW("{} in {}", message, loc);
    } else {
      J_THROW("{}", message);
    }
  }
  [[noreturn]] void expr_scope::throw_expr_error(const char * J_NOT_NULL message) const {
    throw_expr_error(s::string(message));
  }

  [[noreturn]] void expr_scope::throw_expr_error(strings::const_string_view && message) const {
    throw_expr_error(s::string(message));
  }

  [[noreturn]] void expr_scope::throw_prop_not_found_error(strings::const_string_view prop, const attr_value & v) const {
    s::string name;
    if (v.type == attr_struct) {
      name = " " + v.struct_val.def->name;
    }

    if (s::string loc = format_loc()) {
      J_THROW("Property {} not found in {}{#bright_cyan,bold}{}{/} in {}.", prop, v.type, name, loc);
    } else {
      J_THROW("Property {} not found in {}{#bright_cyan,bold}{}{/}.", prop, v.type, name);
    }
  }

  expr_scope::~expr_scope() { }

  void expr_scope_init::set(s::const_string_view name, expr * J_NOT_NULL e) {
    for (auto & p : vars) {
      J_REQUIRE(p.first != name, "Duplicate variable {}", name);
    }
    vars.emplace_back(pair<s::string, expr*>{name, e});
  }

  void expr_scope_init::define_fn(fn * J_NOT_NULL f) noexcept {
    fns.emplace_back(f);
  }


  void expr_scope::clear() noexcept {
    vars.clear();
  }

  [[nodiscard]] attr_value expr_scope::get(s::const_string_view name) {
    for (expr_scope * cur = this; cur; cur = cur->parent) {
      if (const attr_value * val = cur->vars.maybe_at(name)) {
        return *val;
      }
    }
    node ** n = root->mod->nodes.maybe_at(name);
    if (!n) {
      throw_expr_error(s::format("Variable '{}' not found.", name));
    }
    return attr_value(*n);
  }

  [[nodiscard]] attr_value * expr_scope::get_ptr(s::const_string_view name) {
    for (expr_scope * cur = this; cur; cur = cur->parent) {
      if (attr_value * val = cur->vars.maybe_at(name)) {
        return val;
      }
    }
    return nullptr;
  }

  [[nodiscard]] const expr_fn & expr_scope::get_fn(strings::const_string_view name, const attr_value * args, u32_t nargs) {
    if (const expr_fn * fn = fns.maybe_get_fn(name, args, nargs)) {
      return *fn;
    }

    if (parent) {
      return parent->get_fn(name, args, nargs);
    }
    s::string types;
    for (u32_t i = 0U; i < nargs; ++i) {
      if (i) {
        types += ", ";
      }
      types += s::format("{}", args[i].type);
    }
    J_FAIL("No matching function found for {}({}).", name, types);
  }

  void expr_scope::set(s::const_string_view name, attr_value && value) noexcept {
    vars[name] = static_cast<attr_value &&>(value);
  }

  void expr_scope::assign(s::const_string_view name, attr_value && value) noexcept {
    if (attr_value * ptr = get_ptr(name)) {
      *ptr = static_cast<attr_value &&>(value);
    } else {
      vars[name] = static_cast<attr_value &&>(value);
    }
  }

  [[nodiscard]] attr_value & expr_scope::operator[](s::const_string_view name) noexcept {
    return vars[name];
  }

  expr_scope_root::expr_scope_root(module * J_NOT_NULL mod) noexcept
    : expr_scope{this},
      mod(mod)
  {
    fns = make_root_fn_collection();
    vars.emplace("Terms", attr_term_set, basic_node_set::all(mod, node_term));
    vars.emplace("Types", attr_val_type_set, basic_node_set::all(mod, node_term));
  }

  expr_scope_root::expr_scope_root(module * J_NOT_NULL mod, const expr_scope_init & init) noexcept
    : expr_scope{this},
      mod(mod)
  {
    fns = make_root_fn_collection();
    vars.emplace("Terms", attr_term_set, basic_node_set::all(mod, node_term));
    vars.emplace("Types", attr_val_type_set, basic_node_set::all(mod, node_term));
    init_expr_scope(this, init);
  }
}
