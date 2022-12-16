#include "ui/rendering/renderer.hpp"

#include "ui/rendering/context.hpp"
#include "ui/element.hpp"
#include "ui/text/paragraph_layout_engine.hpp"
#include "services/service_definition.hpp"
#include "rendering/renderers/box/box_renderer.hpp"
#include "rendering/renderers/text/text_renderer.hpp"
#include "rendering/vulkan/command_buffers/command_buffer_builder.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"

namespace j::ui::inline rendering {
  renderer::renderer(mem::shared_ptr<r::box::box_renderer> && box_renderer,
                     mem::shared_ptr<r::text::text_renderer> && text_renderer,
                     mem::shared_ptr<paragraph_layout_engine> && ple)
    : m_box_renderer(static_cast<mem::shared_ptr<r::box::box_renderer> &&>(box_renderer)),
      m_text_renderer(static_cast<mem::shared_ptr<r::text::text_renderer> &&>(text_renderer)),
      m_paragraph_layout_engine(static_cast<mem::shared_ptr<paragraph_layout_engine> &&>(ple))
  { J_ASSERT_NOT_NULL(m_box_renderer, m_text_renderer, m_paragraph_layout_engine); }

  void renderer::render(v::command_buffer_builder & builder,
                        const element & element,
                        const g::rect_u16 & region) {
    context ctx{
      &builder,
      m_box_renderer.get(),
      m_text_renderer.get(),
      m_paragraph_layout_engine.get()
    };
    element.render(ctx, region);
  }


  void renderer::render(v::command_buffer_builder & builder,
                        const element & element) {
    context ctx{
      &builder,
      m_box_renderer.get(),
      m_text_renderer.get(),
      m_paragraph_layout_engine.get()
    };
    const auto & ext = builder.context.framebuffer->extent;
    element.render(ctx, g::rect_u16(0U, 0U, ext.width, ext.height));
  }
  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<renderer> g_renderer_def(
      "ui.renderer",
      "UI Renderer",
      s::create = s::constructor<
        mem::shared_ptr<r::box::box_renderer> &&,
        mem::shared_ptr<r::text::text_renderer> &&,
        mem::shared_ptr<paragraph_layout_engine> &&
      >()
    );
  }
}
