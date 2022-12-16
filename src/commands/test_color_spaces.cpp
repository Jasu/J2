#include "cli/cli.hpp"
#include "colors/lms.hpp"
#include "events/event_loop.hpp"
#include "fonts/font_set.hpp"
#include "fonts/selection/font_selector.hpp"
#include "fonts/selection/font_specification.hpp"
#include "geometry/vec4.hpp"
#include "input/key_event.hpp"
#include "mem/shared_ptr.hpp"
#include "rendering/images/image_format.hpp"
#include "rendering/images/static_image_source.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/images/image_description.hpp"
#include "rendering/vulkan/images/image_state.hpp"
#include "rendering/vulkan/rendering/scene.hpp"
#include "rendering/vulkan/context/device_context_factory.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_state.hpp"
#include "rendering/vulkan/command_buffers/draw_command.hpp"
#include "rendering/vulkan/surfaces/vulkan_window.hpp"
#include "rendering/vulkan/descriptors/descriptor_binding_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_state.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_description.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_description.hpp"
#include "rendering/vulkan/rendering/pipeline_description.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_description.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_state.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer.hpp"
#include "services/service_instance.hpp"
#include "strings/unicode/rope/rope_utf8.hpp"
#include "strings/format.hpp"
#include "ui/elements/text_view.hpp"
#include "ui/layout/linear_layout.hpp"
#include "ui/rendering/renderer.hpp"
#include "ui/rendering/context.hpp"
#include "windowing/window.hpp"

namespace j::commands {
  namespace {
    namespace g = j::geometry;
    namespace v = j::rendering::vulkan;
    namespace e = j::ui::elements;
    namespace a = j::ui::attributes;
    namespace va = j::rendering::vulkan::attributes;
    namespace sel = j::fonts::selection;

    g::vec4u8 color_to_vec(g::vec3d color, bool von_kries) noexcept {
      if (von_kries) {
        color = colors::lms_to_xyz_d65_von_kries(color);
      } else {
        color = colors::lms_to_xyz_d65_bradford(color);
      }
      auto lin = colors::xyz_d65_to_linear_rgb(color);
      return g::vec4u8(
        clamp(0, (int)(255 * lin.r), 255),
        clamp(0, (int)(255 * lin.g), 255),
        clamp(0, (int)(255 * lin.b), 255),
        255);
    }


    class color_space_slider_renderer final {
    public:
      color_space_slider_renderer()
        : m_uniform_buffer(va::record = v::uniform_buffer_record(
                             v::renderer_state_uniform_layout)),
          m_descriptor_set_layout(
            va::descriptor[0] = va::uniform(
              va::layout = v::renderer_state_uniform_layout,
              va::vertex_stage_only),
            va::descriptor[1] = va::combined_image_sampler(va::fragment_stage_only),
            va::descriptor[2] = va::combined_image_sampler(va::fragment_stage_only)
          ),
          m_descriptor_set(m_descriptor_set_layout),
          m_pipeline(
            va::name = "Color space slider pipeline",
            va::fragment_shader = "color_space_slider.frag.spv",
            va::vertex_shader = "color_space_slider.vert.spv",
            va::descriptor_set_layout = m_descriptor_set_layout,
            va::vertex_input = va::vertex_input_state{},
            va::push_constant[0] = va::vertex_shader_push_constant<g::vec4f>(),
            va::push_constant[16] = va::fragment_shader_push_constant<float>()
          ),
          m_image(va::image_1d, va::width = 256U, va::format = j::rendering::images::image_format::rgba32),
          m_image2(va::image_1d, va::width = 256U, va::format = j::rendering::images::image_format::rgba32),
          m_image_buffer_view(256U, j::rendering::images::image_format::rgba32, (u8_t*)&m_slider),
          m_image_buffer_view2(256U, j::rendering::images::image_format::rgba32, (u8_t*)&m_slider_with_others)
      {
      }

