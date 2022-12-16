#pragma once

#include "rendering/data_sources/source_handler.hpp"
#include "rendering/vertex_data/vertex_data_source_key.hpp"

namespace j::rendering::vertex_data {
  class vertex_buffer_info;
  namespace detail {
    template<typename Ptr> class vertex_buffer_view;
  }
  using vertex_buffer_view = detail::vertex_buffer_view<void>;
  using vertex_data_source_handler = data_sources::source_handler<vertex_data_source_key, vertex_buffer_info, vertex_buffer_view>;
}
