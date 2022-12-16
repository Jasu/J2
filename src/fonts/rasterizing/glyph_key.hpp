#pragma once

#include "rendering/images/image_source_key.hpp"

namespace j::fonts::rasterizing {
  struct rasterizer_font;

  class glyph_key final : public j::rendering::images::image_source_key {
  public:
    constexpr glyph_key() noexcept = default;

    explicit glyph_key(uptr_t uptr) noexcept
      : j::rendering::images::image_source_key(uptr)
    {
    }

    explicit glyph_key(j::rendering::images::image_source_key image_source_key) noexcept
      : j::rendering::images::image_source_key(image_source_key)
    {
    }

    glyph_key(u8_t image_source_index, u16_t font_index, u32_t glyph_index) noexcept
      : j::rendering::images::image_source_key(image_source_index, ((u64_t)font_index << 32) | glyph_index)
    {
    }

    J_INLINE_GETTER u16_t font_index() const noexcept {
      return uptr() >> 32;
    }

    J_INLINE_GETTER u32_t glyph_index() const noexcept {
      return uptr();
    }

    using j::rendering::images::image_source_key::operator==;
  };
}
