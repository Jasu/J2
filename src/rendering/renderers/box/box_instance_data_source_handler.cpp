#include "rendering/renderers/box/box_instance_data_source_handler.hpp"

#include "rendering/renderers/box/box.hpp"
#include "rendering/vertex_data/get_vertex_attribute_format.hpp"
#include "rendering/vertex_data/vertex_buffer_view.hpp"
#include "services/service_definition.hpp"
#include "strings/format.hpp"

namespace j::rendering::renderers::box {
  namespace {
    const vd::vertex_binding_info instance_info({
        { 0, &instance::rectangle_px },
        { 1, &instance::background_color },
        { 2, &instance::border_left_color },
        { 3, &instance::border_right_color },
        { 4, &instance::border_top_color },
        { 5, &instance::border_bottom_color },
        { 6, &instance::border_size },
        { 7, &instance::corner_radius },
    });
  }

  void box_instance_data_source_handler::copy_to(const vulkan::render_context &,
                                                 vd::vertex_buffer_view & to, vd::vertex_data_source_key source) const {
    J_ASSERT_NOT_NULL(to, source);
    const auto box = get_box(source);
    instance * p = reinterpret_cast<instance *>(to.data());
    J_ASSERT_NOT_NULL(p);
    *p = box->m_instance;
  }

  strings::string box_instance_data_source_handler::describe(vd::vertex_data_source_key source) const {
    const auto & i = get_box(source)->m_instance;
    return strings::format("Box at ({}, {}), {}x{}",
                           i.rectangle_px.left(), i.rectangle_px.top(),
                           i.rectangle_px.width(), i.rectangle_px.height());
  }

  vd::vertex_buffer_info box_instance_data_source_handler::get_info(vd::vertex_data_source_key) const {
    return vd::vertex_buffer_info(&instance_info, 1);
  }

  u64_t box_instance_data_source_handler::get_userdata(vd::vertex_data_source_key key) const {
    const auto box = get_box(key);
    return box->m_allocation.userdata();
  }

  J_INTERNAL_LINKAGE box * box_instance_data_source_handler::get_box(vd::vertex_data_source_key source) const {
    J_ASSERT(source.handler_index() == index(), "Wrong vertex source handler.");
    auto ptr = reinterpret_cast<box *>(source.ptr());
    J_ASSERT_NOT_NULL(ptr);
    return ptr;
  }

  namespace {
    namespace s = services;
    J_A(ND, NODESTROY) const s::service_definition<box_instance_data_source_handler> def(
      "rendering.box_instance_data_source_handler",
      "Box instance data source handler",
      s::global_singleton,
      s::implements = s::interface<vd::vertex_data_source_handler>
    );
  }
}
