#include "cli/command_definition.hpp"
#include "strings/string_view.hpp"
#include "containers/unsorted_string_map.hpp"
#include "strings/string_map.hpp"
#include "cli/argument_definition.hpp"
#include "exceptions/assert_lite.hpp"
#include "strings/find.hpp"
#include "mem/shared_ptr.hpp"

J_DEFINE_EXTERN_UNSORTED_STRING_MAP(j::cli::argument_definition);
J_DEFINE_EXTERN_UNSORTED_STRING_MAP(j::mem::shared_ptr<j::cli::command_definition>);
J_DEFINE_EXTERN_STRING_MAP(j::cli::argument_definition);

namespace j::cli {
  command_definition::command_definition(const char * short_description,
                                         const char * long_description) noexcept
    : m_short_description(short_description),
      m_long_description(long_description)
  { }

  void command_definition::add_option(strings::const_string_view spec, argument_definition option) {
    J_ASSUME(option.is_option);
    J_ASSUME(!option.is_required);
    while (spec.size()) {
      i32_t end = strings::find_char(spec, '|');
      J_ASSERT(end, "Option spec cannot be empty.");
      [[maybe_unused]] bool did_insert;
      J_ASSERT(spec[0] != '-', "Option spec should not include the dash.");
      if (end == 1U) {
        // A single character - register as a short option.
        did_insert = m_options.emplace(spec.prefix(1), option).second;
      } else {
        // Multiple characters - register as a long option.
        did_insert = m_options.emplace(
          end < 0 ? spec.slice(0, end) : spec,
          option).second;
      }
      J_ASSERT(did_insert, "Duplicate option.");
      if (end < 0) {
        return;
      }
      spec.remove_prefix(end + 1);
    }
  }

  void command_definition::add_argument(strings::const_string_view name, argument_definition argument) {
    J_ASSUME(!argument.is_option);
    J_ASSUME(argument.has_value);
    J_ASSERT(!argument.is_required || m_arguments.empty() || m_arguments.values().back().is_required,
             "Required arguments cannot follow optional ones.");
    J_ASSERT(m_arguments.empty() || !m_arguments.values().back().is_multiple,
             "Multiple argument must be the last one.");
    m_arguments.emplace(name, argument);
  }

  void command_definition::add_subcommand(
    strings::const_string_view name,
    mem::shared_ptr<command_definition> command
  ) {
    J_ASSERT(!name.empty(), "Subcommand name cannot be empty.");
    J_ASSERT_NOT_NULL(command);
    J_ASSERT(!command->m_parent_command, "Subcommand is already a parent command of some other command.");
    [[maybe_unused]] const bool did_insert = m_subcommands.emplace(name, command).second;
    J_ASSERT(did_insert, "Subcommand was added twice.");
    command->m_parent_command = this;
  }

  bool command_definition::has_subcommand(strings::const_string_view name) const noexcept {
    return m_subcommands.contains(name);
  }

  command_definition & command_definition::get_subcommand(strings::const_string_view name) {
    return *m_subcommands.at(name);
  }

  const command_definition & command_definition::get_subcommand(strings::const_string_view name) const {
    return *m_subcommands.at(name);
  }

  command_definition * command_definition::maybe_get_subcommand(strings::const_string_view name) noexcept {
    const auto res = m_subcommands.maybe_at(name);
    return res ? res->get() : nullptr;
  }

  const command_definition * command_definition::maybe_get_subcommand(strings::const_string_view name) const noexcept {
    const auto res = m_subcommands.maybe_at(name);
    return res ? res->get() : nullptr;
  }

  bool command_definition::has_option(const strings::const_string_view & name) const noexcept {
    return m_options.contains(name);
  }

  const argument_definition & command_definition::get_option(const strings::const_string_view & name) const {
    return m_options.at(name);
  }

  span<const argument_definition> command_definition::arguments() const noexcept {
    return m_arguments.values();
  }
}
