#pragma once

#include "hzd/types.hpp"

namespace j::fonts {
  namespace detail {
    enum class point_size_tag_t { v };
    enum class pixel_size_tag_t { v };
  }


  inline constexpr detail::pixel_size_tag_t pixel_size_tag{detail::pixel_size_tag_t::v};
  inline constexpr detail::point_size_tag_t point_size_tag{detail::point_size_tag_t::v};

  class font_size {
    u32_t m_size:31;
    bool m_is_pixel_size:1;
  public:
    constexpr font_size() noexcept
      : m_size(0),
        m_is_pixel_size(false)
    {
    }

    constexpr font_size(detail::point_size_tag_t, u32_t size) noexcept
      : m_size(size),
        m_is_pixel_size(false)
    {
    }

    constexpr font_size(detail::pixel_size_tag_t, u32_t size) noexcept
      : m_size(size),
        m_is_pixel_size(true)
    {
    }

    constexpr bool is_pixel_size() const noexcept {
      return m_is_pixel_size;
    }

    constexpr bool is_point_size() const noexcept {
      return !m_is_pixel_size;
    }

    constexpr u32_t size() const noexcept {
      return m_size;
    }
  };
}
