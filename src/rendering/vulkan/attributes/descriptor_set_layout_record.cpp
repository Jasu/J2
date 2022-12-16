#include "rendering/vulkan/attributes/descriptor_set_layout_record.hpp"

namespace j::rendering::vulkan::attributes {
  descriptor_set_layout_record::descriptor_set_layout_record() noexcept {
  }

  descriptor_set_layout_record::descriptor_set_layout_record(descriptors::descriptor_set_layout_ref value)
    : m_value(static_cast<descriptors::descriptor_set_layout_ref &&>(value))
  {
  }

  descriptor_set_layout_record::descriptor_set_layout_record(descriptors::descriptor_set_ref value)
    : m_value(static_cast<descriptors::descriptor_set_ref &&>(value))
  {
  }

  descriptor_set_layout_record::descriptor_set_layout_record(descriptor_set_layout_record && rhs) noexcept
    : m_value(static_cast<util::tagged_union<descriptors::descriptor_set_layout_ref, descriptors::descriptor_set_ref> &&>(rhs.m_value))
  {
  }

  descriptor_set_layout_record::descriptor_set_layout_record(const descriptor_set_layout_record & rhs)
    : m_value(rhs.m_value)
  {
  }

  descriptor_set_layout_record & descriptor_set_layout_record::operator=(descriptor_set_layout_record && rhs) noexcept {
    if (this != &rhs) {
      m_value = static_cast<util::tagged_union<descriptors::descriptor_set_layout_ref, descriptors::descriptor_set_ref> &&>(rhs.m_value);
    }
    return *this;
  }

  descriptor_set_layout_record & descriptor_set_layout_record::operator=(const descriptor_set_layout_record & rhs) {
    if (this != &rhs) {
      m_value = rhs.m_value;
    }
    return *this;
  }

  descriptor_set_layout_record::~descriptor_set_layout_record() {
  }


  bool descriptor_set_layout_record::is_descriptor_set_layout() const noexcept {
    return m_value.is<descriptors::descriptor_set_layout_ref>();
  }


  bool descriptor_set_layout_record::is_descriptor_set() const noexcept {
    return m_value.is<descriptors::descriptor_set_ref>();
  }

  descriptors::descriptor_set_layout_ref & descriptor_set_layout_record::as_descriptor_set_layout() {
    return m_value.as<descriptors::descriptor_set_layout_ref>();
  }

  const descriptors::descriptor_set_layout_ref & descriptor_set_layout_record::as_descriptor_set_layout() const {
    return m_value.as<descriptors::descriptor_set_layout_ref>();
  }

  descriptors::descriptor_set_ref & descriptor_set_layout_record::as_descriptor_set() {
    return m_value.as<descriptors::descriptor_set_ref>();
  }

  const descriptors::descriptor_set_ref & descriptor_set_layout_record::as_descriptor_set() const {
    return m_value.as<descriptors::descriptor_set_ref>();
  }
}
