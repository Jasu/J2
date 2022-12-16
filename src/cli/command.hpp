#pragma once

#include "cli/command_definition.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "cli/arguments.hpp"
#include "services/injected_calls/stored_injected_call.hpp"
#include "containers/ptr_set.hpp"
#include "containers/hash_map_fwd.hpp"

namespace j::services {
  class container;
  struct injection_context;
}

J_DECLARE_EXTERN_HASH_MAP(u32_t, j::ptr_array<const char *>);

namespace j::cli {
  class application;
  namespace detail {
    struct callback;
  }

  class command final : public command_definition {
  public:
    using parsed_arguments_t = hash_map<u32_t, ptr_array<const char*>>;
    command(
      services::container * c,
      const services::injection_context * ic,
      mem::shared_ptr<command> parent_command,
      const char * subcommand_name,
      const char * short_description,
      const char * description,
      const detail::callback & callback);

    command(command &&) noexcept = delete;
    command & operator=(command &&) = delete;

    ~command();


    void execute();

    J_INLINE_GETTER parsed_arguments_t & parsed_arguments() noexcept
    { return m_parsed_arguments; }

    J_INLINE_GETTER const parsed_arguments_t & parsed_arguments() const noexcept
    { return m_parsed_arguments; }

    J_INLINE_GETTER const mem::shared_ptr<command> & parent_command() const noexcept
    { return m_parent_command; }

    J_INLINE_GETTER mem::shared_ptr<command> & parent_command() noexcept
    { return m_parent_command; }

    J_INLINE_GETTER const char * subcommand_name() const noexcept
    { return m_subcommand_name; }

  private:
    parsed_arguments_t m_parsed_arguments;

    services::container * m_container;
    const services::class_metadata * m_service_metadata;
    const services::class_metadata * m_interface_metadata;
    services::detail::container::service_instance_state * m_service_instance_state;
    services::injected_calls::stored_injected_call<void> m_invoke;
    mem::shared_ptr<command> m_parent_command;
    const char * m_subcommand_name;
  };
}
