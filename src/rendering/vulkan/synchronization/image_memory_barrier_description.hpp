#pragma once

#include "rendering/vulkan/synchronization/barrier_description.hpp"
#include "attributes/attribute_definition.hpp"
#include "attributes/basic_operations.hpp"
#include "rendering/vulkan/images/image_resource.hpp"

namespace j::rendering::vulkan::synchronization {
  /// The layout of the image at the end of the barrier.
  inline constexpr auto next_layout = j::attributes::attribute_definition(
    j::attributes::value_type = type<VkImageLayout>,
    j::attributes::tag = type<struct next_layout_tag>);

  /// Barrier on image memory access and/or layout transition.
  class image_memory_barrier_description final : public barrier_description {
  public:
    /// Construct an invalid [image_memory_barrier_description]
    constexpr image_memory_barrier_description() noexcept = default;

    template<typename... Args>
    J_ALWAYS_INLINE explicit image_memory_barrier_description(images::image_ref image,
                                              Args && ... args)
      : barrier_description(static_cast<Args &&>(args)...),
        m_image(static_cast<images::image_ref &&>(image)),
        m_next_layout(synchronization::next_layout.get(static_cast<Args &&>(args)...))
    {
      J_ASSERT_NOT_NULL(m_image);
      J_ASSERT(m_next_layout != VK_IMAGE_LAYOUT_UNDEFINED
               && m_next_layout != VK_IMAGE_LAYOUT_PREINITIALIZED,
               "Image cannot be transitioned to undefined or preinitialized layout.");
    }

    /// Get the new layout of the image.
    J_INLINE_GETTER VkImageLayout next_layout() const noexcept {
      return m_next_layout;
    }

    /// Return true if this barrier represents an image layout transition.
    J_INLINE_GETTER bool is_layout_transition() const noexcept {
      return m_next_layout != VK_IMAGE_LAYOUT_UNDEFINED;
    }

    /// Get the image this barrier concerns.
    J_INLINE_GETTER const images::image_ref & image() const noexcept {
      return m_image;
    }
  private:
    /// The image that this barrier concerns.
    images::image_ref m_image;

    /// Layout of the image after the barrier.
    VkImageLayout m_next_layout = VK_IMAGE_LAYOUT_UNDEFINED;
  };
}
