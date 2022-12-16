#pragma once

#include "rendering/vulkan/state/epochs.hpp"
#include "rendering/vulkan/state/node_insertion_context.hpp"

namespace j::strings {
  class string;
}

namespace j::inline containers {
  template<typename, int> class obstack;
}
namespace j::rendering::vulkan::rendering {
  class reserved_resource;
}

namespace j::rendering::vulkan::inline command_buffers {
  struct command_context;
}

namespace j::rendering::vulkan::state {
  struct node_insertion_context;

  using reserved_resources_t = containers::obstack<rendering::reserved_resource, 8>;

  inline constexpr u16_t merges_with_siblings_flag  = 1U;
  inline constexpr u16_t merges_with_parent_flag    = 2U;
  inline constexpr u16_t prefers_bottom_insert_flag = 4U;

  enum class epoch_options : u8_t {
    no_epoch = 0U,
    bound_to_epoch,
    starts_epoch,
  };

  class condition_group {
  public:
    u32_t level = 0U;
    u16_t conditions_index = U16_MAX;
    u8_t num_preconditions = 0U, num_postconditions = 0U;
    epoch_key epoch[2];
    epoch_options epoch_opts[2] = { epoch_options::no_epoch };
    u16_t insertion_options = 0U;
  public:
    constexpr condition_group() noexcept = default;

    void starts_epoch(const epoch_definition & def) noexcept {
      const int index = epoch_opts[0] == epoch_options::no_epoch ? 0 : 1;
      epoch[index] = epoch_key(end_tag, def.index);
      epoch_opts[index] = epoch_options::starts_epoch;
    }

    void is_bound_to_current_epoch(const epoch_definition & def) noexcept {
      const int index = epoch_opts[0] == epoch_options::no_epoch ? 0 : 1;
      epoch[index] = epoch_key(end_tag, def.index);
      epoch_opts[index] = epoch_options::bound_to_epoch;
    }

    void is_bound_to_epoch(epoch_key key) noexcept {
      const int index = epoch_opts[0] == epoch_options::no_epoch ? 0 : 1;
      epoch[index] = key;
      epoch_opts[index] = epoch_options::bound_to_epoch;
    }

    virtual bool can_merge_with(const node_insertion_context & context, const condition_group & group) const noexcept;

    virtual void merge(node_insertion_context & context, condition_group && group);

    J_INLINE_GETTER u8_t size() const noexcept {
      return num_preconditions + num_postconditions;
    }

    condition_group(condition_group &&) = delete;
    condition_group(const condition_group &) = delete;
    condition_group & operator=(condition_group &&) = delete;
    condition_group & operator=(const condition_group &) = delete;

    virtual ~condition_group();

    virtual strings::string name() const = 0;

    virtual void execute(command_context & context, reserved_resources_t & resources) const = 0;

    /// Get more debug information (one line).
    virtual strings::string debug_details() const;
  };
}
