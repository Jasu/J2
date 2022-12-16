#pragma once

#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/attributes/descriptor_attributes.hpp"
#include "attributes/enable_if_attributes.hpp"
#include "attributes/basic_operations.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "exceptions/assert_lite.hpp"

namespace j::rendering::vulkan::descriptors {
  struct layout_count_rec final {
    descriptors::descriptor_set_layout_ref layout;
    u32_t count = 0U;

    constexpr layout_count_rec() noexcept = default;

    layout_count_rec(descriptors::descriptor_set_layout_ref layout, u32_t count = 1)
      : layout(static_cast<descriptors::descriptor_set_layout_ref &&>(layout)),
        count(count)
    { J_ASSERT_NOT_NULL(this->layout, count); }
  };
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::descriptors::layout_count_rec);

namespace j::rendering::vulkan::descriptors {
  struct descriptor_pool_description final {
    J_PACKED trivial_array<layout_count_rec> layouts;
    u32_t num_uniforms = 0;
    u32_t num_combined_image_samplers = 0;
    u32_t max_sets = 0;

    template<typename... Args, typename = j::attributes::enable_if_attributes_t<Args...>>
    descriptor_pool_description(Args && ... args) {
      constexpr u32_t num_layout = ::j::attributes::count<Args...>(attributes::layout);
      constexpr u32_t num_layout_count = ::j::attributes::count<Args...>(attributes::layout_count);
      if constexpr (num_layout > 0 || num_layout_count > 0) {
        layouts = trivial_array<layout_count_rec>(
          containers::uninitialized,
          num_layout + num_layout_count);

        if constexpr (num_layout > 0) {
          attributes::layout.apply([&](auto && ... ref) {
            (layouts.initialize_element(static_cast<descriptor_set_layout_ref &&>(ref)), ...);
          }, static_cast<Args &&>(args)...);
        }
        if constexpr (num_layout_count > 0) {
          attributes::layout_count.apply([&](auto && ... rec) {
            (layouts.initialize_element(static_cast<layout_count_rec &&>(rec)), ...);
          }, static_cast<Args &&>(args)...);
        }
      }
      if constexpr (::j::attributes::has<Args...>(attributes::num_uniforms))  {
        num_uniforms = attributes::num_uniforms.get(static_cast<Args &&>(args)...);
      }
      if constexpr (::j::attributes::has<Args...>(attributes::num_combined_image_samplers))  {
        num_combined_image_samplers = attributes::num_combined_image_samplers.get(static_cast<Args &&>(args)...);
      }
      if constexpr (::j::attributes::has<Args...>(attributes::max_sets))  {
        max_sets = attributes::max_sets.get(static_cast<Args &&>(args)...);
      }
    }
  };
}
