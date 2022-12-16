#pragma once

#include "lisp/lisp_fwd.hpp"
#include "lisp/common/id.hpp"
#include "lisp/sources/source_location.hpp"
#include "mem/bump_pool_fwd.hpp"

namespace j::lisp::compilation {
  struct context;

  enum class symbol_status : u8_t {
    initialized_func,
    initialized_var,
    initializing_func,
    initializing_var,
    writable_var,
    optional_func,
    optional_var,
  };

  /// Require that the symbol is initialized.
  constexpr inline auto initialized_var = symbol_status::initialized_var;
  /// Require that the symbol is an initialized function.
  constexpr inline auto initialized_func = symbol_status::initialized_func;
  /// Require that the symbol is initialized or initializing.
  constexpr inline auto initializing_var = symbol_status::initializing_var;
  /// Require that the symbol is an initialized or initializing function.
  constexpr inline auto initializing_func = symbol_status::initializing_func;
  /// Require that the symbol is writable.
  constexpr inline auto writable_var = symbol_status::writable_var;
  /// Require that the symbol is function, or it does not exist.
  constexpr inline auto optional_func = symbol_status::optional_func;
  /// Require nothing.
  constexpr inline auto optional_var = symbol_status::optional_var;

  /// Context for parsing or compiling code within a package.
  struct pkg_context final {
    J_BOILERPLATE(pkg_context, CTOR_DEL, COPY_DEL, MOVE_DEL)

    /// Start `pkg`.
    pkg_context(env::env_compiler & env_compiler, packages::pkg & pkg) noexcept;

    /// Clean up afetr compiling the symbol.
    ~pkg_context();

    /// Get the pool for allocating value and function metadata.
    j::mem::bump_pool & metadata_pool() const noexcept;

    /// Get the name of a symbol.
    strings::const_string_view name(id id) const noexcept;

    /// Get the name of a symbol.
    strings::const_string_view name(symbols::symbol * J_NOT_NULL) const noexcept;

    /// The package in which the symbol is being compiled in.
    packages::pkg & pkg;

    /// Env compiler, needed for accessing symbols from other packages.
    env::env_compiler & env_compiler;

    /// Current source location.
    sources::source_location source_location;

    /// Resolves a symbol id.
    J_INLINE_GETTER id resolve(id symbol, symbol_status status) {
      return symbol.is_unresolved() ? do_resolve(symbol, status) : symbol;
    }

    /// Gets the symbol referenced by `id`. `id` need not be resolved.
    [[nodiscard]] symbols::symbol * get_symbol(id id, symbol_status status);

    context enter_context(symbols::symbol & sym);
  private:
    /// Resolves a definitely unresolved symbol.
    [[nodiscard]] id do_resolve(id symbol, symbol_status status);
  };
}
