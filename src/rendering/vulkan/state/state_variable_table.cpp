#include "rendering/vulkan/state/state_variable_table.hpp"
#include "hzd/mem.hpp"

namespace j::rendering::vulkan::state {

  u32_t state_variable_table::get_or_create(
    state_variable_key key, const state_variable_definition_base * const def)
  {
    J_ASSERT_NOT_NULL(def);
    const state_variable * const begin = m_variables.begin(),
      * const end = m_variables.end(),
      * cur = begin;
    for (; cur != end; ++cur) {
      if (key == *cur) {
        return cur - begin;
      }
    }
    const u32_t i = cur - begin;
    m_variables.emplace(i, key, def);
    return i;
  }

  state_variable & state_variable_table::at(u32_t i) {
    i = get_index(i);
    J_ASSERT(i < m_variables.size(), "Out of range");
    return m_variables[i];
  }

  void state_variable_table::clear() noexcept {
    m_variables.clear(mem::keep_allocation);
  }
}
