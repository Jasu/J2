#pragma once

#include "rendering/vulkan/command_buffers/command.hpp"

namespace j::rendering::vulkan {
  inline namespace push_constants {
    namespace detail {
      enum class copy_tag { v };
      enum class no_copy_tag { v };
    }
    J_NO_DEBUG inline constexpr detail::copy_tag copy{detail::copy_tag::v};
    J_NO_DEBUG inline constexpr detail::no_copy_tag no_copy{detail::no_copy_tag::v};

    class push_constants_command final : public command_buffers::command {
    public:
      push_constants_command(state::node_insertion_context & context,
                             detail::copy_tag,
                             u8_t push_constant_offset,
                             u8_t size,
                             const void * data,
                             const void * push_constant_tag);

      push_constants_command(state::node_insertion_context & context,
                             detail::no_copy_tag,
                             u8_t push_constant_offset,
                             const void * data);

      ~push_constants_command();

      void execute(command_buffers::command_context & context, state::reserved_resources_t &) const override;

      strings::string name() const override;

    private:
      void * m_data;
      u8_t m_push_constant_offset;
      bool m_owns_data;
      bool m_inline_data;
    };
  }
}
