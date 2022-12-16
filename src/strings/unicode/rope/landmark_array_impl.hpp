#pragma once

#include "strings/unicode/rope/landmark_array.hpp"
#include "strings/unicode/rope/landmark_impl.hpp"

namespace j::strings::inline unicode::inline rope {
  inline void landmark_array::handle_state_change(landmark_change & change, const_string_view full_text) noexcept {
    for (u32_t i = 0; i < max_landmarks_v && m_landmarks[i]; ++i) {
      m_landmarks[i].handle_lbr_state_change(change, full_text.take_prefix(m_landmarks[i].offset()));
    }
    J_ASSERT(!full_text, "Full text was not consumed.");
  }
}
