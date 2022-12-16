#pragma once

#include "cli/command_definition.hpp"
#include "containers/ptr_set.hpp"
#include "containers/hash_map_fwd.hpp"

J_DECLARE_EXTERN_HASH_MAP(u32_t, j::ptr_array<const char *>);

namespace j::inline containers {
  template<typename T> class vector;
}

namespace j::cli {
  class command;
  /// The main CLI application, which manages configuration and state for CLI
  /// argument and command processing.
  class application final : public command_definition {
  public:
    application(span<const char *> arguments, vector<mem::shared_ptr<command>> && commands) noexcept;

    application(const application &) = delete;
    application & operator=(const application &) = delete;

    ~application();

    strings::const_string_view executable_name() const noexcept;

    void parse();
    void execute();
    bool has_option(u32_t index) const noexcept;
    span<const char *> get_option(u32_t index) noexcept;
    J_HIDDEN void set_parsed_command(command * cmd) noexcept;
    hash_map<u32_t, ptr_array<const char*>> parsed_arguments;
  private:
    span<const char *> m_arguments;
    u32_t m_option_counter:31;
    bool m_is_parsed:1;
    command * m_parsed_command = nullptr;
    friend class cli_configuration_pass;
  };
}
