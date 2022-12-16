#pragma once

#include "command.hpp"
#include "geometry/rect.hpp"

namespace j::rendering::vulkan::inline command_buffers {
  namespace g = j::geometry;
  enum class full_size_tag_t { v };
  inline constexpr full_size_tag_t full_size{full_size_tag_t::v};
  /// Sets the viewport and scissor.
  class set_viewport_command final : public command {
  public:
    set_viewport_command(state::node_insertion_context & context, const g::rect_u16 & region);

    set_viewport_command(state::node_insertion_context & context, full_size_tag_t)
      : set_viewport_command(context, g::rect_u16(0, 0, U16_MAX, U16_MAX))
    { }

    void execute(command_context & context, state::reserved_resources_t &) const override;
    strings::string name() const override;
  private:
    g::rect_u16 m_region;
  };
}
