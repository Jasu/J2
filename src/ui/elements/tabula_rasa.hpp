#pragma once

#include "ui/element.hpp"

#include "ui/rendering/element_frame.hpp"

namespace j::ui::elements {
  /// A static element that only draws its frame.
  class tabula_rasa final : public static_element {
  public:
    using static_element::static_element;
    void render(context & ctx, const g::rect_u16 & region) const override;
  private:
    element_frame m_frame;
  };
}
