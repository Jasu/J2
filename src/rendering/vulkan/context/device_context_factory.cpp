#include "rendering/vulkan/context/device_context_factory.hpp"

#include "services/service_definition.hpp"
#include "windowing/x11/context.hpp"
#include "rendering/vulkan/instance.hpp"
#include "events/event_loop.hpp"
#include "rendering/vertex_data/vertex_data_source_manager.hpp"
#include "rendering/vulkan/surfaces/window_factory.hpp"

namespace j::rendering::vulkan::inline context {
  device_context_factory::device_context_factory(
    mem::shared_ptr<windowing::x11::context> windowing_context,
    mem::shared_ptr<instance> instance,
    mem::shared_ptr<events::event_loop> event_loop,
    mem::shared_ptr<surfaces::window_factory> window_factory
  ) : m_windowing_context(static_cast<mem::shared_ptr<windowing::x11::context> &&>(windowing_context)),
      m_instance(static_cast<mem::shared_ptr<vulkan::instance> &&>(instance)),
      m_event_loop(static_cast<mem::shared_ptr<events::event_loop> &&>(event_loop)),
      m_window_factory(static_cast<mem::shared_ptr<surfaces::window_factory> &&>(window_factory))
  {
    J_ASSERT_NOT_NULL(m_windowing_context, m_instance, m_event_loop, m_window_factory);
  }

  mem::shared_ptr<device_context> device_context_factory::create() {
    return mem::make_shared<device_context>(m_windowing_context,
                                            m_instance,
                                            m_event_loop,
                                            m_window_factory);
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<device_context_factory> def(
      "vulkan.device_context_factory",
      "Vulkan device context factory",
      s::global_singleton,
      s::create = s::constructor<
        mem::shared_ptr<windowing::x11::context>,
        mem::shared_ptr<instance>,
        mem::shared_ptr<events::event_loop>,
        mem::shared_ptr<surfaces::window_factory>
      >()
    );
  }
}
