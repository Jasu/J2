#pragma once

#include "rendering/vulkan/surfaces/surface.hpp"
#include "rendering/vulkan/device_resource.hpp"
#include "rendering/vulkan/rendering/renderer.hpp"
#include "geometry/rect.hpp"

namespace j::windowing { struct window; }

namespace j::rendering::vulkan::surfaces {
  struct vulkan_window final {
    vulkan_window(mem::shared_ptr<const device_context_base> && context,
                  mem::shared_ptr<j::windowing::window> && window,
                  VkSurfaceKHR vk_surface) noexcept;

    void on_before_destroy_window();
    void on_show_window();
    void on_resize_window(const j::geometry::rect_i16 &);
    void on_button_press(u8_t button, j::geometry::vec2i16);

    void set_scene(mem::shared_ptr<j::rendering::vulkan::rendering::scene> scene);

    mem::shared_ptr<const device_context_base> device_context;
    mem::shared_ptr<j::windowing::window> window;
    struct surface surface;
    rendering::renderer renderer;
  };
}
