#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_data_source_key.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_data_source_handler.hpp"
#include "rendering/data_sources/source_manager.hpp"

namespace j::rendering::vulkan {
  struct uniform_buffer_layout;
  inline namespace uniform_buffers {
    class uniform_data_source_manager final
      : public data_sources::source_manager<uniform_data_source_key, uniform_buffer_layout, uniform_view> {
    public:
      void initialize(vector<mem::shared_ptr<uniform_data_source_handler>> && handlers) noexcept;
      static uniform_data_source_manager instance;
    };
  }
}
