#pragma once

#include "rendering/vertex_data/vertex_data_source_key.hpp"
#include "rendering/vertex_data/vertex_buffer_view.hpp"

namespace j::rendering::vertex_data {
  class static_vertex_data_source final : public const_vertex_buffer_view {
  public:
    using const_vertex_buffer_view::const_vertex_buffer_view;
    using const_vertex_buffer_view::operator=;

    vertex_data_source_key get_key() const noexcept {
      return { 0, this };
    }

    explicit operator vertex_data_source_key() const noexcept {
      return { 0, this };
    }
  };
}
