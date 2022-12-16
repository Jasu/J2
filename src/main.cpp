#include "application/information.hpp"
#include "cli/cli.hpp"
#include "backtrace/dump_backtrace.hpp"
#include "cli/application.hpp"
#include "events/event_loop.hpp"
#include "fonts/font_set.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "input/key_event.hpp"
#include "logging/global.hpp"
#include "rendering/renderers/box/box_renderer.hpp"
#include "rendering/renderers/box/box.hpp"
#include "rendering/renderers/text/text_renderer.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/rendering/scene.hpp"
#include "rendering/vulkan/surfaces/vulkan_window.hpp"
#include "rendering/vulkan/context/device_context_factory.hpp"
#include "services/services.hpp"
#include "strings/string.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"
#include "windowing/window.hpp"

J_APPLICATION_INFORMATION(
  .application_name = "j",
  .major_version = 1,
  .minor_version = 0,
  .patch_version = 0,
  .configuration_files{{"j.yaml"}},
)

namespace {
  namespace r = j::rendering::renderers;
  namespace v = j::rendering::vulkan;
  namespace g = j::geometry;
  namespace f = j::fonts;
  namespace c = j::colors;
  namespace s = j::strings;
  class sample_scene : public v::rendering::scene {
    bool m_x = false;
    j::mem::shared_ptr<r::text::text_renderer> m_text_renderer;
    j::mem::shared_ptr<r::box::box_renderer> m_box_renderer;
    f::font_set m_fonts;
    f::font_set m_fonts2;
    j::mem::shared_ptr<r::text::text_renderer_string> m_str;
    r::box::box m_box;
  public:
    sample_scene(
      j::mem::shared_ptr<j::rendering::renderers::text::text_renderer> r,
      j::mem::shared_ptr<j::rendering::renderers::box::box_renderer> br,
      f::font_set && fonts,
      f::font_set && fonts2
    ) : m_text_renderer(static_cast<j::mem::shared_ptr<j::rendering::renderers::text::text_renderer> &&>(r)),
        m_box_renderer(static_cast<j::mem::shared_ptr<j::rendering::renderers::box::box_renderer> &&>(br)),
        m_fonts(fonts),
        m_fonts2(fonts2),
        m_str(m_text_renderer->allocate_string(s::rope_utf8{"Hello, world!"}, g::vec2f{12,123}, m_fonts, c::rgba8(129,234,210))),
        m_box{
          g::rect_i16(70, 90, 100, 200),
          c::rgba8(255,100,80),
          c::rgba8(5,199,200),
          c::rgba8(255,255,0),
          c::rgba8(255,0,255),
          c::rgba8(0,255,255),
          g::perimeter_u8(10u, 15u, 0u, 8u),
          g::perimeter_u8(10u, 2u, 20u, 30u)}
    {
    }

    ~sample_scene() {
      J_DEBUG("Destroying sample scene.");
    }

    void build_command_buffer(v::command_buffers::command_buffer_builder & b) override {
      b.set_viewport(g::rect_u16{0, 0, U16_MAX, U16_MAX});
      m_box_renderer->draw_box(b, m_box);
      if (m_x) {
        m_text_renderer->draw_string(b, m_str);
      } else {
        auto str2 = m_text_renderer->allocate_string(s::rope_utf8{"Asdf"}, g::vec2f{42,223}, m_fonts2, c::rgba8(244,255,10));
        m_text_renderer->draw_string(b, str2);
      }
    }

    void flip_buf() noexcept { m_x = !m_x, invalidate(); }
};

int vulkan_test(j::mem::shared_ptr<v::device_context_factory> factory,
                j::mem::shared_ptr<r::text::text_renderer> text_renderer,
                j::mem::shared_ptr<r::box::box_renderer> box_renderer,
                j::mem::shared_ptr<f::selection::font_selector> selector,
                j::mem::shared_ptr<j::events::event_loop> event_loop)
{
  auto ctx = factory->create();
  auto window = ctx->create_window(600, 600);

  auto scene = j::mem::make_shared<sample_scene>(
    static_cast<j::mem::shared_ptr<r::text::text_renderer> &&>(text_renderer),
    static_cast<j::mem::shared_ptr<r::box::box_renderer> &&>(box_renderer),
    selector->select_fonts(
      f::selection::family = "sans-serif",
      f::selection::size = j::fonts::font_size(f::pixel_size_tag, 30)
    ),
    selector->select_fonts(
      f::selection::family = "URW Gothic",
      f::selection::slant = f::selection::font_slant::italic,
      f::selection::size = f::font_size(f::pixel_size_tag, 30)
    ));

  window->set_scene(scene);

  auto on_button = [&scene](u8_t, j::geometry::vec2i16) { scene->flip_buf(); };
  auto on_key_press = [](const j::input::key_event & e) {
    J_DEBUG("Key press {}", e);
  };
  auto on_key_release = [](const j::input::key_event & e) {
    J_DEBUG("Key release {}", e);
  };
  auto w = window->window;
  w->on_button_press.connect<&decltype(on_button)::operator()>(&on_button);
  w->on_key_press.connect<&decltype(on_key_press)::operator()>(&on_key_press);
  w->on_key_release.connect<&decltype(on_key_release)::operator()>(&on_key_release);

  event_loop->run();

  return 0;
}

using namespace j::cli;
using namespace j::services::arg;
auto subcmd = service_instance<command>(
  "cli.command.vulkan-test",
  "Vulkan test command",
  command_name = "vulkan-test",
  callback = call(&vulkan_test, autowire, autowire, autowire, autowire, autowire));
}

int main(int argc, const char ** argv) {
  using namespace j;
  services::service_container->configure_service("cli.app", "command_line_arguments", span(argv, argc));
  services::service_container->initialize();
  services::service_container->get<j::cli::application>()->execute();
  services::service_container->finalize();
  return 0;
}
