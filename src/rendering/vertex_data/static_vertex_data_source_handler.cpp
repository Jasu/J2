#include "rendering/vertex_data/static_vertex_data_source_handler.hpp"
#include "rendering/vertex_data/vertex_buffer_view.hpp"
#include "rendering/vertex_data/static_vertex_data_source.hpp"
#include "hzd/string.hpp"
#include "strings/format.hpp"

namespace j::rendering::vertex_data {
  static_vertex_data_source_handler::static_vertex_data_source_handler()
    : vertex_data_source_handler(0)
  {
  }

  void static_vertex_data_source_handler::copy_to(
    const vulkan::render_context &,
    vertex_buffer_view & to, vertex_data_source_key source) const
  {
    J_ASSERT_NOT_NULL(to, source);
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    auto static_src = reinterpret_cast<static_vertex_data_source *>(source.ptr());
    const u8_t * src_data = reinterpret_cast<const u8_t*>(static_src->data());
    u8_t * dst_data = reinterpret_cast<u8_t*>(to.data());
    J_ASSERT_NOT_NULL(src_data, dst_data);
    const u32_t vertex_sz = static_src->info().binding_info().stride();
    J_ASSERT(to.info().binding_info().stride() == vertex_sz, "Vertex size mismatch.");
    u32_t num_vertices = ::j::min(static_src->info().num_vertices(), to.info().num_vertices());
    const u32_t to_stride = to.info().stride_bytes();
    const u32_t from_stride = static_src->info().stride_bytes();
    if (to_stride == vertex_sz && from_stride == vertex_sz) {
      ::j::memcpy(dst_data, src_data, vertex_sz * num_vertices);
    } else {
      while(num_vertices--) {
        ::j::memcpy(dst_data, src_data, vertex_sz);
        dst_data += to_stride, src_data += from_stride;
      }
    }
  }

  vertex_buffer_info static_vertex_data_source_handler::get_info(vertex_data_source_key source) const {
    J_ASSERT_NOT_NULL(source);
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    return reinterpret_cast<static_vertex_data_source *>(source.ptr())->info();
  }

  u64_t static_vertex_data_source_handler::get_userdata(vertex_data_source_key) const {
    return 0ULL;
  }

  strings::string static_vertex_data_source_handler::describe(vertex_data_source_key source) const {
    J_ASSERT(source.handler_index() == 0, "Source has incorrect handler.");
    return strings::format("Static vertex data at 0x{:012X}", source.uptr());
  }

  static_vertex_data_source_handler static_vertex_data_source_handler::instance{};
}
