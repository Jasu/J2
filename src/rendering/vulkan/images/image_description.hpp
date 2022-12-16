#pragma once

#include "geometry/vec2.hpp"
#include "rendering/images/image_format.hpp"
#include "rendering/vulkan/attributes/common_attributes.hpp"
#include "attributes/basic_operations.hpp"

namespace j::rendering::vulkan::images {
  struct image_description final {
    J_BOILERPLATE(image_description, CTOR_CE, COPY_CE, MOVE_CE)

    j::geometry::vec2u16 size;
    j::rendering::images::image_format format;
    u8_t dimensionality = 2U;
    const char * name = nullptr;

    template<typename... Args>
    J_ALWAYS_INLINE constexpr image_description(Args && ... args) noexcept
      : format(attributes::format.get(static_cast<Args &&>(args)...))
    {
      if constexpr (j::attributes::has<Args...>(attributes::image_1d)) {
        dimensionality = 1U;
        if constexpr (j::attributes::has<Args...>(attributes::width)) {
          size = j::geometry::vec2u16{attributes::width.get(static_cast<Args &&>(args)...), 1U};
        } else {
          size = j::geometry::vec2u16{attributes::height.get(static_cast<Args &&>(args)...), 1U};
        }
      } else {
        size = j::geometry::vec2u16{attributes::width.get(static_cast<Args &&>(args)...),
                                    attributes::height.get(static_cast<Args &&>(args)...)};
      }
      if constexpr (j::attributes::has<Args...>(attributes::name)) {
        name = attributes::name.get(static_cast<Args &&>(args)...);
      }
    }
  };
}
