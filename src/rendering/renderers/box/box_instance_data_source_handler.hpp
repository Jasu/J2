#pragma once

#include "rendering/vertex_data/vertex_data_source_handler.hpp"

namespace j::rendering::renderers::box {
  class box;

  namespace vd = j::rendering::vertex_data;
  class box_instance_data_source_handler final : public vd::vertex_data_source_handler {
  public:
    void copy_to(const vulkan::render_context &,
                 vd::vertex_buffer_view & to, vd::vertex_data_source_key source) const override;
    strings::string describe(vd::vertex_data_source_key source) const override;
    vd::vertex_buffer_info get_info(vd::vertex_data_source_key source) const override;
    u64_t get_userdata(vd::vertex_data_source_key key) const override;
  private:
    J_INTERNAL_LINKAGE box * get_box(vd::vertex_data_source_key source) const;
  };
}
