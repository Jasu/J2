#pragma once

#include "rendering/vulkan/uniform_buffers/uniform_buffer_layout.hpp"
#include "rendering/vulkan/attributes/common_attributes.hpp"
#include "rendering/data_types/get_data_type.hpp"
#include "attributes/basic_operations.hpp"
#include "attributes/foreach.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::rendering::vulkan {
  namespace attributes {
    namespace a = j::attributes;
    inline constexpr a::attribute_definition record{
      a::tag = type<struct record_tag>,
      a::is_multiple,
    };

    inline constexpr a::attribute_definition member{
      a::tag = type<struct member_tag>,
      a::is_multiple,
    };
  }

  inline namespace uniform_buffers {
    namespace detail {
      template<typename T>
      struct J_TYPE_HIDDEN class_size {
        J_NO_DEBUG constexpr static inline u32_t value = 0U;
      };

      template<typename C, typename T>
      struct J_TYPE_HIDDEN class_size<T C::*> {
        J_NO_DEBUG constexpr static inline u32_t value = sizeof(C);
      };
    }

    template<typename... Args>
    J_ALWAYS_INLINE_NO_DEBUG uniform_buffer_layout make_uniform_layout(Args && ... args) noexcept {
      namespace a = ::j::attributes;
      namespace va = vulkan::attributes;
      u32_t type_index = 0U;
      uniform_buffer_layout layout;
      a::foreach(va::member, [&](auto ptr) noexcept {
        layout.size = j::max(layout.size, detail::class_size<decltype(ptr)>::value);
        layout.types[type_index++] = data_types::target_type_v<decltype(ptr)>;
      }, static_cast<Args &&>(args)...);
      if constexpr (a::has<Args...>(va::size)) {
        layout.size = va::size.get(static_cast<Args &&>(args)...);
      }
      return layout;
    }

    struct uniform_buffer_record final {
      J_BOILERPLATE(uniform_buffer_record, CTOR_NE, COPY, MOVE_NE)

      uniform_buffer_layout layout;
      u16_t count = 1;

      constexpr explicit uniform_buffer_record(const uniform_buffer_layout & layout) noexcept
        : layout(layout)
      { }

      template<typename... Args>
      J_ALWAYS_INLINE_NO_DEBUG constexpr explicit uniform_buffer_record(Args && ... args) noexcept
        : layout(vulkan::attributes::layout.get_first(static_cast<Args &&>(args)...))
      {
        namespace a = ::j::attributes;
        namespace va = vulkan::attributes;
        if constexpr (a::has<Args...>(va::count)) {
          count = va::count.get(static_cast<Args &&>(args)...);
        }
      }
    };
    static_assert(sizeof(uniform_buffer_record) == 16);
  }
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::rendering::vulkan::uniform_buffer_record);

namespace j::rendering::vulkan {
  inline namespace uniform_buffers {
    struct uniform_buffer_description final {
      J_BOILERPLATE(uniform_buffer_description, CTOR_NE, COPY_DEL, MOVE_NE)

      trivial_array<uniform_buffer_record> records;

      template<typename... Args>
      J_ALWAYS_INLINE_NO_DEBUG explicit uniform_buffer_description(Args && ... args) noexcept
        : records(j::attributes::count<Args...>(vulkan::attributes::record))
      {
        namespace a = ::j::attributes;
        namespace va = vulkan::attributes;
        u32_t i = 0;
        a::foreach(va::record, [&](const uniform_buffer_record & rec) noexcept {
          records[i] = rec;
        }, static_cast<Args &&>(args)...);
      }
    };
  }
}
