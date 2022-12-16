#include "lisp/env/compilation_options.hpp"
#include "lisp/env/compilation_passes.hpp"
#include "exceptions/assert_lite.hpp"
#include "logging/global.hpp"
#include "strings/string_map.hpp"

J_DEFINE_EXTERN_STRING_MAP(j::lisp::env::resolved_pass_options);

namespace j::lisp::env {
  [[nodiscard]] strings::string compilation_pass_context_base::get_dump_file_path(const char * J_NOT_NULL suffix, bool is_error) const {
      return (dump_debug_files || is_error)
        ? substituted_dump_path + suffix
        : strings::string{};
    }
  phase_options::phase_options(const global_pass_options & pass_defaults, bool dump_initial_graph) noexcept
    : pass_defaults(pass_defaults),
      dump_initial_graph(dump_initial_graph)
  {
  }

  global_pass_options apply_global_pass_overrides(
    const global_pass_options & defaults,
    const pass_options & overrides
  ) noexcept {
    J_ASSERT(defaults.dump_file_pattern);
    return {
      .print_debug = overrides.print_debug / defaults.print_debug,
      .dump_graph_after = overrides.dump_graph_after / defaults.dump_graph_after,
      .dump_debug_files = overrides.dump_debug_files / defaults.dump_debug_files,
      .dump_file_pattern = overrides.dump_file_pattern ? overrides.dump_file_pattern : defaults.dump_file_pattern,
    };
  }

  void phase_options::configure_pass(strings::const_string_view name, const pass_options & overrides) {
    // This basically checks that the configuration is initialized with the defaults.
    J_ASSERT_NOT_NULL(pass_defaults.dump_file_pattern, name);
    const bool did_insert = per_pass.emplace(
      name,
      resolved_pass_options{
        {
          .print_debug = overrides.print_debug / pass_defaults.print_debug,
          .dump_graph_after = overrides.dump_graph_after / pass_defaults.dump_graph_after,
          .dump_debug_files = overrides.dump_debug_files / pass_defaults.dump_debug_files,
          .dump_file_pattern = overrides.dump_file_pattern ? overrides.dump_file_pattern : pass_defaults.dump_file_pattern,
        },
        overrides.enabled,
      }
    ).second;
    J_ASSUME(did_insert);
  }

  bool should_run_pass(const compilation_pass_instance_base & pass,
                       const resolved_pass_options * pass_opts) noexcept
  {
      if (pass_opts && pass_opts->enabled.is_true()) {
        return true;
      }
      const bool is_disabled = pass_opts && pass_opts->enabled.is_false();
      const bool is_optional = pass.necessity == pass_optional;
      if (is_disabled || is_optional) {
        if (pass_opts && pass_opts->print_debug) {
          J_WARNING_IF(pass.necessity != env::pass_mandatory,
                       "{#bright_yellow}Disabling mandatory pass){/} {#bright_yellow_bg,black,bold} {} {/}", pass.get_name());
          J_DEBUG("{#bold,bright_yellow_bg,black} Skipping pass {#bold}{}{/} -{}{} pass ", pass.get_name(), is_disabled ? " disabled" : "", is_optional ? " optional" : "");
        }
        return false;
      }
      return true;
  }

  compilation_options::compilation_options(const assembly::target * J_NOT_NULL target,
                                           trivial_array_copyable<files::path> && include_paths,
                                           bool debug) noexcept
    : target(target),
      include_paths(static_cast<trivial_array_copyable<files::path> &&>(include_paths)),
      air_options(debug ? air_options_debug : air_options_nodebug),
      cir_options(debug ? cir_options_debug : cir_options_nodebug)
  { }

  compilation_options::compilation_options(const phase_options & air_options,
                                           const phase_options & cir_options) noexcept
    : air_options(air_options),
      cir_options(cir_options)
  { }

  const phase_options air_options_nodebug J_A(NODESTROY){
    {
      .print_debug = false,
      .dump_graph_after = false,
      .dump_debug_files = false,
      .dump_file_pattern = "air.{}.",
    },
    false,
  };
  const phase_options cir_options_nodebug J_A(NODESTROY){
    {
      .print_debug = false,
      .dump_graph_after = false,
      .dump_debug_files = false,
      .dump_file_pattern = "cir.{}.{}.",
    },
    false,
  };

  const phase_options air_options_debug J_A(NODESTROY){
    {
      .print_debug = true,
      .dump_graph_after = true,
      .dump_debug_files = true,
      .dump_file_pattern = "air.{}.",
    },
    true,
  };
  const phase_options cir_options_debug J_A(NODESTROY){
    {
      .print_debug = true,
      .dump_graph_after = true,
      .dump_debug_files = true,
      .dump_file_pattern = "cir.{}.{}.",
    },
    true,
  };

  const compilation_options compilation_options_nodebug J_A(NODESTROY){
    air_options_nodebug,
    /// Options for CIR compilation phase. Configures passes and debug.
    cir_options_nodebug
  };

  const compilation_options compilation_options_debug J_A(NODESTROY){
    air_options_debug,
    /// Options for CIR compilation phase. Configures passes and debug.
    cir_options_debug,
  };
}
