#pragma once

#include "lisp/common/id.hpp"
#include "lisp/lisp_fwd.hpp"
#include "lisp/sources/source_manager.hpp"
#include "mem/bump_pool.hpp"

namespace j::lisp::env {
  /// Manages compilation for the whole environment.
  struct env_compiler final {
    environment * env = nullptr;
    const compilation_options * opts = nullptr;

    env_compiler(const compilation_options * J_NOT_NULL opts, environment * J_NOT_NULL env) noexcept;

    J_RETURNS_NONNULL packages::pkg * create_pkg(strings::const_string_view name);

    J_RETURNS_NONNULL packages::pkg * compile_pkg(strings::const_string_view name, sources::source * J_NOT_NULL source);

    J_RETURNS_NONNULL packages::pkg * compile_pkg(const lisp_str * J_NOT_NULL name, sources::source * J_NOT_NULL source);

    J_RETURNS_NONNULL packages::pkg * compile_pkg(strings::const_string_view name);

    J_RETURNS_NONNULL packages::pkg * compile_pkg(const lisp_str * J_NOT_NULL name);

    J_RETURNS_NONNULL packages::pkg * load_pkg(strings::const_string_view package_name);

    J_RETURNS_NONNULL sources::string_source * get_repl_source();

    strings::const_string_view symbol_name(id symbol) noexcept;

    symbols::symbol * try_get_symbol(id name, bool allow_initializing = false);

    sources::source_manager source_manager;

    j::mem::bump_pool temp_pool;

    compilation::root_source_compilation_context & get_root_compilation_context();

    compilation::root_source_compilation_context * root_context = nullptr;
    sources::string_source * repl_source = nullptr;
  };
}
