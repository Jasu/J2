#include "strings/unicode/rope/rope_utf8_value.hpp"
#include "strings/unicode/rope/marker.hpp"
#include "logging/global.hpp"
#include "exceptions/assert.hpp"
#include "strings/unicode/rope/landmark_array_impl.hpp"
#include "strings/unicode/utf8_hard_lbr_state.hpp"

namespace j::strings::inline unicode::inline rope {
  rope_utf8_value::rope_utf8_value(rope_utf8_value && rhs) noexcept
    : text(rhs.text),
      landmarks(rhs.landmarks),
      m_markers(rhs.m_markers),
      m_string(rhs.m_string)
  {
    rhs.m_string = nullptr;
    rhs.m_markers = nullptr;
    for_each_marker(*this, [&](marker & m) noexcept { m.set_chunk(this); });
  }

  rope_utf8_value & rope_utf8_value::operator=(rope_utf8_value && rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      if (m_string) {
        m_string->remove_reference();
      }
      J_ASSUME(m_markers == nullptr);
      text = rhs.text;
      landmarks = rhs.landmarks;
      m_markers = rhs.m_markers;
      m_string = rhs.m_string;
      rhs.m_string = nullptr;
      rhs.m_markers = nullptr;
      for_each_marker(*this, [&](marker & m) noexcept { m.set_chunk(this); });
    }
    return *this;
  }

  rope_utf8_value::rope_utf8_value(const rope_utf8_value & rhs) noexcept
    : text(rhs.text),
      landmarks(rhs.landmarks),
      m_string(rhs.m_string)
  {
    if (m_string) {
      m_string->add_reference();
    }
  }

  rope_utf8_value & rope_utf8_value::operator=(const rope_utf8_value & rhs) noexcept {
    if (J_LIKELY(this != &rhs)) {
      if (m_string) {
        m_string->remove_reference();
      }
      landmarks = rhs.landmarks;
      text = rhs.text;
      m_string = rhs.m_string;
      if (m_string) {
        m_string->add_reference();
      }
    }
    return *this;
  }

  void rope_utf8_value::initialize_state(landmark_change & change) noexcept {
    landmarks.reset();
    landmarks.initialize(change, text);
    rope_utf8_value * prev = ref.previous();
    rope_utf8_value * next = ref.next();
    if (prev && !next) {
      for_each_marker_at(
        *prev,
        prev->text.size(),
        [&](marker & m) noexcept { m.move_different(this, text.size()); },
        binding_to_next);
    } else if (next && !prev) {
      for_each_marker_at(
        *next,
        0,
        [&](marker & m) noexcept { m.move_different(this, 0); },
        binding_to_previous);
    } else {
      for_each_marker(
        *this,
        [&](marker & m) noexcept {
          if (m.binds_to_char_after()) {
            m.set_offset(text.size());
          }});
    }
  }
  void rope_utf8_value::update_state(landmark_change & change) noexcept
  { landmarks.handle_state_change(change, text); }

  u16_t rope_utf8_value::find_line_break(u16_t line_index) const {
    if (line_index == 0U) {
      return 0U;
    }
    u16_t offset = 0U;
    const landmark * J_RESTRICT previous_landmark = previous() ? &previous()->landmarks.back() : &g_empty_landmark;
    for (auto & lmk : landmarks) {
      if (lmk.hard_breaks_before() < line_index) {
        previous_landmark = &lmk;
        line_index -= lmk.hard_breaks_before();
        offset += lmk.offset();
        continue;
      }

      utf8_hard_lbr_state lbr(previous_landmark->lbr_state_after());
      auto section = text.without_prefix(offset);
      for (auto it = section.begin(), end = section.end(); it < end; ++it) {
        if (lbr.should_break(*it)) {
          --line_index;
          if (!line_index) {
            return offset + it - section.begin();
          }
        }
        lbr.update_state(*it);
      }
      break;
    }
    J_THROW("Line break #{} not found (have {} line breaks).", line_index, metrics().num_hard_breaks);
  }

  rope_utf8_value rope_utf8_value::split(landmark_change & change, u64_t at) noexcept {
    // J_DEBUG("Split {} LMKCHG[Len={},HBRCHG={}]", at, change.length_delta, change.hard_breaks_delta);
    J_ASSUME(at > 0);
    const utf8_string_view suffix = text.take_suffix(text.size() - at);
    rope_utf8_value result = rope_utf8_value(suffix,
                                             m_string,
                                             landmarks.split(change, text, suffix));
    // J_DEBUG("         OwnLmk[{},{},{},{}] OwnOff[{},{},{},{}] OwnSz={}",
    //         landmarks[0].hard_breaks_before(), landmarks[1].hard_breaks_before(), landmarks[2].hard_breaks_before(), landmarks[3].hard_breaks_before(),
    //         landmarks[0].offset(), landmarks[1].offset(), landmarks[2].offset(), landmarks[3].offset(),
    //         text.size());
    // J_DEBUG("         ResLmk[{},{},{},{}] ResSz={}",
    //         result.landmarks[0].hard_breaks_before(), result.landmarks[1].hard_breaks_before(), result.landmarks[2].hard_breaks_before(), result.landmarks[3].hard_breaks_before(),
    //         result.landmarks[0].offset(), result.landmarks[1].offset(), result.landmarks[2].offset(), result.landmarks[3].hard_breaks_before(),
    //         result.text.size());
    for_each_marker_after(
      *this,
      at,
      [&result, at](marker & m) noexcept {
        m.move_different(&result, m.offset() - at);
      });
    return result;
  }

  void rope_utf8_value::erase_prefix(landmark_change & change, u64_t len) noexcept {
    do_erase_prefix(change, len, ref.previous());
  }

  void rope_utf8_value::erase(landmark_change & change) noexcept {
    landmarks.reset(change);
    rope_utf8_value * J_RESTRICT next = ref.next();
    rope_utf8_value * J_RESTRICT previous = ref.previous();
    u16_t previous_size = previous ? previous->size_bytes() : 0U;
    text.clear();
    m_string = nullptr;
    for_each_marker(
      *this,
      [next, previous, previous_size](marker & m) noexcept {
        if (previous && m.binds_to_char_before()) {
          m.move_different(previous, previous_size);
        } else if (next) {
          m.move_different(next, 0U);
        } else if (previous) {
          m.move_different(previous, previous_size);
        } else {
          m.set_offset(0);
        }
      });
  }

  void rope_utf8_value::erase_suffix(landmark_change & change, u64_t len) noexcept {
    // J_DEBUG("Erase suffix {}", len);
    J_ASSUME(len > 0);
    const const_string_view suffix = text.take_suffix(len);
    landmarks.remove_suffix(change, text, suffix);
    rope_utf8_value * J_RESTRICT next = ref.next();
    u16_t sz = text.size();
    for_each_marker_after(
      *this,
      text.size(),
      [next, sz](marker & m) noexcept {
        if (next && m.binds_to_char_after()) {
          m.move_different(next, 0U);
        } else {
          m.set_offset(sz);
        }
      });
  }

  pair<rope_utf8_value, rope_utf8_metrics> rope_utf8_value::erase_middle(landmark_change & change, u64_t at, u64_t len) noexcept {
    // J_DEBUG("Erase mid {} Len{} LMKCHG[Len={},HBRCHG={}]", at, len, change.length_delta, change.hard_breaks_delta);
    // J_DEBUG("         OwnLmk[{},{},{},{}] OwnOff[{},{},{},{}] OwnSz={}",
            // landmarks[0].hard_breaks_before(), landmarks[1].hard_breaks_before(), landmarks[2].hard_breaks_before(), landmarks[3].hard_breaks_before(),
            // landmarks[0].offset(), landmarks[1].offset(), landmarks[2].offset(), landmarks[3].offset(),
            // text.size());
    // J_DEBUG("         BefSpl CHG LBR", change.previous_lbr_state);
    rope_utf8_value right_value(split(change, at));
    // J_DEBUG("         AftSpl CHG LBR", change.previous_lbr_state);
    change.reset_metrics_delta();
    change.previous_lbr_state = landmarks.lbr_state_after();
    // J_DEBUG("After Split, reset metrics");
    // J_DEBUG("         LMKCHG[Len={},HBRCHG={}]", change.length_delta, change.hard_breaks_delta);
    // J_DEBUG("         OwnLmk[{},{},{},{}] OwnOff[{},{},{},{}] OwnSz={}",
            // landmarks[0].hard_breaks_before(), landmarks[1].hard_breaks_before(), landmarks[2].hard_breaks_before(), landmarks[3].hard_breaks_before(),
            // landmarks[0].offset(), landmarks[1].offset(), landmarks[2].offset(), landmarks[3].offset(),
            // text.size());
    right_value.do_erase_prefix(change, len, this);
    return {static_cast<rope_utf8_value &&>(right_value), right_value.metrics()};
  }

  void rope_utf8_value::append_in_place(u32_t modification_stamp, const_utf8_string_view sv) noexcept {
    J_ASSERT(sv.size() <= (i32_t)space_after(), "Out of range");
    ::j::memcpy(text.end(), sv.data(), sv.size());
    handle_append_in_place(modification_stamp, sv.size());
  }

  void rope_utf8_value::append_in_place(u32_t modification_stamp, const_utf8_string_view sv1, const_utf8_string_view sv2) noexcept {
    J_ASSERT(sv1.size() + sv2.size() <= (i32_t)space_after(), "Out of range");
    ::j::memcpy(text.end(), sv1.data(), sv1.size());
    ::j::memcpy(text.end() + sv1.size(), sv2.data(), sv2.size());
    handle_append_in_place(modification_stamp, sv1.size() + sv2.size());
  }

  void rope_utf8_value::prepend_in_place(u32_t modification_stamp, const_utf8_string_view sv1, const_utf8_string_view sv2) noexcept {
    J_ASSERT(sv1.size() + sv2.size() <= (i32_t)space_before(), "Out of range");
    ::j::memcpy(text.begin() - sv2.size(), sv2.data(), sv2.size());
    ::j::memcpy(text.begin() - sv2.size() - sv1.size(), sv1.data(), sv1.size());
    handle_prepend_in_place(modification_stamp, sv1.size() + sv2.size());
  }

  void rope_utf8_value::prepend_in_place(u32_t modification_stamp, const_utf8_string_view sv) noexcept {
    J_ASSERT(sv.size() <= (i32_t)space_before(), "Out of range");
    ::j::memcpy(text.begin() - sv.size(), sv.data(), sv.size());
    handle_prepend_in_place(modification_stamp, sv.size());
  }

  void J_INTERNAL_LINKAGE rope_utf8_value::handle_prepend_in_place(
    u32_t modification_stamp, u32_t size) noexcept
  {
    text = utf8_string_view{text.data() - size,  (i32_t)(text.size() + size)};
    const auto prefix = text.prefix(size);
    landmark_change chg(previous());
    if (J_UNLIKELY(prefix.truncated_by() != 0)) {
      landmarks.initialize(chg, text);
    } else {
      landmarks.prepend(chg, text, prefix);
    }
    move_markers_after(0U, size);
    ref.propagate_state_change(modification_stamp, chg);
  }

  void J_INTERNAL_LINKAGE rope_utf8_value::handle_append_in_place(
    u32_t modification_stamp, u32_t size) noexcept
  {
    for_each_marker_at(
      *this,
      text.size(),
      [&](marker & m) noexcept { m.set_offset(text.size() + size); },
      binding_to_next);
    text = utf8_string_view{text.data(), (i32_t)(text.size() + size)};
    const auto suffix = text.suffix(size);
    landmark_change chg(previous());
    if (J_UNLIKELY(suffix.starts_with_successor_byte())) {
      landmarks.initialize(chg, text);
    } else {
      landmarks.append(chg, text, suffix);
    }
    ref.propagate_state_change(modification_stamp, chg);
  }

  void J_INTERNAL_LINKAGE rope_utf8_value::move_markers_after(u32_t offset, i32_t delta) noexcept {
    for_each_marker_after(
      *this,
      offset,
      [delta](marker & m) noexcept {
        m.set_offset(m.offset() + delta);
      });
  }

  void J_INTERNAL_LINKAGE rope_utf8_value::do_erase_prefix(landmark_change & change, u64_t len,
                                                           rope_utf8_value * J_RESTRICT previous) noexcept {
    u16_t previous_size = previous ? previous->size_bytes() : 0U;
    const const_string_view prefix = text.take_prefix(len);
    landmarks.remove_prefix(change, prefix, text);
    for_each_marker(
      *this,
      [previous, previous_size, len](marker & m) noexcept {
        if (m.offset() > len || (m.offset() == len && m.binds_to_char_after())) {
          m.set_offset(m.offset() - len);
        } else if (previous && m.binds_to_char_before()) {
          m.move_different(previous, previous_size);
        } else {
          m.set_offset(0U);
        }
      });
  }
}
