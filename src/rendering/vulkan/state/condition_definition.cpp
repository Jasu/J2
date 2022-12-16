#include "rendering/vulkan/state/condition_definition.hpp"
#include "rendering/vulkan/state/scalar_variable.hpp"
#include "rendering/vulkan/state/state_variable.hpp"

namespace j::rendering::vulkan::state {
  condition_definition_base::~condition_definition_base() {
  }

  bool precondition_definition::get_initial_state(const state_variable &, uptr_t) const {
    return false;
  }

  bool equals_precondition::get_initial_state(const state_variable & state, uptr_t precondition_data) const {
    return state.data == precondition_data;
  }

  precondition_change equals_precondition::get_change(
    const postcondition_instance & postcondition,
    uptr_t precondition_data) const
  {
    return postcondition.data == precondition_data
      ? precondition_change::satisfied
      : precondition_change::unsatisfied;
  }
}
