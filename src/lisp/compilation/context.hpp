#pragma once

#include "lisp/common/id.hpp"
#include "lisp/sources/source_location.hpp"
#include "mem/bump_pool_fwd.hpp"
#include "strings/string_view.hpp"
#include "lisp/compilation/pkg_context.hpp"
#include "lisp/compilation/const_table.hpp"


namespace j::lisp::env {
  struct env_compiler;
}
namespace j::lisp::functions {
  struct func_info;
}
namespace j::lisp::packages {
  struct pkg;
}
namespace j::lisp::symbols {
  struct symbol;
}
namespace j::lisp::inline values {
  struct lisp_vec_ref;
}

namespace j::lisp::compilation {
  /// Context when compiling a symbol.
  ///
  /// The AST has been read at this point, but not macroexpanded.
  struct context final {
    /// Start compiling `sym` in `pkg`.
    context(pkg_context & ctx, symbols::symbol & sym) noexcept;

    /// Clean up afetr compiling the symbol.
    ~context();

    /// Get the pool for allocating value and function metadata.
    J_INLINE_GETTER j::mem::bump_pool & metadata_pool() const noexcept {
      return package_context.metadata_pool();
    }

    /// Get the name of the current symbol.
    strings::const_string_view name() const noexcept;

    /// Get the name of a symbol.
    J_INLINE_GETTER strings::const_string_view name(id id) const noexcept {
      return package_context.name(id);
    }

    /// Get the name of a symbol.
    J_INLINE_GETTER strings::const_string_view name(symbols::symbol * J_NOT_NULL sym) const noexcept {
      return package_context.name(sym);
    }

    /// The package in which the symbol is being compiled in.
    packages::pkg & pkg;
    /// Env compiler, needed for accessing symbols from other packages.
    env::env_compiler & env_compiler;
    /// Current source location.
    sources::source_location & source_location() const noexcept {
      return package_context.source_location;
    }
    /// Pool for allocating temporary data.
    j::mem::bump_pool & temp_pool;
    /// The symbol being compiled.
    symbols::symbol & symbol;

    /// Resolves a symbol id.
    J_INLINE_GETTER id resolve(id symbol, symbol_status status) {
      return package_context.resolve(symbol, status);
    }

    /// Gets the symbol referenced by `id`. `id` need not be resolved.
    [[nodiscard]] J_INLINE_GETTER symbols::symbol * get_symbol(id id, symbol_status status) {
      return package_context.get_symbol(id, status);
    }

    /// Gets the symbol referenced by `id`. `id` need not be resolved.
    [[nodiscard]] J_RETURNS_NONNULL symbols::symbol * alloc_closure_symbol(u32_t index, functions::func_info * info);

    pkg_context & package_context;

    // Data applicable per symbol type.
    union {
      // Data specific for function compilation.
      struct {
        /// Function info of the current function or closure, allocated from the metadata pool.
        functions::func_info * func_info;
        /// `func_info` of the current closure's parent, or null if top level.
        functions::func_info * parent_func_info;
      };
    };


    lisp_vec_ref allocate_vec(u32_t sz);

    lisp_vec_ref allocate_vec(u32_t sz, const sources::source_location & loc);

    const_table constants;
  private:
    /// Used to rewind the temp pool when done.
    j::mem::bump_pool_checkpoint temp_checkpoint;
  };
}
