#pragma once

#include "services/service_reference.hpp"
#include "cli/common_attributes.hpp"
#include "cli/callback.hpp"

namespace j::cli {
  namespace s = services;

  /// Attribute specifying the parent command of a subcommand.
  inline constexpr a::attribute_definition parent_command{
    a::value_type = type<s::service_reference>,
    a::tag = type<struct parent_command_tag>};

  /// Attribute specifying the command name for a subcommand.
  inline constexpr a::attribute_definition command_name{
    a::value_type = type<const char *>,
    a::tag = type<struct command_name_tag>};

  /// Attribute specifying the callback for a command.
  inline constexpr a::attribute_definition callback{
    a::value_type = type<detail::callback>,
    a::tag = type<detail::callback>};

  inline constexpr a::attribute_definition cli_option(
    a::value_type = type<const char *>,
    a::tag = type<struct option_tag>);
}
