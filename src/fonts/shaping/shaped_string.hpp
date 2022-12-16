#pragma once

#include "fonts/shaping/positioned_glyph.hpp"
#include "containers/span.hpp"
#include "mem/shared_ptr_fwd.hpp"

namespace j::fonts::shaping {
  class shaped_string {
  public:
    static mem::shared_ptr<shaped_string> allocate(u32_t size, u16_t font_index, u16_t num_uniques);

    J_INLINE_GETTER_NONNULL positioned_glyph * data() noexcept
    { return reinterpret_cast<positioned_glyph *>(this + 1U); }

    J_INLINE_GETTER_NONNULL const positioned_glyph * data() const noexcept
    { return reinterpret_cast<const positioned_glyph *>(this + 1U); }

    J_INLINE_GETTER_NONNULL positioned_glyph * begin() noexcept
    { return reinterpret_cast<positioned_glyph *>(this + 1U); }

    J_INLINE_GETTER_NONNULL const positioned_glyph * begin() const noexcept
    { return reinterpret_cast<const positioned_glyph *>(this + 1U); }

    J_INLINE_GETTER_NONNULL positioned_glyph * end() noexcept
    { return begin() + m_size; }

    J_INLINE_GETTER_NONNULL const positioned_glyph * end() const noexcept
    { return begin() + m_size; }

    J_INLINE_GETTER span<const u32_t> unique_glyphs() const noexcept
    { return { reinterpret_cast<const u32_t*>(end()), m_num_uniques }; }

    J_INLINE_GETTER span<u32_t> unique_glyphs() noexcept
    { return { reinterpret_cast<u32_t*>(end()), m_num_uniques }; }

    J_INLINE_GETTER positioned_glyph & operator[](u32_t i) noexcept {
      return data()[i];
    }

    J_INLINE_GETTER const positioned_glyph & operator[](u32_t i) const noexcept {
      return data()[i];
    }

    J_ALWAYS_INLINE void set_size(float w, float h) noexcept {
      m_width = w;
      m_height = h;
    }

    J_INLINE_GETTER float width()            const noexcept { return m_width; }
    J_INLINE_GETTER float height()           const noexcept { return m_height; }

    J_INLINE_GETTER u16_t font_index()       const noexcept { return m_font_index; }
    J_INLINE_GETTER u32_t size()             const noexcept { return m_size; }
    J_INLINE_GETTER bool empty()             const noexcept { return !m_size; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return m_size; }
    J_INLINE_GETTER bool operator!()         const noexcept { return !m_size; }

    shaped_string(u32_t size, u16_t font_index, u16_t num_uniques) noexcept;
  private:
    u32_t m_size;
    float m_width = 0.0f;
    float m_height = 0.0f;
    u16_t m_font_index;
    u16_t m_num_uniques;
  };
}
