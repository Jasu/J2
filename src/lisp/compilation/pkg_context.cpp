#include "lisp/compilation/pkg_context.hpp"
#include "lisp/compilation/compilation_errors.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/env/env_compiler.hpp"
#include "lisp/symbols/symbol.hpp"

namespace j::lisp::compilation {
  pkg_context::pkg_context(env::env_compiler & env_compiler, packages::pkg & pkg) noexcept
    : pkg(pkg),
      env_compiler(env_compiler)
  {
  }

  context pkg_context::enter_context(symbols::symbol & sym) {
    return context(*this, sym);
  }

  pkg_context::~pkg_context() {
  }

  j::mem::bump_pool & pkg_context::metadata_pool() const noexcept {
    return pkg.metadata_pool;
  }

  strings::const_string_view pkg_context::name(id id) const noexcept {
    return env_compiler.symbol_name(id);
  }

  strings::const_string_view pkg_context::name(symbols::symbol * J_NOT_NULL sym) const noexcept {
    return env_compiler.symbol_name(sym->name);
  }

  [[nodiscard]] symbols::symbol * pkg_context::get_symbol(id id, symbol_status status) {
    id = resolve(id, status);
    const bool is_optional = status >= symbol_status::optional_func;
    if (is_optional && !id) {
      return nullptr;
    }
    const bool allow_initializing = status >= symbol_status::initializing_func;

    const bool is_function = status == symbol_status::initialized_func
      || status == symbol_status::initializing_func
      || status == symbol_status::optional_func;

    symbols::symbol * const sym = env_compiler.try_get_symbol(id, allow_initializing);
    if (!sym) {
      if (is_optional) {
        return nullptr;
      }
      if (is_function) {
        throw_compilation_error(source_location, id, function_recursive, id);
      } else {
        throw_compilation_error(source_location, id, variable_recursive, id);
      }
    }

    if (is_function) {
      if (!sym->value_info.is_function() && !sym->value_info.is_macro()) {
        throw_compilation_error(source_location, id, not_function, id);
      }
    } else {
      if (status == symbol_status::writable_var && sym->is_constant()) {
        throw_compilation_error(source_location, id, not_writable, id);
      }
    }

    return sym;
  }

  [[nodiscard]] id pkg_context::do_resolve(struct id id, symbol_status status) {
    J_ASSERT(id.is_unresolved());
    if (struct id result = pkg.symbol_table.do_try_resolve(id)) {
      return result;
    }

    const bool is_optional = status >= symbol_status::optional_func;
    if (is_optional) {
      return id::none;
    }

    const bool is_function = status == symbol_status::initialized_func
      || status == symbol_status::initializing_func
      || status == symbol_status::optional_func;
    if (is_function) {
      throw_compilation_error(source_location, id, function_not_found, id);
    } else {
      throw_compilation_error(source_location, id, variable_not_found, id);
    }
  }
}
