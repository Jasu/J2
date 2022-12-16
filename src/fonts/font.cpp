#include "fonts/font.hpp"

#include "mem/shared_ptr.hpp"
#include "containers/vector.hpp"

J_DEFINE_EXTERN_VECTOR(j::fonts::font);

namespace j::fonts {

  font::font() noexcept {
  }

  font::font(font && rhs) noexcept
    : m_shaper_font(static_cast<mem::shared_ptr<void> &&>(rhs.m_shaper_font)),
      m_rasterizer_font(static_cast<mem::shared_ptr<rasterizing::rasterizer_font> &&>(rhs.m_rasterizer_font)),
      m_size(rhs.m_size),
      m_selector_font(static_cast<mem::shared_ptr<void> &&>(rhs.m_selector_font))
  {
  }

  font & font::operator=(font && rhs) noexcept {
    if (this != &rhs) {
      m_shaper_font = static_cast<mem::shared_ptr<void> &&>(rhs.m_shaper_font);
      m_rasterizer_font = static_cast<mem::shared_ptr<rasterizing::rasterizer_font> &&>(rhs.m_rasterizer_font);
      m_size = rhs.m_size;
      m_selector_font = static_cast<mem::shared_ptr<void> &&>(rhs.m_selector_font);
    }

    return *this;
  }

  font::font(const font & rhs) noexcept
    : m_shaper_font(rhs.m_shaper_font),
      m_rasterizer_font(rhs.m_rasterizer_font),
      m_size(rhs.m_size),
      m_selector_font(rhs.m_selector_font)
  {
  }

  font & font::operator=(const font & rhs) noexcept {
    if (this != &rhs) {
      m_shaper_font = rhs.m_shaper_font;
      m_rasterizer_font = rhs.m_rasterizer_font;
      m_size = rhs.m_size;
      m_selector_font = rhs.m_selector_font;
    }
    return *this;
  }

  font::~font() {
  }

  font::font(mem::shared_ptr<void> selector_font, font_size size) noexcept
    : m_size(size),
      m_selector_font(static_cast<mem::shared_ptr<void> &&>(selector_font))
  {
  }

  void font::set_shaper_font(mem::shared_ptr<void> shaper_font) const noexcept {
    m_shaper_font = static_cast<mem::shared_ptr<void> &&>(shaper_font);
  }

  void font::set_rasterizer_font(mem::shared_ptr<rasterizing::rasterizer_font> rasterizer_font) const noexcept {
    m_rasterizer_font = static_cast<mem::shared_ptr<rasterizing::rasterizer_font> &&>(rasterizer_font);
  }
}
