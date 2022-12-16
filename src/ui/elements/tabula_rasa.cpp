#include "ui/elements/tabula_rasa.hpp"

namespace j::ui::elements {
  void tabula_rasa::render(context & ctx, const g::rect_u16 & region) const {
    m_frame.render(ctx, region, m_box.border, m_style);
  }
}
