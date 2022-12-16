#pragma once

#include "rendering/vertex_data/vertex_data_source_handler.hpp"

namespace j::rendering::vertex_data {
  class static_vertex_data_source_handler final : public vertex_data_source_handler {
  private:
    static_vertex_data_source_handler();
  public:
    static static_vertex_data_source_handler instance;

    void copy_to(const vulkan::render_context &,
                 vertex_buffer_view & to, vertex_data_source_key source) const override;
    strings::string describe(vertex_data_source_key source) const override;
    vertex_buffer_info get_info(vertex_data_source_key source) const override;
    u64_t get_userdata(vertex_data_source_key source) const override;
  };
}
