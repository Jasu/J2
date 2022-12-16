#include "rendering/vulkan/context/surface_context.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "rendering/vulkan/surfaces/surface.hpp"

namespace j::rendering::vulkan::inline context {
  surface_context::surface_context(const mem::shared_ptr<const device_context_base> & device_context,
                                    surfaces::surface & surf) noexcept
    : surface(&surf),
      m_device_context(device_context)
  {
    J_ASSERT_NOT_NULL(m_device_context);
  }

  surface_context::~surface_context() {
  }
  mem::shared_ptr<surfaces::vulkan_window> surface_context::create_window(u16_t width, u16_t height) const {
    return m_device_context->create_window(width, height);
  }

  rendering::render_pass_ref surface_context::render_pass_ref() const noexcept {
    return surface->render_pass;
  }

  const rendering::render_pass & surface_context::render_pass() const {
    return get(surface->render_pass);
  }

  vulkan::instance & surface_context::instance() const noexcept {
    return m_device_context->instance();
  }

  const vulkan::device_ref & surface_context::device_ref() const noexcept {
    return m_device_context->device_ref();
  }

  const samplers::sampler_ref & surface_context::default_sampler_ref() const noexcept {
    return m_device_context->default_sampler_ref();
  }

  physical_devices::physical_device & surface_context::physical_device() const noexcept {
    return m_device_context->physical_device();
  }

  transfer::transfer_source_buffer_set & surface_context::transfer_source_buffer_set() const noexcept {
    return m_device_context->transfer_source_buffer_set();
  }

  windowing::x11::context & surface_context::windowing_context() const noexcept {
    return m_device_context->windowing_context();
  }

  events::event_loop & surface_context::event_loop() const noexcept {
    return m_device_context->event_loop();
  }

  shaders::shader_manager & surface_context::shader_manager() const noexcept {
    return m_device_context->shader_manager();
  }

  class debug_utils * surface_context::debug_utils() const noexcept {
    return m_device_context->debug_utils();
  }
}
