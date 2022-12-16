#pragma once

#include "lisp/env/compilation_passes.hpp"
#include "lisp/functions/calling_convention.hpp"

namespace j::lisp::env {
  struct env_compiler;
}
namespace j::lisp::cir::inline ssa {
  struct cir_ssa;
  struct ssa_builder;
}
namespace j::lisp::assembly {
  class target;
}
namespace j::lisp::compilation {
  struct context;
}

namespace j::lisp::functions {
  struct func_info;
}
namespace j::lisp::cir {
  struct cir_options;
}
namespace j::lisp::env {
  struct global_pass_options;
}
namespace j::lisp::cir::inline passes {
  struct cir_pass_context final : env::compilation_pass_context_base {
    cir_pass_context(compilation::context & ctx,
                     strings::const_string_view dump_path,
                     j::mem::bump_pool * J_NOT_NULL pool,
                     functions::func_info * func_info,
                     struct ssa_builder * J_NOT_NULL builder,
                     const env::global_pass_options * J_NOT_NULL opts,
                     functions::calling_convention cc
                     ) noexcept;
    compilation::context & compilation_context;
    functions::func_info * func_info;
    /// The architecture being targeted.
    const assembly::target * target = nullptr;
    /// The SSA being compiled.
    cir_ssa * ssa = nullptr;
    /// The builder for the SSA CFG of the current function.
    ///
    /// Builders for manipulating individual basic blocks in the SSA are created
    /// through the builder.
    ssa_builder * ssa_builder = nullptr;
    /// The calling convention being compiled for.
    ///
    /// As some passes target specific calling conventions, their necessity
    /// can be controlled per calling convention. E.g. the full call convention
    /// is transformed from ABI call with a pass.
    functions::calling_convention calling_convention = lisp::functions::calling_convention::none;
  };

  /// Runs all CIR passes that are registered and enabled.
  ///
  /// The actual configuration is read from the `env::env_compiler`.
  void run_cir_passes(compilation::context & ctx,
                      j::mem::bump_pool * J_NOT_NULL pool,
                      functions::func_info * J_NOT_NULL func_info,
                      functions::calling_convention cc,
                      cir::ssa_builder * J_NOT_NULL ssa_builder);

  struct cir_pass final : env::compilation_pass_base {
    void (*execute)(const cir_pass_context &);
    /// For which calling conventions this pass can be run.
    ///
    /// E.g. the full call transformation pass is mandatory for the full call convention, but
    /// disabled fo all others.
    functions::calling_conventions_t enabled_calling_conventions = functions::all_calling_conventions;

    consteval cir_pass(void (*J_NOT_NULL execute)(const cir_pass_context &), const char * J_NOT_NULL name, const char * J_NOT_NULL title, bool modifies_graph = false)
      : compilation_pass_base(name, title, modifies_graph),
        execute(execute)
    { }

    consteval cir_pass(void (*J_NOT_NULL execute)(const cir_pass_context &),
                       functions::calling_conventions_t ccs,
                       const char * J_NOT_NULL name, const char * J_NOT_NULL title, bool modifies_graph = false)
      : compilation_pass_base(name, title, modifies_graph),
        execute(execute),
        enabled_calling_conventions(ccs)
    { }
  };
}
