#pragma once

#include "hzd/types.hpp"

namespace j::rendering::vulkan::inline context {
  struct render_context;
}

namespace j::rendering::vulkan::state {
  class state_tracker;
  class condition_group;
  struct condition_initializer;
  struct postcondition_initializer;
  struct precondition_initializer;
  class condition_instance;
  class postcondition_instance;
  class precondition_instance;

  /// Context passed to commands when they are inserted into the state graph.
  ///
  /// This is also passed when merging nodes.
  struct node_insertion_context {
    /// Reference to the state tracker where the node is being inserted to.
    state_tracker & tracker;
    /// The Vulkan context of the instance.
    const render_context & render_context;

    u32_t start_index = 0;

    void begin_add_node() noexcept;

    void add_condition(const condition_initializer & c);
    void add_precondition(const precondition_initializer & precondition);
    void add_postcondition(const postcondition_initializer & postcondition);

    void move_condition(condition_instance && c);
    void move_condition(condition_group & g, u32_t i);

    void finish_add_node(condition_group * g) noexcept;
  };
}
