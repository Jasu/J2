#pragma once

#include "rendering/images/image_source_key.hpp"
#include "rendering/images/image_source_handler.hpp"
#include "rendering/data_sources/source_manager.hpp"

namespace j::rendering::images {
  class image_buffer_info;
  class image_source_manager final : public data_sources::source_manager<image_source_key, image_buffer_info, image_buffer_view> {
  public:
    void initialize(vector<mem::shared_ptr<image_source_handler>> && handlers) noexcept;
    static image_source_manager instance;
  };
}
