#pragma once

#include "rendering/vulkan/context/device_context_base.hpp"

#include "mem/shared_ptr.hpp"
#include "mem/shared_from_this.hpp"

namespace j::rendering::vulkan::surfaces {
  class window_factory;
}

namespace j::rendering::vulkan::inline context {
  class device_context final : public device_context_base,
                               public mem::enable_shared_from_this<device_context>
  {
  public:
    device_context(mem::shared_ptr<windowing::x11::context> windowing_context,
                   mem::shared_ptr<vulkan::instance> instance,
                   mem::shared_ptr<events::event_loop> event_loop,
                   mem::shared_ptr<surfaces::window_factory> window_factory);

    ~device_context();

    const vulkan::device_ref & device_ref() const noexcept override;

    const samplers::sampler_ref & default_sampler_ref() const noexcept override;

    physical_devices::physical_device & physical_device() const noexcept override;

    transfer::transfer_source_buffer_set & transfer_source_buffer_set() const noexcept override;

    shaders::shader_manager & shader_manager() const noexcept override;

    windowing::x11::context & windowing_context() const noexcept override;

    vulkan::instance & instance() const noexcept override;

    events::event_loop & event_loop() const noexcept override;

    mem::shared_ptr<surfaces::vulkan_window> create_window(u16_t width, u16_t height) const override;


    class debug_utils * debug_utils() const noexcept override;

    mem::shared_ptr<const device_context> as_shared() const noexcept;

    mem::shared_ptr<device_context> as_shared() noexcept;
  private:
    vulkan::device_ref m_device;
    physical_devices::physical_device_ref m_physical_device;
    transfer::transfer_source_buffer_set_ref m_transfer_source_buffer_set;
    samplers::sampler_ref m_default_sampler;

    mem::shared_ptr<windowing::x11::context> m_windowing_context;
    mem::shared_ptr<vulkan::instance> m_instance;
    mem::shared_ptr<events::event_loop> m_event_loop;
    mem::shared_ptr<shaders::shader_manager> m_shader_manager;
    mem::shared_ptr<surfaces::window_factory> m_window_factory;
    mem::shared_ptr<class debug_utils> m_debug_utils;
  };
}
