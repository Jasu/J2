#pragma once

#include <vulkan/vulkan.h>
#include "attributes/keyed_attributes.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_layout.hpp"

namespace j::rendering::vulkan::descriptors {
  struct descriptor_binding_description {
    /// Description of the actual binding.
    VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    /// Which stages can access the binding.
    VkShaderStageFlags stages = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
    uniform_buffers::uniform_buffer_layout uniform_layout;
  };

  /// Describes a descriptor binding, along with the binding index.
  class bound_descriptor_binding_description final {
  public:
    /// Constructor called after attribute_definition when doing binding[12] = uniform(..)
    constexpr bound_descriptor_binding_description(
      const ::j::attributes::attribute_pair<u32_t, descriptor_binding_description> & pair
    ) noexcept : index(pair.key), binding(pair.value)
    { }

    /// Index of the binding, i.e. layout (binding=m_binding).
    u32_t index = U32_MAX;

    descriptor_binding_description binding;

    /// \note This constructor probably shouldn't be used, it exists only to make
    ///       trivial_array explicitly instantiate.
    constexpr bound_descriptor_binding_description() noexcept = default;
  };
}
