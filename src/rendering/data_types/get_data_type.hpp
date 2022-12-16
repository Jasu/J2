#pragma once

#include "rendering/data_types/data_type.hpp"
#include "hzd/type_traits.hpp"

namespace j::colors {
  template<typename> struct rgb;
  template<typename> struct rgba;
}

namespace j::geometry {
  template<typename> struct vec2;
  template<typename> struct vec4;
  template<typename> struct perimeter;
  template<typename Num> class rect;
}

namespace j::rendering::data_types {
namespace detail {
  template<typename ElementT>
    constexpr data_type get_data_type_impl(const u8_t shape) noexcept {
      u8_t fmt = shape;
      if constexpr (is_floating_point_v<ElementT>) {
        fmt |= type_fp;
      } else if constexpr (is_signed_v<ElementT>) {
        fmt |= type_signed;
      } else {
        fmt |= type_unsigned;
      }
      if constexpr (sizeof(ElementT) == 2U) {
        fmt |= elem_16;
      } else if constexpr (sizeof(ElementT) == 4U) {
        fmt |= elem_32;
      } else if constexpr (sizeof(ElementT) == 8U) {
        fmt |= elem_64;
      } else {
        static_assert(sizeof(ElementT) == 1U);
      }
      return (data_type)fmt;
    }
    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v = get_data_type_impl<T>(0U);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<geometry::vec2<T>> = get_data_type_impl<T>(shape_vec2);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<geometry::vec4<T>> = get_data_type_impl<T>(shape_vec4);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<colors::rgb<T>> = get_data_type_impl<T>(shape_vec3);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<colors::rgba<T>> = get_data_type_impl<T>(shape_vec4);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<geometry::rect<T>> = get_data_type_impl<T>(shape_vec4);

    template<typename T>
    [[maybe_unused]] J_NO_DEBUG static inline constexpr data_type get_data_type_helper_v<geometry::perimeter<T>> = get_data_type_impl<T>(shape_vec4);

    template<typename T>
    J_NO_DEBUG inline constexpr data_type target_type_helper_v = T::__j_non_existent;

    template<typename T>
    J_NO_DEBUG inline constexpr data_type target_type_helper_v<T *> = get_data_type_helper_v<T>;

    template<typename C, typename T>
    J_NO_DEBUG inline constexpr data_type target_type_helper_v<T C::*> = get_data_type_helper_v<remove_const_t<T>>;
  }

  template<typename T>
  J_NO_DEBUG inline constexpr data_type target_type_v = detail::target_type_helper_v<j::remove_cref_t<T>>;

  template<typename T>
  inline constexpr data_type data_type_v = detail::get_data_type_helper_v<T>;
}
