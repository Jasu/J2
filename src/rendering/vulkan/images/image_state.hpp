#pragma once

#include <vulkan/vulkan.h>

#include "rendering/vulkan/state/persistent_scalar_variable.hpp"

#include "rendering/vulkan/images/image_resource.hpp"
#include "rendering/images/image_source_key.hpp"

namespace j::rendering::vulkan::images {
  struct image_state {
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    j::rendering::images::image_source_key contents;
  };

  class image_layout_var final : public state::persistent_scalar_variable<image_ref, &image_state::layout> {
  public:
    using state::persistent_scalar_variable<image_ref, &image_state::layout>::persistent_scalar_variable;
  protected:
    bool can_transition(
      const state::transition_context & context,
      VkImageLayout from,
      VkImageLayout to) const override;

    state::condition_group * transition(
      state::transition_context & context,
      image_ref ref,
      VkImageLayout from,
      VkImageLayout to) const override;

    strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::precondition_instance & c
    ) const override;

    strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::postcondition_instance & c
    ) const override;
  };

  class image_contents_var final : public state::persistent_scalar_variable<image_ref, &image_state::contents> {
  protected:
    bool can_transition(
      const state::transition_context & context,
      j::rendering::images::image_source_key from,
      j::rendering::images::image_source_key to) const override;

    state::condition_group * transition(
      state::transition_context & context,
      image_ref ref,
      j::rendering::images::image_source_key from,
      j::rendering::images::image_source_key to) const override;

    strings::string describe_precondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::precondition_instance & c
    ) const override;

    strings::string describe_postcondition(
      resources::resource_wrapper * wrapper,
      uptr_t detail,
      const state::postcondition_instance & c
    ) const override;

  public:
    using state::persistent_scalar_variable<image_ref, &image_state::contents>::persistent_scalar_variable;
  };

  extern const image_contents_var image_content;
  extern const image_layout_var image_layout;
}
