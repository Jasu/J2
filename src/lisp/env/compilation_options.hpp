#pragma once

#include "util/tristate.hpp"
#include "strings/string_map_fwd.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "files/paths/path.hpp"
#include "containers/trivial_array_fwd.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY_COPYABLE(j::files::path);

namespace j::lisp::assembly {
  class target;
}
namespace j::lisp::env {
  struct global_pass_options {
    /// Whether to output debug messages to console.
    bool print_debug = false;
    /// Whether AIR's expression tree or CIR's SSA graph should be saved on disk after this pass.
    bool dump_graph_after = false;
    /// Whether pass-specific debug files other than the graph should be saved to disk.
    bool dump_debug_files = false;
    /// Path pattern for saving debug files. Substitutions differ for AIR and CIR.
    strings::const_string_view dump_file_pattern;
  };

  /// Pass options after all defaults are resolved.
  struct resolved_pass_options final : global_pass_options {
    /// Whether the pass is explicitly enabled or disabled.
    util::tristate enabled;
  };

  /// Overrides or defaults for compilation passes (both AIR- and CIR-passes).
  ///
  /// Used both for global defaults and per-pass overrides.
  struct pass_options final {
    /// Whether the pass is enabled at all.
    ///
    /// Passes that are mandatory cannot be disabled.
    util::tristate enabled;
    /// Whether to output debug messages to console.
    util::tristate print_debug;
    /// Whether AIR's expression tree or CIR's SSA graph should be saved on disk after this pass.
    util::tristate dump_graph_after;
    /// Whether pass-specific debug files other than the graph should be saved to disk.
    util::tristate dump_debug_files;
    /// Path pattern for saving debug files. Substitutions differ for AIR and CIR.
    strings::const_string_view dump_file_pattern;
  };

  /// Apply `overrides` on top of `defaults`.
  ///
  /// This is used to create the per-phase (not per pass) default values.
  global_pass_options apply_global_pass_overrides(const global_pass_options & defaults,
                                                  const pass_options & overrides) noexcept;
}

J_DECLARE_EXTERN_STRING_MAP(j::lisp::env::resolved_pass_options);

namespace j::lisp::env {
  struct phase_options final {
    J_A(AI,ND) inline phase_options() noexcept = default;
    explicit phase_options(const global_pass_options & pass_defaults, bool dump_initial_graph = true) noexcept;
    /// Default configuration to apply to the passes of this phase.
    global_pass_options pass_defaults;
    /// Per-pass configuration. If not found, `pass_defaults` are used.
    strings::string_map<resolved_pass_options> per_pass;
    /// Whether to dump the graph before any passes are run.
    bool dump_initial_graph = true;

    /// Apply pass-specific overrides.
    ///
    /// Passes without overrides use `pass_defaults`.
    void configure_pass(strings::const_string_view name,
                        const pass_options & overrides);
  };

  extern const phase_options air_options_nodebug J_A(NODESTROY);
  extern const phase_options air_options_debug J_A(NODESTROY);
  extern const phase_options cir_options_nodebug J_A(NODESTROY);
  extern const phase_options cir_options_debug J_A(NODESTROY);

  struct compilation_options final {
    compilation_options(const assembly::target * J_NOT_NULL target,
                        trivial_array_copyable<files::path> && include_paths,
                        bool debug) noexcept,

    compilation_options(const phase_options & air_options,
                        const phase_options & cir_options) noexcept;

    J_A(AI,ND,HIDDEN) inline compilation_options(bool debug) noexcept
      : compilation_options(debug ? air_options_debug : air_options_nodebug,
                            debug ? cir_options_debug : cir_options_nodebug)
    { }

    /// The targeted architecture (HW + OS)
    const assembly::target *  target = nullptr;

    /// Directories to search packages for.
    trivial_array_copyable<files::path> include_paths;

    /// Options for AIR compilation phase. Configures passes and debug.
    phase_options air_options;
    /// Options for CIR compilation phase. Configures passes and debug.
    phase_options cir_options;
  };

  struct compilation_pass_instance_base;

  [[nodiscard]] bool should_run_pass(const compilation_pass_instance_base & pass,
                       const resolved_pass_options * pass_opts) noexcept;

  extern const compilation_options compilation_options_nodebug J_A(NODESTROY);
  extern const compilation_options compilation_options_debug J_A(NODESTROY);

}
