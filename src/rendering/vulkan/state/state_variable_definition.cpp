#include "rendering/vulkan/state/state_variable_definition.hpp"
#include "rendering/vulkan/state/conditions.hpp"

namespace j::rendering::vulkan::state {
  state_variable_definition_base::~state_variable_definition_base() {
  }

  bool state_variable_definition_base::do_can_transition(const transition_context &) const {
    return false;
  }

  condition_group * state_variable_definition_base::do_transition(transition_context &) const {
    J_THROW("Automatic transitions not supported.");
  }

  void state_variable_definition_base::initialize_state(
    resources::resource_wrapper *, uptr_t, uptr_t & state) const noexcept
  { state = 0; }

  void state_variable_definition_base::commit(state_context &) const
  { }
}
