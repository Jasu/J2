#pragma once

#include "mem/shared_ptr_fwd.hpp"

#include "fonts/font_size.hpp"

namespace j::fonts {
  namespace rasterizing {
    struct rasterizer_font;
  }

  /// Base class for fonts.
  class font {
  public:
    font() noexcept;
    font(const font & rhs) noexcept;
    font & operator=(const font & rhs) noexcept;
    font(font && rhs) noexcept;
    font & operator=(font && rhs) noexcept;
    ~font();

    font(mem::shared_ptr<void> selector_font, font_size size) noexcept;

    void set_shaper_font(mem::shared_ptr<void> shaper_font) const noexcept;

    void set_rasterizer_font(mem::shared_ptr<rasterizing::rasterizer_font> rasterizer_font) const noexcept;

    const mem::shared_ptr<void> & selector_font() const noexcept {
      return m_selector_font;
    }

    const mem::shared_ptr<void> & shaper_font() const noexcept {
      return m_shaper_font;
    }

    const mem::shared_ptr<rasterizing::rasterizer_font> & rasterizer_font() const noexcept {
      return m_rasterizer_font;
    }

    font_size size() const noexcept {
      return m_size;
    }

    using nicely_copyable_tag_t = void;
    using zero_initializable_tag_t = void;
  private:
    mutable mem::shared_ptr<void> m_shaper_font;
    mutable mem::shared_ptr<rasterizing::rasterizer_font> m_rasterizer_font;
    font_size m_size;
    mem::shared_ptr<void> m_selector_font;
  };
}
