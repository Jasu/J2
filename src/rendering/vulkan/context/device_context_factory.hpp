#pragma once

#include "rendering/vulkan/context/device_context.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_factory final {
  public:
    device_context_factory(
      mem::shared_ptr<windowing::x11::context> windowing_context,
      mem::shared_ptr<instance> instance,
      mem::shared_ptr<events::event_loop> event_loop,
      mem::shared_ptr<surfaces::window_factory> window_factory);

    mem::shared_ptr<device_context> create();
  private:
    mem::shared_ptr<windowing::x11::context> m_windowing_context;
    mem::shared_ptr<instance> m_instance;
    mem::shared_ptr<events::event_loop> m_event_loop;
    mem::shared_ptr<surfaces::window_factory> m_window_factory;
  };
}
