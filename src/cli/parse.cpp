#include "cli/parse.hpp"

#include "cli/command_definition.hpp"

namespace j::cli {
  namespace {
    inline command_definition * parse_command(
      command_definition * command,
      span<const char *> & argv,
      parse_callback_t callback,
      void * user_data
    ) {
      span<const argument_definition> arguments = command->arguments();
      parsed_argument * first_argument = nullptr;
      parsed_argument ** current_argument = &first_argument;
      while (argv) {
        const char * arg{argv.pop_front()};
        if (*arg == '-' && arg[1]) {
          ++arg;
          if (*arg == '-') {
            ++arg;
            J_REQUIRE(*arg, "TODO: \"--\" on cli.");

            i32_t name_len = j::strcspn(arg, "=");
            auto & desc = command->get_option(strings::const_string_view{arg, name_len});
            *current_argument = J_ALLOCA_NEW(parsed_argument){
              .value = !desc.has_value
              ? arg : (arg[name_len]
                       ? arg + name_len + 1
                       : argv.pop_front()),
              .argument = desc,
            };
            current_argument = &(*current_argument)->next;
          } else {
            while (*arg) {
              auto & desc = command->get_option(strings::const_string_view{arg, 1});
              *current_argument = J_ALLOCA_NEW(parsed_argument){
                .value = !desc.has_value
                ? arg : (arg[1]
                         ? arg + 1
                         : argv.pop_front()),
                .argument = desc,
              };
              current_argument = &(*current_argument)->next;
              if (desc.has_value) {
                break;
              }
              ++arg;
            }
          }
        } else if ((!arguments || !arguments.front().is_required) && command->has_subcommand(arg)) {
          callback(command, first_argument, false, user_data);
          return &command->get_subcommand(arg);
        } else {
          J_REQUIRE(!arguments.empty(), "Unexpected argument. TODO handle this better");
          *current_argument = J_ALLOCA_NEW(parsed_argument){
            .value = arg,
            .argument = arguments.front(),
          };
          current_argument = &(*current_argument)->next;
          if (!arguments.front().is_multiple) {
            arguments.pop_front();
          }
        }
      }
      if (!arguments.empty() && arguments.front().is_required) {
        J_THROW("Missing required argument.");
      }
      callback(command, first_argument, true, user_data);
      return nullptr;
    }
  }

  void parse(command_definition * command, span<const char*> argv, parse_callback_t callback, void * user_data) {
    J_ASSERT_NOT_NULL(command, callback);
    J_ASSERT(!command->is_subcommand(), "The main command cannot have a parnent.");
    J_ASSERT(!argv.empty(), "Argv was empty - at least executable path must be present.");
    argv.pop_front(); // Strip the executable name.
    do {
      command = parse_command(command, argv, callback, user_data);
    } while (command);
  }
}
