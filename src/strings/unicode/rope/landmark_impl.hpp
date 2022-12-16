#pragma once

#include "strings/unicode/rope/landmark.hpp"
#include "strings/unicode/line_breaks.hpp"
#include "strings/unicode/utf8_string_view_access.hpp"

namespace j::strings::inline unicode::inline rope {
  inline void landmark::initialize(landmark_change & change, const_utf8_string_view section) noexcept {
    change.hard_breaks_delta -= m_hard_breaks_before;
    if (m_offset) {
      change.previous_lbr_state = m_lbr_state_after;
      change.length_delta -= m_offset;
    }
    m_offset = m_hard_breaks_before = 0U;
    m_lbr_state_after = change.new_lbr_state;
    do_append(change, section);
  }

  inline void landmark::do_append(landmark_change & change, const_utf8_string_view section) noexcept {
    J_ASSERT_NOT_NULL(section);
    J_ASSERT_RANGE(1, section.size() + m_offset, U16_MAX);
    m_offset += section.size();
    change.length_delta += section.size();
    auto lbr = m_lbr_state_after;
    for (u32_t c : section.code_points()) {
      if (lbr.get_break_type(c) == line_break_type::mandatory) {
        ++change.hard_breaks_delta;
        ++m_hard_breaks_before;
      }
      lbr.update_state(c);
    }
    m_lbr_state_after = change.new_lbr_state = lbr;
  }

    /// Forward the landmark by reading an additional section of text.
  inline void landmark::prepend(landmark_change & change,
                                const_utf8_string_view section,
                                const_utf8_string_view old_section) noexcept {
    J_ASSERT_NOT_NULL(section);
    J_ASSERT(old_section.size() == m_offset, "Old section mismatch");
    J_ASSERT_RANGE(1, section.size() + m_offset, U16_MAX);
    m_offset += section.size();
    change.length_delta += section.size();
    line_break_state lbr = change.new_lbr_state;
    for (u32_t c : section.code_points()) {
      if (lbr.get_break_type(c) == line_break_type::mandatory) {
        ++change.hard_breaks_delta;
        ++m_hard_breaks_before;
      }
      lbr.update_state(c);
    }
    change.new_lbr_state = lbr;
    handle_lbr_state_change(change, old_section);
  }

  inline void landmark::handle_lbr_state_change(landmark_change & change,
                                                const_utf8_string_view section) noexcept
  {
    J_ASSERT(section.size() <= m_offset, "Section size mismatch.");
    line_break_state cur = change.new_lbr_state, prev = change.previous_lbr_state;
    change.previous_lbr_state = m_lbr_state_after;
    for (u32_t c : section.code_points()) {
      if (cur == prev) {
        change.new_lbr_state = m_lbr_state_after;
        return;
      }
      i16_t line_delta = cur.get_break_type(c) == line_break_type::mandatory;
      line_delta -= prev.get_break_type(c) == line_break_type::mandatory;
      if (line_delta) {
        m_hard_breaks_before += line_delta;
        change.hard_breaks_delta += line_delta;
      }
      cur.update_state(c);
      prev.update_state(c);
    }
    m_lbr_state_after = change.new_lbr_state = cur;
  }

  inline void landmark::remove_prefix(landmark_change & change,
                                      const_utf8_string_view removed_section,
                                      const_utf8_string_view remaining_section) noexcept
  {
    J_ASSERT(removed_section.size() + remaining_section.size() == m_offset, "Content mismatch");
    m_offset = remaining_section.size();
    change.length_delta -= removed_section.size();
    for (u32_t c : removed_section.code_points()) {
      if (change.previous_lbr_state.get_break_type(c) == line_break_type::mandatory) {
        --change.hard_breaks_delta;
        --m_hard_breaks_before;
      }
      change.previous_lbr_state.update_state(c);
    }
    handle_lbr_state_change(change, remaining_section);
  }

}
