#include "rendering/vulkan/x11/window_factory.hpp"
#include "rendering/vulkan/x11/surface_factory.hpp"
#include "rendering/vulkan/surfaces/vulkan_window.hpp"
#include "rendering/vulkan/context/device_context.hpp"
#include "mem/shared_ptr.hpp"
#include "services/service_definition.hpp"
#include "windowing/x11/window.hpp"
#include "windowing/x11/context.hpp"

namespace j::rendering::vulkan::x11 {
  window_factory::window_factory(mem::shared_ptr<surface_factory> && factory)
    : m_surface_factory(static_cast<mem::shared_ptr<surface_factory> &&>(factory))
  { J_ASSERT_NOT_NULL(m_surface_factory); }

  mem::shared_ptr<surfaces::vulkan_window> window_factory::create_window(
    const device_context_base & context, u16_t width, u16_t height) const
  {
    auto window = context.windowing_context().create_window(width, height);

    auto surface = m_surface_factory->create_surface(context, *window);
    auto result = mem::make_shared<surfaces::vulkan_window>(
      static_cast<const device_context &>(context).as_shared(),
      static_cast<mem::shared_ptr<windowing::window> &&>(window),
      surface);
    result->renderer.on_after_render.connect<&windowing::x11::context::handle_queued_events>(
        &context.windowing_context());
    window->show();
    context.windowing_context().flush();
    return result;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) s::service_definition<window_factory> def(
      "vulkan.x11.window_factory",
      "X11 Window Factory",
      s::implements = s::interface<surfaces::window_factory>,
      s::create = s::constructor<mem::shared_ptr<surface_factory> &&>()
    );
  }
}
