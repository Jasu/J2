#include "cli/cli.hpp"

#include "cli/application.hpp"
#include "services/service_instance.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::cli {
  namespace {
    namespace str = strings;
    void write_subcommand(str::const_string_view name,
                          const mem::shared_ptr<command_definition> & cmd,
                          str::formatted_sink & stdout) {
      const char * short_description = cmd->short_description();
      stdout.write("  {#bold}{}{/}{}{}\n", name, short_description ? " - " : "", short_description ? short_description : "");
    }

    void detailed_help(str::const_string_view name,
                       const command_definition * cmd,
                       str::formatted_sink & stdout)
    {
      stdout.write("{#bold}{}{/}", name);
      const auto & args = cmd->arguments_with_names();
      for (auto arg : args) {
        const char * fmt = nullptr;
        if (arg.second->is_required) {
          fmt = arg.second->is_multiple ? " {#bold}{}{/}{#light_gray}...{/}" : "{#bold}{}{/}";
        } else {
          fmt = arg.second->is_multiple ? " {#light_gray}[{#bold}{}{/}...]{/}" : "{#light_gray}[{#bold}{}{/}]{/}";
        }
        stdout.write(fmt, *arg.first);
      }
      stdout.write("\n");

      const char * short_description = cmd->short_description();
      if (short_description) {
        stdout.write("\n{}\n", short_description);
      }

      if (const char * long_description = cmd->long_description()) {
        stdout.write("\n{}\n", long_description);
      }

      const auto & options = cmd->options();
      if (!options.empty()) {
        stdout.write("\n{#bold}Options:{/}\n\n");
        for (auto & option : options) {
          stdout.write("  --{#bold}{}{/}{}{}\n", option.first, option.second.short_description ? " - " : "", option.second.short_description ? option.second.short_description : "");
        }
      }

      const bool has_subcommands = !cmd->subcommands().empty();
      if (has_subcommands) {
        stdout.write("\n{#bold}Subcommands:{/}\n\n");
        for (auto sub : cmd->subcommands()) {
          write_subcommand(*sub.first, *sub.second, stdout);
        }
      }
    }

    void help(const mem::shared_ptr<application> & app,
              const mem::shared_ptr<str::formatted_sink> & stdout,
              vector<str::string> args) {
      span<str::string> commands(args.begin(), args.end());
      const command_definition * cmd = app.get();
      str::const_string_view name = app->executable_name();
      while (commands) {
        stdout->write("{#bold}{}{/} ", name);
        name = commands.pop_front();
        cmd = cmd->maybe_get_subcommand(name);
        if (!cmd) {
          stdout->write("{#bold,bright_red}{}{/} {#bright_red} - Subcommand not found.{/}\n", name);
          return;
        }
      }
      detailed_help(name, cmd, *stdout);
    }
  }

  namespace s = services;
  J_A(ND, NODESTROY) const s::service_instance<command> cmd(
    "cli.help",
    "Show command-line help",

    command_name = "help",
    callback     = s::call(&help,
                           s::arg::autowire,
                           s::service("stdout"),
                           argument("SUBCOMMAND", a::is_multiple)));
}
