#pragma once

#include "rendering/vulkan/state/node_insertion_context.hpp"
#include "rendering/vulkan/state/conditions.hpp"

namespace j::strings {
  class string;
}

namespace j::rendering::vulkan::state {
  class state_tracker;
  class condition_group;
  class precondition_instance;
  class postcondition_instance;
  struct precondition_initializer;
  struct postcondition_initializer;

  struct state_context {
    uptr_t index;
    resources::resource_wrapper * wrapper;
    uptr_t initial_state;
    postcondition_instance * first_postcondition;
    postcondition_instance * last_postcondition;

    uptr_t & latest_state() noexcept {
      if (last_postcondition) {
        return last_postcondition->data;
      }
      return initial_state;
    }

    const uptr_t & latest_state() const noexcept {
      return const_cast<state_context*>(this)->latest_state();
    }
  };

  struct transition_context final : state_context {
    const precondition_instance & to;
    node_insertion_context & context;
  };

  class state_variable_definition_base {
    friend struct state_variable;
  protected:
    virtual void initialize_state(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      uptr_t & state
    ) const noexcept;

    virtual void commit(state_context & context) const;

    virtual bool do_can_transition(const transition_context & context) const;

    virtual condition_group * do_transition(transition_context & context) const;

  public:
    constexpr state_variable_definition_base() noexcept { }
    virtual strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const precondition_instance & c
    ) const = 0;

    virtual strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t index,
      const postcondition_instance & c
    ) const = 0;

    virtual ~state_variable_definition_base();
  };
}
