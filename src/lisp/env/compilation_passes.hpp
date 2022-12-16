#pragma once

#include "strings/string.hpp"

namespace j::mem {
  class bump_pool;
}

namespace j::lisp::env {
  /// Whether a compilation pass is required, enabled by default, or disabled by default.
  enum pass_necessity : i8_t {
    pass_optional = -1,
    pass_enabled_by_default,
    pass_mandatory,
  };

  enum modifies_graph_t{modifies_graph_tag};

  /// Base class for compilation pass definitions.
  ///
  /// Mostly this is missing the function pointer to the pass.
  struct compilation_pass_base {
    /// Name of the pass, usable in filenames and CLI arguments.
    const char * name;
    /// Human-readable title of the pass, suitable for status and error messages.
    const char * title;
    /// Whether the pass modifies the structure of the expression tree or the SSA/CFG of the function.
    ///
    /// This is used to skip dumping the graph if debug dumps are enabled. E.g. register allocation
    /// does not change the structure of the pass, as won't validation passes.
    bool modifies_graph = true;

    consteval compilation_pass_base(const char * name, const char * title, bool modifies_graph = false)
      : name(name),
        title(title),
        modifies_graph(modifies_graph)
    { }

    consteval compilation_pass_base(const char * name, const char * title, modifies_graph_t)
      : name(name),
        title(title),
        modifies_graph(true)
    { }
  };

  struct compilation_pass_instance_base {
    const compilation_pass_base * pass;
    pass_necessity necessity = pass_enabled_by_default;
    const char * name_override = nullptr;
    const char * title_override = nullptr;

    J_A(RNN,AI,ND,NODISC) inline const char * get_name() const noexcept {
      return name_override ? name_override : pass->name;
    }

    J_A(RNN,AI,ND,NODISC) inline const char * get_title() const noexcept {
      return title_override ? title_override : pass->title;
    }
  };

  template<typename T>
  struct compilation_pass_instance : compilation_pass_instance_base {
    compilation_pass_instance(const T * J_NOT_NULL pass, const char * J_NOT_NULL name, const char * J_NOT_NULL title, pass_necessity necessity = pass_enabled_by_default) noexcept
      : compilation_pass_instance_base{
          .pass = pass,
          .necessity = necessity,
          .name_override = name,
          .title_override = title,
        }
    { }

    compilation_pass_instance(const T * J_NOT_NULL pass, pass_necessity necessity = pass_enabled_by_default) noexcept
      : compilation_pass_instance_base{
          .pass = pass,
          .necessity = necessity,
        }
    { }

    J_A(RNN,AI,ND,NODISC) inline const T * operator->() const noexcept {
      return (const T*)pass;
    }

  };

  /// Base class for the context passed to compilation pass functions.
  struct compilation_pass_context_base {
    J_A(AI) inline compilation_pass_context_base(j::mem::bump_pool * J_NOT_NULL pool,
                                                 strings::const_string_view substituted_dump_path,
                                                 bool dump_debug_files,
                                                 bool print_debug)
      : pool(pool),
        substituted_dump_path(substituted_dump_path),
        dump_debug_files(dump_debug_files),
        print_debug(print_debug)
    { }

    j::mem::bump_pool * pool = nullptr;

    /// Path where to make any debug dumps to, minus suffix (i.e. name of the pass and file extension)
    strings::const_string_view substituted_dump_path;

    /// Whether to dump pass-specific debug files (potentially in addition to the .dot graph).
    bool dump_debug_files = false;

    /// Whether to enable debug logging to console.
    bool print_debug = false;

    /// Returns a path to the file where the dumps should be stored.
    ///
    /// No missing directories are created.
    [[nodiscard]] strings::string get_dump_file_path(const char * J_NOT_NULL suffix, bool is_error = false) const;
  };
}
