#pragma once

#include "containers/unsorted_string_map_fwd.hpp"
#include "strings/string_map_fwd.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "cli/argument_definition.hpp"

namespace j::cli {
  class command_definition;
}

J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::cli::argument_definition);
J_DECLARE_EXTERN_UNSORTED_STRING_MAP(j::mem::shared_ptr<j::cli::command_definition>);
J_DECLARE_EXTERN_STRING_MAP(j::cli::argument_definition);

namespace j::cli {
  /// Container for options, arguments, and subcommands of a command.
  ///
  /// A command definition may have:
  ///
  ///   - Options (--file=test.txt, -vvvv), i.e. optional named arguments.
  ///   - Arguments (filename.txt filename2.txt), i.e. unnamed arguments.
  ///   - Subcommands (git-like, or ip-like)
  ///
  /// Crucially, a command definition does not contain a callback, the name of
  /// the command itself, etc. - it only describes the requirements for parsing
  /// the command, but not executing it.
  class command_definition {
  public:
    using subcommands_t = uncopyable_unsorted_string_map<mem::shared_ptr<command_definition>>;
    command_definition() noexcept = default;
    command_definition(const char * short_description,
                       const char * long_description) noexcept;

    void add_option(strings::const_string_view spec, argument_definition option);
    void add_argument(strings::const_string_view name, argument_definition argument);
    void add_subcommand(strings::const_string_view name, mem::shared_ptr<command_definition> command);
    bool has_subcommand(strings::const_string_view name) const noexcept;
    command_definition & get_subcommand(strings::const_string_view name);
    const command_definition & get_subcommand(strings::const_string_view name) const;
    command_definition * maybe_get_subcommand(strings::const_string_view name) noexcept;
    const command_definition * maybe_get_subcommand(strings::const_string_view name) const noexcept;
    span<const argument_definition> arguments() const noexcept;

    bool has_option(const strings::const_string_view & name) const noexcept;
    const argument_definition & get_option(const strings::const_string_view & name) const;

    J_INLINE_GETTER bool is_subcommand() const noexcept
    { return m_parent_command; }

    J_INLINE_GETTER const auto & options() const noexcept
    { return m_options; }

    const command_definition & parent_command();

    J_INLINE_GETTER const char * short_description() const noexcept
    { return m_short_description; }

    J_INLINE_GETTER const char * long_description() const noexcept
    { return m_long_description; }

    J_INLINE_GETTER const subcommands_t & subcommands() const noexcept
    { return m_subcommands; }

    J_INLINE_GETTER const uncopyable_unsorted_string_map<argument_definition> & arguments_with_names() const noexcept {
      return m_arguments;
    }
  private:
    strings::string_map<argument_definition> m_options;
    subcommands_t m_subcommands;
    uncopyable_unsorted_string_map<argument_definition> m_arguments;
    const command_definition * m_parent_command = nullptr;
    const char * m_short_description = nullptr;
    const char * m_long_description = nullptr;
  };
}
