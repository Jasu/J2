#include "rendering/vertex_data/vertex_binding_info.hpp"
#include "rendering/vertex_data/vertex_input_info.hpp"

namespace j::rendering::vertex_data {

  matched_vertex_binding_info::matched_vertex_binding_info(const vertex_binding_info & binding_info,
                                                           const vertex_input_info & input_info)
    : m_stride(binding_info.stride())
  {
    for (auto & b : binding_info) {
      [[maybe_unused]] const auto & i = input_info[b.key];
      u8_t location = input_info.location_of(b.key);
      J_ASSERT(m_bindings[location].type == data_types::data_type::none, "Duplicate binding");
      J_ASSERT(data_types::cols(b.target.type) == data_types::cols(i.type)
               && data_types::rows(b.target.type) == data_types::rows(i.type),
               "Shape mismatch");
      J_ASSERT(data_types::is_float(i.type) == data_types::is_float(b.target.type) || (
                 data_types::is_float(i.type) && !data_types::is_float(b.target.type)
                 && b.target.scaling != scaling::integer),
               "Data type mismatch");
      m_bindings[location] = b.target;
    }
  }
}
