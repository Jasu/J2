#pragma once

#include "rendering/vertex_data/vertex_data_source_key.hpp"
#include "rendering/vertex_data/vertex_data_source_handler.hpp"
#include "rendering/data_sources/source_manager.hpp"

namespace j::inline containers {
  template<typename T> class vector;
}

namespace j::rendering::vertex_data {
  class vertex_buffer_info;
  class vertex_data_source_manager final
    : public data_sources::source_manager<vertex_data_source_key, vertex_buffer_info, vertex_buffer_view> {
  public:
    void initialize(vector<mem::shared_ptr<vertex_data_source_handler>> && handlers) noexcept;
    static vertex_data_source_manager instance;
  };
}
