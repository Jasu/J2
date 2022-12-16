#include "ui/elements/tabula_rasa.hpp"
#include "ui/elements/text_view.hpp"
#include "ui/layout/linear_layout.hpp"
#include "ui/rendering/renderer.hpp"
#include "cli/cli.hpp"
#include "fonts/font_set.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "mem/shared_ptr.hpp"
#include "rendering/vulkan/rendering/scene.hpp"
#include "services/service_instance.hpp"
#include "windowing/window.hpp"
#include "events/event_loop.hpp"
#include "rendering/vulkan/context/device_context_factory.hpp"
#include "rendering/vulkan/surfaces/vulkan_window.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"

namespace j::ui::utils {
  namespace {
    namespace g = j::geometry;
    namespace v = j::rendering::vulkan;
    namespace e = j::ui::elements;
    namespace a = j::ui::attributes;
    namespace sel = j::fonts::selection;

    class J_TYPE_HIDDEN test_scene : public v::rendering::scene {
    public:
      test_scene(mem::shared_ptr<renderer> && renderer, const fonts::font_set & fonts, strings::rope_utf8 * text)
        : m_renderer(static_cast<mem::shared_ptr<class renderer> &&>(renderer)),
          m_fonts(fonts),
          m_text(text)
      {
        J_ASSERT_NOT_NULL(m_renderer);
        auto text_view = mem::make_shared<e::text_view>(
                             a::background_color = colors::rgba8(255, 128, 0, 255),
                             a::border_size = g::perimeter_u8(10),
                             a::border_color = colors::rgba8(50,100,200));
        text_view->set_text(m_text);
        text_view->set_fonts(m_fonts);
        m_layout.push_back(text_view);
        m_layout.push_back(mem::make_shared<e::tabula_rasa>(
                             a::background_color = colors::rgba8(0, 128, 255, 255),
                             a::border_size = g::perimeter_u8(20),
                             a::corner_radius = g::perimeter_u8(10,20,30,40),
                             a::border_color = colors::rgba8(50,100,200),
                             a::margin = g::perimeter_u8(10U)));
      }

      void build_command_buffer(v::command_buffers::command_buffer_builder & b) override {
        m_renderer->render(b, m_layout);
      }
    private:
      linear_layout m_layout;
      mem::shared_ptr<renderer> m_renderer;
      fonts::font_set m_fonts;
      strings::rope_utf8 * m_text;
    };

    void test_rendering(mem::shared_ptr<v::device_context_factory> factory,
                        mem::shared_ptr<sel::font_selector> selector,
                        mem::shared_ptr<renderer> r,
                        mem::shared_ptr<j::events::event_loop> event_loop)
    {
      J_ASSERT_NOT_NULL(factory, selector, r, event_loop);

      sel::font_specification spec{
        sel::family = "sans-serif",
        sel::weight = sel::font_weight::regular,
        sel::slant = sel::font_slant::roman,
        sel::width = sel::font_width::normal,
        sel::spacing = sel::font_spacing::proportional,
        sel::size = fonts::font_size(fonts::pixel_size_tag, 48)
      };
      fonts::font_set fonts = selector->select_fonts(spec);
      strings::rope_utf8 text("Foo Bar, Baz Lorem Ipsum dolor sit Amet consectuter yada yada asfd gölgöl.\nTesti test12");

      auto ctx = factory->create();
      auto window = ctx->create_window(600, 600);

      auto scene = mem::make_shared<test_scene>(static_cast<mem::shared_ptr<renderer> &&>(r), fonts, &text);

      window->set_scene(scene);

      auto on_button = [&scene](u8_t, g::vec2i16) { scene->invalidate(); };
      window->window->on_button_press.connect<&decltype(on_button)::operator()>(&on_button);
      event_loop->run();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "ui.utils.test_rendering",
      "UI rendering test command",

      c::command_name = "test-ui",
      c::callback     = s::call(&test_rendering));
  }
}
