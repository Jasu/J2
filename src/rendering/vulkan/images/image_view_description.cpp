#include "rendering/vulkan/images/image_view_description.hpp"

namespace j::rendering::vulkan::images {
  image_view_description::image_view_description(image_ref image, bool retain) noexcept
    : m_data(static_cast<image_ref &&>(image)),
      retain(retain)
  {
  }

  image_view_description::image_view_description(VkImage vk_image, VkFormat format) noexcept
    : m_data(swapchain_image_rec{ vk_image, format })
  {
  }

  image_view_description::image_view_description(const image_view_description & rhs)
    : m_data(rhs.m_data),
      retain(rhs.retain)
  {
  }
  image_view_description::image_view_description(image_view_description && rhs) noexcept
    : m_data(static_cast<util::tagged_union<image_ref, swapchain_image_rec> &&>(rhs.m_data)),
      retain(rhs.retain)
  {
  }

  image_view_description & image_view_description::operator=(const image_view_description & rhs) {
    m_data = rhs.m_data;
    retain = rhs.retain;
    return *this;
  }

  image_view_description & image_view_description::operator=(image_view_description && rhs) noexcept {
    m_data = static_cast<util::tagged_union<image_ref, swapchain_image_rec> &&>(rhs.m_data);
    retain = rhs.retain;
    return *this;
  }

  image_view_description::~image_view_description() {
  }

  bool image_view_description::is_image() const noexcept{
    return m_data.is<image_ref>();
  }

  bool image_view_description::is_swapchain_image() const noexcept{
    return m_data.is<swapchain_image_rec>();
  }

  const image_ref & image_view_description::as_image() const {
    return m_data.as<image_ref>();
  }

  image_ref & image_view_description::as_image() {
    return m_data.as<image_ref>();
  }

  const swapchain_image_rec & image_view_description::as_swapchain_image() const {
    return m_data.as<swapchain_image_rec>();
  }
}
