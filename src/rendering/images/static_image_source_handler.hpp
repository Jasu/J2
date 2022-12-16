#pragma once

#include "rendering/images/image_source_handler.hpp"

namespace j::rendering::images {
  class static_image_source_handler final : public image_source_handler {
  private:
    static_image_source_handler();
  public:
    static static_image_source_handler instance;

    void copy_to(const vulkan::render_context &,
                 image_buffer_view & to, image_source_key source) const override;

    strings::string describe(image_source_key source) const override;

    image_buffer_info get_info(image_source_key source) const override;

    u64_t get_userdata(image_source_key source) const override;
  };
}
