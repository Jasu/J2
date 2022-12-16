#include "rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_view.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_description.hpp"
#include "rendering/vulkan/context/render_context.hpp"
#include "rendering/vulkan/rendering/framebuffer.hpp"
#include "strings/string.hpp"
#include "geometry/vec2.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    namespace g = geometry;
    namespace a = j::rendering::vulkan::attributes;
    namespace {
      struct J_TYPE_HIDDEN renderer_state_uniform final {
        g::vec2f window_size_px_inv;
        g::vec2f window_size_px;
      };
    }
    const uniform_buffer_layout renderer_state_uniform_layout = make_uniform_layout(
      a::member = &renderer_state_uniform::window_size_px_inv,
      a::member = &renderer_state_uniform::window_size_px
    );

    renderer_state_uniform_handler::renderer_state_uniform_handler() noexcept
      : uniform_data_source_handler(0U) {
    }


    void renderer_state_uniform_handler::copy_to(const vulkan::render_context & context,
                                                 uniform_view & to,
                                                 [[maybe_unused]] uniform_data_source_key source) const
    {
      J_ASSERT_NOT_NULL(to);
      J_ASSERT(source == renderer_state_uniform_key, "Unknown source.");
      const VkExtent2D & extent = context.framebuffer->extent;
      *reinterpret_cast<renderer_state_uniform*>(to.data) = {
        g::vec2f(extent.width ? 1.0f / extent.width : 0.0f, extent.height ? 1.0f / extent.height : 0.0f),
        g::vec2f(extent.width, extent.height),
      };
    }

    strings::string renderer_state_uniform_handler::describe([[maybe_unused]] uniform_data_source_key source) const {
      J_ASSERT(source == renderer_state_uniform_key, "Unknown source.");
      return "Renderer state";
    }

    uniform_buffer_layout renderer_state_uniform_handler::get_info([[maybe_unused]] uniform_data_source_key source) const {
      J_ASSERT(source == renderer_state_uniform_key, "Unknown source.");
      return renderer_state_uniform_layout;
    }

    u64_t renderer_state_uniform_handler::get_userdata(uniform_data_source_key) const {
      return 0ULL;
    }

    renderer_state_uniform_handler renderer_state_uniform_handler::instance{};
  }
}
