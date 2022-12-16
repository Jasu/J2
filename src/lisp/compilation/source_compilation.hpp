#pragma once

#include "lisp/compilation/pkg_context.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::compilation {
  struct context;

  struct tlf final {
    enum class tlf_status : u8_t {
      none = 0U,
      pending,
      parsed,
    };

    tlf_status status = tlf_status::none;

    lisp_imm form{};

    tlf * next = nullptr;
    tlf * prev = nullptr;

    tlf * next_pending = nullptr;
    tlf ** work_list = nullptr;
  };

  struct root_source_compilation_context;

  enum class source_compiler_status : u8_t {
    ready,
    pending,
  };

  struct source_compilation_context final {
    source_compilation_context(root_source_compilation_context & root,
                               pkg_context & package_context, source_compilation_context * next,
                               sources::source & src,
                               symbols::symbol & top_level_expr_symbol);


    /// Get the name of a symbol.
    J_INLINE_GETTER strings::const_string_view name(id id) const noexcept {
      return package_context.name(id);
    }

    /// Get the name of a symbol.
    J_INLINE_GETTER strings::const_string_view name(symbols::symbol * J_NOT_NULL sym) const noexcept {
      return package_context.name(sym);
    }

    /// The package in which source is being compiled in.
    packages::pkg & pkg;

    /// The source being compiled.
    sources::source & src;

    /// Env compiler.
    env::env_compiler & env_compiler;

    /// Pool for allocating temporary data.
    j::mem::bump_pool & temp_pool;

    /// Get the pool for allocating value and function metadata.
    J_INLINE_GETTER j::mem::bump_pool & metadata_pool() const noexcept {
      return package_context.metadata_pool();
    }

    /// Current source location.
    J_INLINE_GETTER sources::source_location & source_location() const noexcept {
      return package_context.source_location;
    }

    /// Gets the symbol referenced by `id`. `id` need not be resolved.
    [[nodiscard]] J_INLINE_GETTER symbols::symbol * get_symbol(id id, symbol_status status) {
      return package_context.get_symbol(id, status);
    }

    /// Resolves a symbol id.
    J_INLINE_GETTER id resolve(id symbol, symbol_status status) {
      return package_context.resolve(symbol, status);
    }

    void compile_tlf(lisp_imm imm);

    J_RETURNS_NONNULL tlf * push_tlf(lisp_imm imm, tlf::tlf_status status);

    u32_t num_expr_tlfs = 0U;
    u32_t num_pending_tlfs = 0U;

    tlf * first_tlf = nullptr;
    tlf * last_tlf = nullptr;

    tlf * work_list = nullptr;

    J_RETURNS_NONNULL symbols::symbol * to_final_ast();

    J_RETURNS_NONNULL symbols::symbol * to_defun();

    source_compilation_context *next = nullptr, *prev = nullptr;

    symbols::symbol & top_level_expr_symbol;

    root_source_compilation_context & root;

    pkg_context & package_context;

    source_compiler_status compile(sources::source & src);

    void compile_work_list();

  private:
    void pend_tlf(id id, tlf * J_NOT_NULL tlf);
  };

  struct root_source_compilation_context final {
    explicit root_source_compilation_context(env::env_compiler & env_compiler, j::mem::bump_pool_checkpoint cp) noexcept;

    void compile_pending();

    env::env_compiler & env_compiler;

    source_compilation_context & add_source_compiler(packages::pkg & pkg, sources::source & src, symbols::symbol * top_level_expr_symbol = nullptr);

    /// Pool for allocating temporary data.
    j::mem::bump_pool & temp_pool;

    source_compilation_context * first_context = nullptr;
    /// Used to rewind the temp pool when done.
    ///
    /// Note that compiling single symbols also create checkpoints, i.e. temp data that
    /// should persist between calls to `do_compile` must be allocated before entering
    /// symbol compilation.
    j::mem::bump_pool_checkpoint temp_checkpoint;
  };

  source_compilation_context & get_source_compiler(env::env_compiler & env_compiler,
                                                   packages::pkg & pkg,
                                                   sources::source & src,
                                                   symbols::symbol * top_level_expr_symbol = nullptr);

  J_RETURNS_NONNULL symbols::symbol * compile_source(env::env_compiler & env_compiler,
                                                     packages::pkg & pkg, sources::source & src,
                                                     symbols::symbol * top_level_expr_symbol = nullptr);
}
