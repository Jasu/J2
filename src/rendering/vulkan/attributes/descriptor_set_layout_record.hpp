#pragma once

#include "util/tagged_union.hpp"
#include "attributes/attribute_definition.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"

namespace j::rendering::vulkan::attributes {
  namespace a = j::attributes;

  class descriptor_set_layout_record {
    util::tagged_union<descriptors::descriptor_set_layout_ref, descriptors::descriptor_set_ref> m_value;
  public:
    descriptor_set_layout_record() noexcept;
    descriptor_set_layout_record(descriptors::descriptor_set_layout_ref value);

    descriptor_set_layout_record(descriptors::descriptor_set_ref value);

    descriptor_set_layout_record(descriptor_set_layout_record && rhs) noexcept;
    descriptor_set_layout_record(const descriptor_set_layout_record & rhs);
    descriptor_set_layout_record & operator=(descriptor_set_layout_record && rhs) noexcept;
    descriptor_set_layout_record & operator=(const descriptor_set_layout_record & rhs);
    ~descriptor_set_layout_record();

    bool is_descriptor_set_layout() const noexcept;

    bool is_descriptor_set() const noexcept;

    descriptors::descriptor_set_layout_ref & as_descriptor_set_layout();

    const descriptors::descriptor_set_layout_ref & as_descriptor_set_layout() const;

    descriptors::descriptor_set_ref & as_descriptor_set();

    const descriptors::descriptor_set_ref & as_descriptor_set() const;
  };

  inline constexpr a::attribute_definition descriptor_set_layout{
    a::tag = type<struct descriptor_set_layout_tag>,
    a::value_type = type<descriptor_set_layout_record>,
    a::is_multiple};
}
