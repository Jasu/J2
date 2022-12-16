#pragma once

#include "colors/rgb.hpp"
#include "geometry/rect.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_layout_resource.hpp"
#include "rendering/vulkan/descriptors/descriptor_set_resource.hpp"
#include "rendering/vulkan/geometry/vertex_buffer_resource.hpp"
#include "rendering/vulkan/rendering/pipeline_resource.hpp"
#include "rendering/vulkan/uniform_buffers/uniform_buffer_allocation.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  struct command_buffer_builder;
}

namespace j::rendering::renderers::box {
  namespace r = ::j::rendering;
  namespace v = r::vulkan;

  class box_instance_data_source_handler;
  class box;

  class box_renderer final {
  public:
    box_renderer(mem::shared_ptr<box_instance_data_source_handler> instance_handler) noexcept;
    void draw_box(v::command_buffers::command_buffer_builder & builder, const box & box);

  private:
    v::geometry::vertex_buffer_ref m_instance_buffer;
    v::uniform_buffer_ref m_uniform_buffer;
    v::descriptors::descriptor_set_layout_ref m_descriptor_set_layout;
    v::descriptors::descriptor_set_ref m_descriptor_set;
    v::rendering::pipeline_ref m_pipeline;
    v::uniform_buffer_allocation m_uniform;
    mem::shared_ptr<box_instance_data_source_handler> m_instance_handler;

    friend class draw_box_command;
    friend class box_vertex_data_source_handler;
  };
}
