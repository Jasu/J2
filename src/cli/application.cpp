#include "cli/application.hpp"
#include "exceptions/assert_lite.hpp"
#include "logging/global.hpp"
#include "cli/parse.hpp"
#include "cli/command.hpp"
#include "strings/string_algo.hpp"
#include "services/service_definition.hpp"
#include "properties/class_registration.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(u32_t, j::ptr_array<const char *>);

namespace j::cli {
  namespace {
    void parse_callback(command_definition * cmd_definition,
                        const parsed_argument * arg,
                        bool is_last_command,
                        void * app_
    ) {
      application * app = reinterpret_cast<application*>(app_);
      J_ASSERT_NOT_NULL(app, cmd_definition);
      if (cmd_definition->is_subcommand() && is_last_command) {
        app->set_parsed_command(static_cast<command*>(cmd_definition));
      }
      while (arg) {
        if (cmd_definition->is_subcommand()) {
          static_cast<command*>(cmd_definition)->parsed_arguments()[arg->argument.index].push_back(arg->value);
        } else {
          app->parsed_arguments[arg->argument.index].push_back(arg->value);
        }
        arg = arg->next;
      }
    }
  }
  application::application(span<const char *> arguments,
                           vector<mem::shared_ptr<command>> && commands
  ) noexcept
  : m_arguments(arguments),
    m_option_counter(0U),
    m_is_parsed(false)
  {
    for (auto && command : commands) {
      command_definition * def = command->is_subcommand()
        ? command->parent_command().get()
        : static_cast<command_definition*>(this);
      def->add_subcommand(command->subcommand_name(), static_cast<mem::shared_ptr<cli::command> &&>(command));
    }
  }

  strings::const_string_view application::executable_name() const noexcept {
    J_ASSERT_NOT_NULL(m_arguments);
    return strings::suffix_until(strings::const_string_view(m_arguments[0]), '/');
  }

  bool application::has_option(u32_t index) const noexcept {
    return parsed_arguments.contains(index);
  }

  span<const char *> application::get_option(u32_t index) noexcept {
    if (auto ptrs = parsed_arguments.maybe_at(index)) {
      return *ptrs;
    }
    return {};
  }

  void application::parse() {
    if (m_is_parsed) {
      return;
    }
    cli::parse(this, m_arguments, &parse_callback, this);
    m_is_parsed = true;
  }

  void application::execute() {
    parse();
    if (m_parsed_command) {
      m_parsed_command->execute();
    }
  }

  application::~application() {
  }

  J_HIDDEN void application::set_parsed_command(command * cmd) noexcept {
    m_parsed_command = cmd;
  }

  namespace {
    namespace s = services;
    namespace a = services::arg;
    J_A(ND, NODESTROY) const s::service_definition<application> app_definition(
      "cli.app",
      "CLI App",
      s::initialize_by_default = s::initialization_stage::after_static_configuration,
      s::setting = s::setting_object<span<const char *>>("command_line_arguments"),
      s::create = s::constructor<span<const char *>, vector<mem::shared_ptr<command>> &&>(a::setting("command_line_arguments"), a::autowire),
      s::global_singleton
    );

    J_A(ND, NODESTROY) properties::object_access_registration<span<const char *>> reg{"command_line_arguments"};
  }
}
