#include "cli/command.hpp"

#include "mem/shared_ptr.hpp"
#include "services/service_definition.hpp"
#include "services/detail/dependencies_t.hpp"
#include "cli/application.hpp"
#include "cli/metadata.hpp"
#include "cli/callback.hpp"

namespace j::cli {
  command::command(
    services::container * c,
    const services::injection_context * ic,
    mem::shared_ptr<command> parent_command,
    const char * subcommand_name,
    const char * short_description,
    const char * description,
    const detail::callback & callback)
    : command_definition(short_description, description),
      m_container(c),
      m_service_metadata(ic->service_metadata),
      m_interface_metadata(ic->interface_metadata),
      m_service_instance_state(ic->service_instance_state),
      m_invoke(callback.invoke),
      m_parent_command(static_cast<mem::shared_ptr<command> &&>(parent_command)),
      m_subcommand_name(subcommand_name)
  {
    J_ASSERT_NOT_NULL(m_container, m_subcommand_name);
    for (auto & arg : callback.arguments) {
      if (arg.second.is_option) {
        add_option(arg.first, arg.second);
      } else {
        add_argument(arg.first, arg.second);
      }
    }
  }

  void command::execute() {
    const services::injection_context ic{
      .interface_metadata = m_interface_metadata,
      .service_metadata = m_service_metadata,
      .service_instance_state = m_service_instance_state,
    };
    services::detail::dependencies_t deps;
    m_invoke(m_container, &ic, &deps);
  }

  command::~command() {
  }

  namespace {
    namespace s = services;
    namespace a = services::arg;
    J_A(ND, NODESTROY) s::service_definition<command> command_def(
      "cli.command",
      "CLI command",
      s::no_default_instance,
      s::initialize_by_default = s::initialization_stage::after_static_configuration,
      s::create = s::constructor<
        services::container *,
        const services::injection_context *,
        mem::shared_ptr<command>,
        const char *,
        const char *,
        const char *,
        const detail::callback &
      >(
        a::autowire,
        a::autowire,
        a::service_attribute(parent_command, a::default_value = mem::shared_ptr<command>{}),
        a::service_attribute(command_name, a::default_value = nullptr),
        a::service_title,
        a::service_attribute(description, a::default_value = nullptr),
        a::service_attribute(callback)
      )
    );
  }
}
