#include "rendering/vulkan/surfaces/vulkan_window.hpp"
#include "rendering/vulkan/context/device_context_base.hpp"
#include "rendering/vulkan/instance.hpp"
#include "windowing/window.hpp"

namespace j::rendering::vulkan::surfaces {
  vulkan_window::vulkan_window(mem::shared_ptr<const device_context_base> && context,
                               mem::shared_ptr<j::windowing::window> && window,
                               VkSurfaceKHR vk_surface) noexcept
    : device_context(static_cast<mem::shared_ptr<const device_context_base> &&>(context)),
      window(static_cast<mem::shared_ptr<j::windowing::window> &&>(window)),
      surface(*device_context, vk_surface),
      renderer(device_context, surface)
  {
    J_ASSERT_NOT_NULL(this->window, device_context, vk_surface);
    this->window->on_before_destroy.connect<&vulkan_window::on_before_destroy_window>(this);
    this->window->on_show.connect<&vulkan_window::on_show_window>(this);
    this->window->on_resize.connect<&vulkan_window::on_resize_window>(this);
    this->window->on_button_press.connect<&vulkan_window::on_button_press>(this);
  }

  void vulkan_window::on_before_destroy_window() {
    renderer.finalize();
    surface.finalize(device_context->device());
    device_context.reset();
  }

  void vulkan_window::on_show_window() {
    renderer.invalidate();
  }

  void vulkan_window::on_resize_window(const j::geometry::rect_i16 &) {
    renderer.invalidate();
  }

  void vulkan_window::on_button_press(u8_t button, j::geometry::vec2i16) {
    if (button == 3 && device_context->instance().configuration().dump_graphviz_on_right_click) {
      renderer.queue_graphviz_dump();
    }
  }

  void vulkan_window::set_scene(mem::shared_ptr<j::rendering::vulkan::rendering::scene> scene) {
    renderer.set_scene(static_cast<mem::shared_ptr<j::rendering::vulkan::rendering::scene> &&>(scene));
  }
}
