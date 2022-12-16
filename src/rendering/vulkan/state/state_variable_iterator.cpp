#include "rendering/vulkan/state/state_variable_iterator.hpp"

#include "rendering/vulkan/state/state_iterator.hpp"
#include "rendering/vulkan/state/state_variable_dag.hpp"

namespace j::rendering::vulkan::state {
  sz_t state_iterator::get_size(state_variable_dag & dag) noexcept {
    return sizeof(state_iterator) + dag.variables.size() * sizeof(state_variable_iterator);
  }

  state_iterator::state_iterator(state_variable_dag & dag) noexcept
    : m_size(dag.variables.size())
  {
    state_variable * vars[m_size];
    dag.get_state_variables(vars, m_size);
    state_variable_iterator * const it = begin();
    for (u32_t i = 0; i < m_size; ++i) {
      ::new (it + i) state_variable_iterator(dag, dag.variable_index(vars[i]));
    }
  }
}
