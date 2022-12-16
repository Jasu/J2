#pragma once

#include <vulkan/vulkan.h>
#include "containers/trivial_array_fwd.hpp"
#include "containers/pair.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(VkQueueFamilyProperties);

namespace j::windowing::x11 {
  class context;
}

namespace j::rendering::vulkan::physical_devices {
  /// Information about all queue families on a physical device.
  class queue_family_set {
  public:
    queue_family_set() noexcept = default;
    explicit queue_family_set(VkPhysicalDevice vk_physical_device);

    /// Finds the first queue that has all bits in flags set.
    ///
    /// If no such queue is find, returns end().
    pair<const VkQueueFamilyProperties *, u32_t> find_queue_with_flags(u32_t flags) const noexcept;

    /// Finds the first queue with graphics capability.
    ///
    /// If no such queue is find, returns end().
    pair<const VkQueueFamilyProperties *, u32_t> find_first_graphics_queue() const noexcept;

    /// Finds the first queue capable of presenting for context.
    ///
    /// If no such queue is find, returns end().
    pair<const VkQueueFamilyProperties *, u32_t> find_first_present_queue_for(::j::windowing::x11::context & context) const noexcept;

    J_INLINE_GETTER const VkQueueFamilyProperties * begin() const noexcept {
      return m_queue_families.begin();
    }

    J_INLINE_GETTER const VkQueueFamilyProperties * end() const noexcept {
      return m_queue_families.end();
    }

    J_INLINE_GETTER bool empty() const noexcept {
      return m_queue_families.empty();
    }

    J_INLINE_GETTER u32_t size() const noexcept {
      return m_queue_families.size();
    }

    J_INLINE_GETTER const VkQueueFamilyProperties & operator[](u32_t index) const noexcept {
      return m_queue_families[index];
    }
  private:
    trivial_array<VkQueueFamilyProperties> m_queue_families;
    VkPhysicalDevice m_vk_physical_device;
  };
}
