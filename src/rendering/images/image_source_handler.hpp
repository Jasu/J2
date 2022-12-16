#pragma once

#include "rendering/data_sources/source_handler.hpp"
#include "rendering/images/image_source_key.hpp"

namespace j::rendering::images {
  class image_buffer_info;
  namespace detail {
    template<typename> class image_buffer_view;
  }
  using image_buffer_view = detail::image_buffer_view<u8_t>;

  using image_source_handler = data_sources::source_handler<image_source_key, image_buffer_info, image_buffer_view>;
}
