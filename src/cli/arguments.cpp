#include "cli/arguments.hpp"

#include "services/container/injection_context.hpp"
#include "services/container/service_instance_state.hpp"
#include "exceptions/assert_lite.hpp"
#include "mem/shared_ptr.hpp"
#include "cli/command.hpp"
#include "containers/deque.hpp"
#include "containers/vector.hpp"

J_DEFINE_EXTERN_DEQUE(j::pair<j::strings::const_string_view, j::cli::argument_definition>);

namespace j::cli::detail {
  vector<strings::const_string_view> get_argument_as_strings(
    const argument_definition & definition,
    const services::injection_context * ic
  ) {
    J_ASSERT_NOT_NULL(ic, ic->service_instance_state);
    auto cmd_ptr = ic->service_instance_state->get_if_instantiated();
    J_ASSERT_NOT_NULL(cmd_ptr);
    auto & cmd = *mem::static_pointer_cast<command>(cmd_ptr);

    vector<strings::const_string_view> result;
    if (auto arg = cmd.parsed_arguments().maybe_at(definition.index)) {
      J_REQUIRE(definition.is_multiple || arg->size() == 1U, "Argument can only be specified once.");
      for (const char * str : *arg) {
        result.push_back(str);
      }
    } else if (definition.is_required) {
      J_THROW("Required argument missing");
    }

    return result;
  }
}