      void draw(v::command_buffers::command_buffer_builder & builder, g::vec2u16 position, g::vec2u16 size, float pos) const {
        if (!m_is_init) {
          m_is_init = true;
          m_uniform = builder.context.get(m_uniform_buffer).allocate();
        }
        g::vec4f pos2(position.x, position.y, size.x, size.y);
        builder.bind_pipeline(m_pipeline);
        builder.push_constant(0, pos2);
        builder.push_constant(16, pos);
        v::state::node_insertion_context c{builder.insertion_context()};
        c.begin_add_node();
        auto cmd = ::new v::command_buffers::draw_command(c, 4U, 1U, 0U, 0U);
        c.add_precondition(v::command_buffers::bound_pipeline == m_pipeline);
        c.add_precondition(v::descriptors::descriptor_sets[0] == m_descriptor_set);
        c.add_precondition(v::images::image_content(m_image) == m_image_buffer_view.get_key());
        c.add_precondition(v::images::image_content(m_image2) == m_image_buffer_view2.get_key());
        c.add_precondition(v::images::image_layout(m_image) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        c.add_precondition(v::images::image_layout(m_image2) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        c.add_precondition(v::uniform_contents(m_uniform) == v::renderer_state_uniform_key);
        c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 0) == m_uniform);
        c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 1) == m_image);
        c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 2) == m_image2);
        c.finish_add_node(cmd);
        builder.add_command(cmd);
      }

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        constexpr double f = 1.0 / 255.0;
        for (u32_t i = 0; i < 256; ++i) {
          g::vec3d res = (1.0 - f * (double)i) * a + f * (double)i * b;
          m_slider[i] = color_to_vec(res, von_kries);
          m_slider_with_others[i] = color_to_vec(res + others, von_kries);
        }
        m_image_buffer_view.invalidate();
        m_image_buffer_view2.invalidate();
      }

    private:
      v::uniform_buffer_ref m_uniform_buffer;
      v::descriptors::descriptor_set_layout_ref m_descriptor_set_layout;
      v::descriptors::descriptor_set_ref m_descriptor_set;
      v::rendering::pipeline_ref m_pipeline;
      mutable v::uniform_buffer_allocation m_uniform;
      v::images::image_ref m_image;
      v::images::image_ref m_image2;
      j::rendering::images::static_image_source m_image_buffer_view;
      j::rendering::images::static_image_source m_image_buffer_view2;
      mutable bool m_is_init = false;
      g::vec4u8 m_slider[256];
      g::vec4u8 m_slider_with_others[256];
    };

    class color_space_preview_renderer final {
    public:
      color_space_preview_renderer()
        : m_uniform_buffer(va::record = v::uniform_buffer_record(
                             v::renderer_state_uniform_layout)),
          m_descriptor_set_layout(
            va::descriptor[0] = va::uniform(
              va::layout = v::renderer_state_uniform_layout,
              va::vertex_stage_only),
            va::descriptor[1] = va::combined_image_sampler(va::fragment_stage_only)
          ),
          m_descriptor_set(m_descriptor_set_layout),
          m_pipeline(
            va::name = "Color space preview pipeline",
            va::fragment_shader = "color_space_preview.frag.spv",
            va::vertex_shader = "color_space_slider.vert.spv",
            va::descriptor_set_layout = m_descriptor_set_layout,
            va::vertex_input = va::vertex_input_state{},
            va::push_constant[0] = va::vertex_shader_push_constant<g::vec4f>(),
            va::push_constant[16] = va::fragment_shader_push_constant<g::vec4f>()
          ),
          m_image(va::image_1d, va::width = 256U, va::format = j::rendering::images::image_format::rgba32),
          m_image_buffer_view(256U, j::rendering::images::image_format::rgba32, (u8_t*)&m_slider)
      {
      }

      g::vec4u8 color_to_vec(g::vec3d color, bool von_kries) noexcept {
        if (von_kries) {
          color = colors::lms_to_xyz_d65_von_kries(color);
        } else {
          color = colors::lms_to_xyz_d65_bradford(color);
        }
        auto lin = colors::xyz_d65_to_linear_rgb(color);
        return g::vec4u8(
          clamp(0, (int)(255 * lin.r), 255),
          clamp(0, (int)(255 * lin.g), 255),
          clamp(0, (int)(255 * lin.b), 255),
          255);
      }

      void draw(v::command_buffers::command_buffer_builder & builder, g::vec2u16 position, g::vec2u16 size, g::vec4u8 color) const {
        if (!m_is_init) {
          m_is_init = true;
          m_uniform = builder.context.get(m_uniform_buffer).allocate();
        }
        g::vec4f pos2(position.x, position.y, size.x, size.y);
        float mul = 1.0f / 255.0f;
        g::vec4f color2(color.x * mul, color.y * mul, color.z * mul, 1.0f);
        builder.bind_pipeline(m_pipeline);
        builder.push_constant(0, pos2);
        builder.push_constant(16, color2);
        v::state::node_insertion_context c{builder.insertion_context()};
        c.begin_add_node();
        auto cmd = ::new v::command_buffers::draw_command(c, 4U, 1U, 0U, 0U);
        c.add_precondition(v::command_buffers::bound_pipeline == m_pipeline);
        c.add_precondition(v::descriptors::descriptor_sets[0] == m_descriptor_set);
        c.add_precondition(v::images::image_content(m_image) == m_image_buffer_view.get_key());
        c.add_precondition(v::images::image_layout(m_image) == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        c.add_precondition(v::uniform_contents(m_uniform) == v::renderer_state_uniform_key);
        c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 0) == m_uniform);
        c.add_precondition(v::descriptors::descriptor_binding(m_descriptor_set, 1) == m_image);
        c.finish_add_node(cmd);
        builder.add_command(cmd);
      }

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        constexpr double f = 1.0 / 255.0;
        for (u32_t i = 0; i < 256; ++i) {
          g::vec3d res = (1.0 - f * (double)i) * a + f * (double)i * b;
          m_slider[i] = color_to_vec(res + others, von_kries);
        }
        m_image_buffer_view.invalidate();
      }

    private:
      v::uniform_buffer_ref m_uniform_buffer;
      v::descriptors::descriptor_set_layout_ref m_descriptor_set_layout;
      v::descriptors::descriptor_set_ref m_descriptor_set;
      v::rendering::pipeline_ref m_pipeline;
      mutable v::uniform_buffer_allocation m_uniform;
      v::images::image_ref m_image;
      j::rendering::images::static_image_source m_image_buffer_view;
      mutable bool m_is_init = false;
      g::vec4u8 m_slider[256];
    };

    class color_space_slider final : public ui::static_element {
    public:

      using static_element::static_element;

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        m_renderer.set_colors(a, b, others, von_kries);
      }

      void render(ui::context & ctx, const g::rect_u16 & region) const override {
        m_frame.render(ctx, region, m_box.border, m_style);
        m_renderer.draw(*ctx.builder, {region.left(), region.top()}, {region.width(), region.height()}, m_editing_bg ? bg_value : fg_value);
      }

      void set_editing_bg(bool editing_bg) {
        m_editing_bg = editing_bg;
      }
      bool editing_bg() const noexcept {
        return m_editing_bg;
      }

      color_space_slider_renderer m_renderer;
      ui::element_frame m_frame;
      float fg_value = 0.5f;
      float bg_value = 0.5f;
      bool m_editing_bg = true;
    };

    class color_space_preview final : public ui::static_element {
    public:

      using static_element::static_element;

      void set_preview_color(g::vec3d color, bool von_kries) {
        m_preview_color = color_to_vec(color, von_kries);
      }

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        m_renderer.set_colors(a, b, others, von_kries);
      }

      void render(ui::context & ctx, const g::rect_u16 & region) const override {
        m_frame.render(ctx, region, m_box.border, m_style);
        m_renderer.draw(*ctx.builder, {region.left(), region.top()}, {region.width(), region.height()}, m_preview_color);
      }
    private:
      color_space_preview_renderer m_renderer;
      g::vec4u8 m_preview_color;
      ui::element_frame m_frame;
    };

    class color_space_slider_wrapper : public ui::linear_layout {
    public:
      color_space_slider_wrapper(const char * J_NOT_NULL format_string,
                                 const fonts::font_set & fonts) noexcept
        : ui::linear_layout(
            a::background_color = colors::rgba8(80, 24, 80, 255),
            a::border_size = g::perimeter_u8(4),
            a::margin = g::perimeter_u8(4),
            a::max_size = g::vec2u16(U16_MAX, 90U),
            a::orientation = ui::orientation::columns,
            a::border_color = colors::rgba8(185,65,185)
          ),
          m_format_string(format_string),
          m_text_view(mem::make_shared<e::text_view>(a::max_size = g::vec2u16(U16_MAX, 90U))),
          m_slider(mem::make_shared<color_space_slider>(a::max_size = g::vec2u16(U16_MAX, 90U)))
        {
          update_text();
          m_text_view->set_fonts(fonts);
          m_text_view->set_text(&m_text);
          push_back(m_text_view);
          push_back(m_slider);
        }

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        m_slider->set_colors(a, b, others, von_kries);
      }

      void set_bg_value(float value) {
        m_slider->bg_value = value;
        update_text();
      }

      void set_fg_value(float value) {
        m_slider->fg_value = value;
        update_text();
      }

      void update_text() {
        m_text = strings::rope_utf8(strings::format(m_format_string, m_slider->editing_bg() ? m_slider->bg_value : m_slider->fg_value));
      }

      J_INLINE_GETTER_NO_DEBUG float bg_value() const noexcept {
        return m_slider->bg_value;
      }
      J_INLINE_GETTER_NO_DEBUG float fg_value() const noexcept {
        return m_slider->fg_value;
      }
      void set_editing_bg(bool editing_bg) {
        m_slider->set_editing_bg(editing_bg);
        update_text();
      }
    private:
      const char * m_format_string;
      strings::rope_utf8 m_text;
      mem::shared_ptr<e::text_view> m_text_view;
      mem::shared_ptr<color_space_slider> m_slider;
    };

    class color_space_preview_wrapper : public ui::linear_layout {
    public:
      color_space_preview_wrapper(const char * J_NOT_NULL title,
                                  const fonts::font_set & fonts) noexcept
        : ui::linear_layout(
            a::background_color = colors::rgba8(45, 14, 40, 255),
            a::border_size = g::perimeter_u8(4),
            a::margin = g::perimeter_u8(4),
            a::border_color = colors::rgba8(185,65,185)
          ),
          m_text(title),
          m_text_view(mem::make_shared<e::text_view>(
                        a::max_size = g::vec2u16(U16_MAX, 90U),
                        a::margin = g::perimeter_u8(15))),
          m_preview(mem::make_shared<color_space_preview>(
                      a::max_size = g::vec2u16(U16_MAX, 600U),
                      a::margin = g::perimeter_u8(15)))
        {
          m_text_view->set_fonts(fonts);
          m_text_view->set_text(&m_text);
          push_back(m_text_view);
          push_back(m_preview);
        }

      void set_colors(g::vec3d a, g::vec3d b, g::vec3d others, bool von_kries) {
        m_preview->set_colors(a, b, others, von_kries);
      }
      void set_preview_color(g::vec3d color, bool von_kries) {
        m_preview->set_preview_color(color, von_kries);
      }
    private:
      strings::rope_utf8 m_text;
      mem::shared_ptr<e::text_view> m_text_view;
      mem::shared_ptr<color_space_preview> m_preview;
    };

    mem::shared_ptr<e::text_view> make_text_view(const fonts::font_set & fonts) {
      auto text_view = mem::make_shared<e::text_view>(
        a::background_color = colors::rgba8(80, 24, 80, 255),
        a::border_size = g::perimeter_u8(10),
        a::margin = g::perimeter_u8(10),
        a::max_size = g::vec2u16(U16_MAX, 90U),
        a::border_color = colors::rgba8(185,65,185));
      text_view->set_fonts(fonts);
      return text_view;
    }

    class J_TYPE_HIDDEN test_color_spaces_scene : public v::rendering::scene {
    public:
      test_color_spaces_scene(const mem::shared_ptr<ui::renderer> & renderer,
                              const fonts::font_set & fonts)
        : m_renderer(renderer),
          m_fonts(fonts),
          m_text_view_title(make_text_view(m_fonts)),
          m_slider_l(mem::make_shared<color_space_slider_wrapper>("L (Long) {}", fonts)),
          m_slider_m(mem::make_shared<color_space_slider_wrapper>("M (Medium) {}", fonts)),
          m_slider_s(mem::make_shared<color_space_slider_wrapper>("S (Short) {}", fonts)),
          m_preview_l(mem::make_shared<color_space_preview_wrapper>("Varying L (Long)", fonts)),
          m_preview_m(mem::make_shared<color_space_preview_wrapper>("Varying M (Medium)", fonts)),
          m_preview_s(mem::make_shared<color_space_preview_wrapper>("Varying S (Short)", fonts))
      {
        set_von_kries(false);

        m_text_view_title->set_text(&m_text_title);
        m_layout.push_back(m_text_view_title);
        m_layout.push_back(m_slider_l);
        m_layout.push_back(m_slider_m);
        m_layout.push_back(m_slider_s);
        m_layout.push_back(m_preview_l);
        m_layout.push_back(m_preview_m);
        m_layout.push_back(m_preview_s);
      }

      bool von_kries() const noexcept {
        return m_von_kries;
      }

      void set_von_kries(bool von_kries) {
        m_von_kries = von_kries;
        update_colors();
        update_title();
      }

      void update_title() {
        m_text_title = strings::rope_utf8(m_editing_bg ? "Editing BG - " : "Editing rectangle - ");
        m_text_title.append(m_von_kries ? "von Kries" : "Bradford");

      }

      bool editing_bg() const noexcept { return m_editing_bg; }

      void set_editing_bg(bool editing_bg) {
        m_editing_bg = editing_bg;
        m_slider_l->set_editing_bg(editing_bg);
        m_slider_m->set_editing_bg(editing_bg);
        m_slider_s->set_editing_bg(editing_bg);
        update_colors();
        update_title();
      }

      void update_colors() {
        m_slider_l->set_colors({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0f, m(), s() }, m_von_kries);
        m_slider_m->set_colors({ 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { l(), 0.0f, s() }, m_von_kries);
        m_slider_s->set_colors({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 }, { l(), m(), 0.0f }, m_von_kries);
        m_preview_l->set_colors({ 0.0, 0.0, 0.0 }, { 1.0, 0.0, 0.0 }, { 0.0f, bg_m(), bg_s() }, m_von_kries);
        m_preview_m->set_colors({ 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { bg_l(), 0.0f, bg_s() }, m_von_kries);
        m_preview_s->set_colors({ 0.0, 0.0, 0.0 }, { 0.0, 0.0, 1.0 }, { bg_l(), bg_m(), 0.0f }, m_von_kries);
        m_preview_l->set_preview_color({ fg_l(), fg_m(), fg_s() }, m_von_kries);
        m_preview_m->set_preview_color({ fg_l(), fg_m(), fg_s() }, m_von_kries);
        m_preview_s->set_preview_color({ fg_l(), fg_m(), fg_s() }, m_von_kries);
      }

      void set_l(float val) {
        m_editing_bg ? m_slider_l->set_bg_value(val) : m_slider_l->set_fg_value(val);
        update_colors();
      }

      void set_m(float val) {
        m_editing_bg ? m_slider_m->set_bg_value(val) : m_slider_m->set_fg_value(val);
        update_colors();
      }

      void set_s(float val) {
        m_editing_bg ? m_slider_s->set_bg_value(val) : m_slider_s->set_fg_value(val);
        update_colors();
      }

      float l() const noexcept { return m_editing_bg ? m_slider_l->bg_value() : m_slider_l->fg_value(); }
      float m() const noexcept { return m_editing_bg ? m_slider_m->bg_value() : m_slider_m->fg_value(); }
      float s() const noexcept { return m_editing_bg ? m_slider_s->bg_value() : m_slider_s->fg_value(); }

      float fg_l() const noexcept { return m_slider_l->fg_value(); }
      float fg_m() const noexcept { return m_slider_m->fg_value(); }
      float fg_s() const noexcept { return m_slider_s->fg_value(); }

      float bg_l() const noexcept { return m_slider_l->bg_value(); }
      float bg_m() const noexcept { return m_slider_m->bg_value(); }
      float bg_s() const noexcept { return m_slider_s->bg_value(); }

      void build_command_buffer(v::command_buffers::command_buffer_builder & b) override {
        m_renderer->render(b, m_layout);
      }
    private:
      mem::shared_ptr<ui::renderer> m_renderer;
      fonts::font_set m_fonts;
      mem::shared_ptr<e::text_view> m_text_view_title;
      mem::shared_ptr<color_space_slider_wrapper> m_slider_l;
      mem::shared_ptr<color_space_slider_wrapper> m_slider_m;
      mem::shared_ptr<color_space_slider_wrapper> m_slider_s;
      mem::shared_ptr<color_space_preview_wrapper> m_preview_l;
      mem::shared_ptr<color_space_preview_wrapper> m_preview_m;
      mem::shared_ptr<color_space_preview_wrapper> m_preview_s;
      ui::linear_layout m_layout;
      strings::rope_utf8 m_text_title;
      bool m_von_kries = false;
      bool m_editing_bg = true;
    };

    void test_color_spaces(mem::shared_ptr<v::device_context_factory> factory,
                           mem::shared_ptr<sel::font_selector> selector,
                           mem::shared_ptr<ui::renderer> r,
                           mem::shared_ptr<j::events::event_loop> event_loop)
    {
      sel::font_specification spec{
        sel::family = "sans-serif",
        sel::weight = sel::font_weight::regular,
        sel::slant = sel::font_slant::roman,
        sel::width = sel::font_width::normal,
        sel::spacing = sel::font_spacing::proportional,
        sel::size = fonts::font_size(fonts::pixel_size_tag, 48)
      };
      fonts::font_set fonts = selector->select_fonts(spec);
      auto ctx = factory->create();
      auto window = ctx->create_window(600, 600);

      auto scene = mem::make_shared<test_color_spaces_scene>(r, fonts);

      window->set_scene(scene);

      auto on_key_press = [&scene](const input::key_event & e) {
        if (e.utf8 == "w") {
          scene->set_l(min(1.0f, scene->l() + 0.05f));
        } else if (e.utf8 == "s") {
          scene->set_m(min(1.0f, scene->m() + 0.05f));
        } else if (e.utf8 == "x") {
          scene->set_s(min(1.0f, scene->s() + 0.05f));
        } else if (e.utf8 == "q") {
          scene->set_l(max(0.0f, scene->l() - 0.05f));
        } else if (e.utf8 == "a") {
          scene->set_m(max(0.0f, scene->m() - 0.05f));
        } else if (e.utf8 == "z") {
          scene->set_s(max(0.0f, scene->s() - 0.05f));
        } else if (e.utf8 == "k") {
          scene->set_von_kries(!scene->von_kries());
        } else if (e.utf8 == "b") {
          scene->set_editing_bg(!scene->editing_bg());
        } else {
          return;
        }
        scene->invalidate();
      };
      window->window->on_key_press.connect<&decltype(on_key_press)::operator()>(&on_key_press);
      event_loop->run();
    }

    namespace c = cli;
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_instance<cli::command> cmd(
      "commands.test_color_spaces",
      "Test color spaces command",

      c::command_name = "test-color-spaces",
      c::callback     = s::call(&test_color_spaces));
  }
}
