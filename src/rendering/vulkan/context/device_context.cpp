#include "rendering/vulkan/context/device_context.hpp"

#include "rendering/vulkan/shaders/shader_manager.hpp"
#include "rendering/vulkan/surfaces/window_factory.hpp"
#include "mem/shared_ptr.hpp"
#include "rendering/vulkan/instance.hpp"
#include "rendering/vulkan/debug/debug_utils.hpp"
#include "rendering/vulkan/transfer/transfer_source_buffer_set_description.hpp"

namespace j::rendering::vulkan::inline context {
  device_context::device_context(
    mem::shared_ptr<windowing::x11::context> windowing_context,
    mem::shared_ptr<class instance> instance,
    mem::shared_ptr<events::event_loop> event_loop,
    mem::shared_ptr<surfaces::window_factory> window_factory
  ) : m_device(create_tag),
      m_physical_device(create_tag),
      m_transfer_source_buffer_set(3, 3 * 1024 * 1024),
      m_default_sampler(create_tag),
      m_windowing_context(static_cast<mem::shared_ptr<windowing::x11::context> &&>(windowing_context)),
      m_instance(static_cast<mem::shared_ptr<class instance> &&>(instance)),
      m_event_loop(static_cast<mem::shared_ptr<events::event_loop> &&>(event_loop)),
      m_shader_manager(mem::make_shared<shaders::shader_manager>()),
      m_window_factory(static_cast<mem::shared_ptr<surfaces::window_factory> &&>(window_factory))
  {
    J_ASSERT_NOT_NULL(m_windowing_context, m_instance, m_event_loop, m_window_factory);
    if (m_instance->configuration().enable_debug) {
      m_debug_utils = mem::make_shared<class debug_utils>(m_instance);
    }
  }

  device_context::~device_context() {
  }

  const vulkan::device_ref & device_context::device_ref() const noexcept {
    return m_device;
  }

  const samplers::sampler_ref & device_context::default_sampler_ref() const noexcept {
    return m_default_sampler;
  }

  physical_devices::physical_device & device_context::physical_device() const noexcept {
    return get(m_physical_device);
  }

  transfer::transfer_source_buffer_set & device_context::transfer_source_buffer_set() const noexcept {
    return get(m_transfer_source_buffer_set);
  }

  windowing::x11::context & device_context::windowing_context() const noexcept {
    J_ASSERT_NOT_NULL(m_windowing_context);
    return *m_windowing_context;
  }

  vulkan::instance & device_context::instance() const noexcept {
    J_ASSERT_NOT_NULL(m_instance);
    return *m_instance;
  }

  events::event_loop & device_context::event_loop() const noexcept {
    J_ASSERT_NOT_NULL(m_event_loop);
    return *m_event_loop;
  }

  shaders::shader_manager & device_context::shader_manager() const noexcept {
    J_ASSERT_NOT_NULL(m_shader_manager);
    return *m_shader_manager;
  }

  class debug_utils * device_context::debug_utils() const noexcept {
    return m_debug_utils.get();
  }

  mem::shared_ptr<surfaces::vulkan_window> device_context::create_window(u16_t width, u16_t height) const {
    return m_window_factory->create_window(*this, width, height);
  }

  mem::shared_ptr<device_context> device_context::as_shared() noexcept {
    return shared_from_this();
  }
  mem::shared_ptr<const device_context> device_context::as_shared() const noexcept {
    return shared_from_this();
  }
}
