#include "rendering/vulkan/state/condition_group.hpp"
#include "strings/string.hpp"

namespace j::rendering::vulkan::state {
  condition_group::~condition_group() {
  }

  strings::string condition_group::debug_details() const {
    return "";
  }

  bool condition_group::can_merge_with(const node_insertion_context &,
                                       const condition_group &) const noexcept
  { J_THROW("Merge not supported."); }

  void condition_group::merge(node_insertion_context &, condition_group &&)
  { J_THROW("Merge not supported."); }
}
