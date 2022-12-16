#include "strings/formatters/enum_formatter.hpp"
#include "rendering/vulkan/spirv/variable_info.hpp"
#include "rendering/vulkan/spirv/type_info.hpp"

namespace j::rendering::vulkan::inline spirv {
  namespace {
    namespace s = j::strings;
    J_A(ND, NODESTROY) const s::formatters::enum_formatter<variable_kind> kind_formatter{{
      { variable_kind::other, "Other" },
      { variable_kind::input, "Input" },
      { variable_kind::output, "Output" },
      { variable_kind::uniform, "Uniform" },
      { variable_kind::push_constant, "Push constant" },
      { variable_kind::spec_constant, "Specialization constant" },
    }};

    J_A(ND, NODESTROY) const s::formatters::enum_formatter<interpolation> interpolation_formatter{{
      { interpolation::smooth, "Smooth" },
      { interpolation::flat, "Flat" },
      { interpolation::no_perspective, "Nonperspective" },
    }};

    J_A(ND, NODESTROY) const s::formatters::enum_formatter<image_dimensionality> dim_formatter{{
      { image_dimensionality::dim_1d, "1D" },
      { image_dimensionality::dim_2d, "2D" },
      { image_dimensionality::dim_3d, "3D" },
      { image_dimensionality::dim_cube, "Cubemap" },
      { image_dimensionality::dim_buffer, "Buffer" },
      { image_dimensionality::dim_subpass_data, "Subpass data" },
      { image_dimensionality::unknown, "Unknown" },
    }};

    J_A(ND, NODESTROY) const s::formatters::enum_formatter<image_sampling_type> sampling_formatter{{
      { image_sampling_type::unknown, "Unknown" },
      { image_sampling_type::sampled, "Sampled" },
      { image_sampling_type::storage, "Storage" },
    }};

    J_A(ND, NODESTROY) const s::formatters::enum_formatter<image_depth_type> depth_formatter{{
      { image_depth_type::not_depth, "Non-depth" },
      { image_depth_type::depth, "Depth" },
      { image_depth_type::unknown, "Unknown" },
    }};
  }
}
