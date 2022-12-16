#pragma once

#include "rendering/renderers/box/box.hpp"
#include "geometry/rect.hpp"

namespace j::ui::inline styling { struct style; }

namespace j::ui::inline rendering {
  struct context;
  namespace b = j::rendering::renderers::box;
  namespace g = j::geometry;
  class element_frame {
  public:
    /// Render the background and border of the element to region.
    ///
    /// If the style has transparent background and border colors, nothing is rendered.
    void render(context & ctx, const g::rect_u16 & region,
                const g::perimeter_u8 & border_size, const style & s) const;
  private:
    mutable b::box m_box;
  };
}
