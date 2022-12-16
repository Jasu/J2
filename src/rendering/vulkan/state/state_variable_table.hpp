#pragma once

#include "rendering/vulkan/state/state_variable.hpp"
#include "mem/typed_arena.hpp"

namespace j::rendering::vulkan::state {
  class state_variable_table final {
  public:
    /// Get index to a state variable, creating it if it does not exist.
    u32_t get_or_create(state_variable_key key,
                        const state_variable_definition_base * def);

    /// Access a state variable, with bounds checking.
    state_variable & at(u32_t i);

    /// Access a state variable, with bounds checking.
    const state_variable & at(u32_t i) const {
      return const_cast<state_variable_table*>(this)->at(i);
    }

    /// Access a state variable, without bounds checking.
    J_INLINE_GETTER state_variable & operator[](u32_t i) noexcept {
      return m_variables[get_index(i)];
    }

    /// Access a state variable, without bounds checking.
    J_INLINE_GETTER const state_variable & operator[](u32_t i) const noexcept {
      return m_variables[get_index(i)];
    }

    J_INLINE_GETTER state_variable * begin() noexcept {
      return m_variables.begin();
    }

    J_INLINE_GETTER const state_variable * begin() const noexcept {
      return m_variables.begin();
    }

    J_INLINE_GETTER state_variable * end() noexcept {
      return m_variables.end();
    }

    J_INLINE_GETTER const state_variable * end() const noexcept {
      return m_variables.end();
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return m_variables.size();
    }
    void clear() noexcept;
  private:
    mem::typed_arena<state_variable> m_variables;
  };
}
