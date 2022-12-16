#pragma once
#include "rendering/images/image_source_handler.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::fonts::rasterizing {
  class font_rasterizer;
  namespace r = j::rendering;

  class font_image_source_handler final : public r::images::image_source_handler {
  public:
    void initialize(mem::shared_ptr<font_rasterizer> rasterizer);
    void copy_to(const r::vulkan::render_context &,
                 r::images::image_buffer_view & to,
                 rendering::images::image_source_key source_key) const override;

    strings::string describe(rendering::images::image_source_key key) const override;
    rendering::images::image_buffer_info get_info(rendering::images::image_source_key key) const override;
    J_ALWAYS_INLINE u64_t get_userdata(rendering::images::image_source_key) const noexcept override {
      return 0;
    }

    static font_image_source_handler instance;
  private:
    mem::shared_ptr<font_rasterizer> m_rasterizer;
  };
}
