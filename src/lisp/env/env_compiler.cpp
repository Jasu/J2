#include "lisp/env/env_compiler.hpp"
#include "lisp/env/environment.hpp"
#include "lisp/packages/pkg.hpp"
#include "lisp/compilation/compile_defun.hpp"
#include "lisp/compilation/compile_symbol.hpp"
#include "lisp/compilation/context.hpp"
#include "lisp/compilation/source_compilation.hpp"

namespace j::lisp::env {
  env_compiler::env_compiler(const compilation_options * J_NOT_NULL opts, environment * J_NOT_NULL env) noexcept
    : env(env),
      opts(opts),
      source_manager(env, opts)
  { }

  J_RETURNS_NONNULL packages::pkg * env_compiler::create_pkg(strings::const_string_view name) {
    J_ASSERT(name);
    J_ASSUME_NOT_NULL(env->packages[package_id_packages]);
    packages::pkg * pkg = env->new_package(name);
    pkg->status = packages::pkg_status::compiled;
    return pkg;
  }

  J_RETURNS_NONNULL packages::pkg * env_compiler::compile_pkg(strings::const_string_view name, sources::source * J_NOT_NULL source) {
    J_ASSERT(name);
    J_ASSUME_NOT_NULL(env->packages[package_id_packages]);
    J_ASSERT(!env->packages[package_id_packages]->symbol_table.names.try_get_id_of(name));
    packages::pkg * pkg = env->new_package(name);
    compilation::compile_source(*this, *pkg, *source);
    {
      auto & pkgc = pkg->get_pkg_context();
      compilation::context ctx(pkgc, *pkg->load_symbol);
      if (pkg->load_symbol && pkg->load_symbol->value_info.is_function() && pkg->load_symbol->value.is_vec_ref()) {
        compilation::compile_defun(ctx);
      } else {
        pkg->load_symbol = nullptr;
      }
      pkg->status = packages::pkg_status::compiled;
    }
    return pkg;
  }

  J_RETURNS_NONNULL packages::pkg * env_compiler::compile_pkg(const lisp_str * J_NOT_NULL name, sources::source * J_NOT_NULL source) {
    return compile_pkg(name->value(), source);
  }

  J_RETURNS_NONNULL packages::pkg * env_compiler::compile_pkg(strings::const_string_view name) {
    J_ASSERT(name);
    J_ASSUME_NOT_NULL(env->packages[package_id_packages]);

    if (auto id = env->packages[package_id_packages]->symbol_table.names.try_get_id_of(name)) {
      auto index = id.index();
      J_ASSUME(index < 256);
      packages::pkg * pkg = env->packages[index];
      J_ASSUME_NOT_NULL(pkg);
      J_ASSERT(pkg->status != packages::pkg_status::compiling);
      return pkg;
    }

    auto src = source_manager.add_package_source(name);
    J_REQUIRE(src, "Package not found.");
    return compile_pkg(name, src);
  }

  J_RETURNS_NONNULL packages::pkg * env_compiler::compile_pkg(const lisp_str * J_NOT_NULL name) {
    return compile_pkg(name->value());
  }

  symbols::symbol * env_compiler::try_get_symbol(id name, bool allow_initializing) {
    J_ASSERT(name && !name.is_unresolved() && !name.is_gensym());
    packages::pkg * pkg = env->packages[name.package_id()];
    J_ASSUME_NOT_NULL(pkg);
    if (symbols::symbol * sym = pkg->symbol_table.maybe_at(name)) {
      if (sym->is_pending()) {
        return nullptr;
      }
      if (sym->is_initializing()) {
        return allow_initializing ? sym : nullptr;
      } else if (sym->is_uninitialized()) {
        compilation::compile_symbol(*pkg, *sym);
      }
      return sym;
    }
    return nullptr;
  }

  packages::pkg * env_compiler::load_pkg(strings::const_string_view name) {
    packages::pkg * pkg = compile_pkg(name);
    if (pkg->status != packages::pkg_status::loaded) {
      pkg->load();
    }
    return pkg;
  }

  strings::const_string_view env_compiler::symbol_name(id symbol) noexcept {
    return env->symbol_name(symbol)->value();
  }

  compilation::root_source_compilation_context & env_compiler::get_root_compilation_context() {
    if (!root_context) {
      auto checkpoint = temp_pool.checkpoint();
      root_context = &temp_pool.emplace<compilation::root_source_compilation_context>(*this, checkpoint);
    }
    return *root_context;
  }

  J_RETURNS_NONNULL sources::string_source * env_compiler::get_repl_source() {
    if (!repl_source) {
      repl_source = source_manager.add_dynamic_source(":repl");
    }
    return repl_source;
  }
}
