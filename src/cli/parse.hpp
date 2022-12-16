#pragma once

#include "cli/argument_definition.hpp"
#include "containers/span.hpp"

namespace j::cli {
  class command_definition;

  struct parsed_argument {
    const char * value = nullptr;
    argument_definition argument;
    parsed_argument * next = nullptr;
  };

  using parse_callback_t = void (*)(command_definition *, const parsed_argument *, bool is_last_command, void * user_data);

  /// Call callback for each (sub)command specified in argv.
  ///
  /// \param main_command The main (top-level) command, where the parsing starts.
  /// This must be a (i.e. the) top-level command - otherwise, an exception is thrown.
  /// \param argv The arguments, wrapped as a span.
  /// \param callback Function to call for every command parsed.
  /// Callback is called first for the top-level command, then for the first subcommand, etc.
  /// If parsing of a subcommand fails, an exception is thrown, but only after callback
  /// has been called for the previously parsed commands.
  void parse(command_definition * main_command, span<const char *> argv, parse_callback_t callback, void * user_data);
}
