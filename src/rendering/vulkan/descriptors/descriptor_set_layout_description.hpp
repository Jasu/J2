#pragma once

#include "rendering/vulkan/attributes/descriptor_attributes.hpp"
#include "attributes/enable_if_attributes.hpp"
#include "containers/trivial_array_fwd.hpp"

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::descriptors::bound_descriptor_binding_description);

namespace j::rendering::vulkan::descriptors {
  struct descriptor_set_layout_description final {
    constexpr descriptor_set_layout_description() noexcept = default;

    template<typename... Args, typename = ::j::attributes::enable_if_attributes_t<Args...>>
    explicit descriptor_set_layout_description(Args && ... args)
      // Hack that works here, since all args have the same type.
      : bindings(containers::move, { args.m_value... })
    {
    }
    trivial_array<bound_descriptor_binding_description> bindings;
  };
}
