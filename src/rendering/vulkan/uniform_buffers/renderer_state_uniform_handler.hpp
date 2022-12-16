#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_data_source_handler.hpp"

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    extern const uniform_buffer_layout renderer_state_uniform_layout;

    inline constexpr uniform_data_source_key renderer_state_uniform_key(0, 1);

    class renderer_state_uniform_handler final : public uniform_data_source_handler {
    private:
      renderer_state_uniform_handler() noexcept;
    public:
      static renderer_state_uniform_handler instance;

      void copy_to(const vulkan::render_context &,
                  uniform_view & to, uniform_data_source_key source) const override;
      strings::string describe(uniform_data_source_key source) const override;
      uniform_buffer_layout get_info(uniform_data_source_key source) const override;
      u64_t get_userdata(uniform_data_source_key source) const override;
    };
  }
}
