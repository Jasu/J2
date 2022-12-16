#pragma once

#include "rendering/vulkan/state/epochs.hpp"
#include "containers/obstack_fwd.hpp"
#include "math/interval.hpp"

namespace j::rendering::vulkan::rendering {
  class reserved_resource;
}

namespace j::mem {
  template<typename T> class unique_ptr;
}

namespace j::rendering::vulkan::inline context {
  struct render_context;
}

namespace j::rendering::vulkan::inline command_buffers {
  struct command_context;
}

namespace j::rendering::vulkan::state {
  class state_variable_dag;
  class condition_group;
  struct node_insertion_context;

  class state_tracker final {
  public:
    state_tracker();

    void define_epoch(const epoch_definition & def) noexcept;

    void add_node(const render_context & context,
                  condition_group * J_NOT_NULL command,
                  math::interval_u32 allowed_levels = math::interval_u32(math::by_endpoints, 0, U32_MAX));

    /// Execute all operations, queue all commands, and commit resource state.
    containers::obstack<rendering::reserved_resource> execute(command_buffers::command_context & context);

    /// Remove all added commands from the tracker.
    void clear() noexcept;

    ~state_tracker();

    state_tracker(state_tracker &&) = delete;
    state_tracker & operator=(state_tracker &&) = delete;

    state_variable_dag * dag;
  private:
    containers::obstack<mem::unique_ptr<condition_group>, 16> m_nodes;

    J_HIDDEN void merge_node(node_insertion_context & context,
                             mem::unique_ptr<condition_group> command,
                             condition_group * J_NOT_NULL merge_target);

  };
}
