#include "strings/unicode/rope/landmark_array.hpp"
#include "strings/unicode/rope/landmark_array_impl.hpp"
#include "logging/global.hpp"

namespace j::strings::inline unicode::inline rope {
  struct J_TYPE_HIDDEN landmark_distribution final {
    u32_t positions[landmark_array::max_landmarks_v] = { 0U };

    J_INLINE_GETTER u32_t size() const noexcept {
      J_ASSUME(positions[0] != 0);
      u32_t i = 1;
      for (; i < landmark_array::max_landmarks_v && positions[i]; ++i) { }
      return i;
    }

    J_INLINE_GETTER_NONNULL const u32_t * begin() const noexcept { return positions; }
    J_INLINE_GETTER_NONNULL const u32_t * end() const noexcept { return positions + size(); }

    static landmark_distribution distribute_landmarks(const_string_view sv) noexcept;
  };

  void landmark_array::initialize(landmark_change & change, const_string_view sv) noexcept {
    auto d = landmark_distribution::distribute_landmarks(sv);
    u32_t i = 0;
    const u32_t sz = d.size();
    J_ASSUME(sz > 0);
    J_ASSUME(sz <= max_landmarks_v);
    for (; i < sz; ++i) {
      m_landmarks[i].initialize(change, sv.take_prefix(d.positions[i]));
    }
    J_ASSERT(!sv, "Fail");
    for (; i != max_landmarks_v && m_landmarks[i]; ++i) {
      m_landmarks[i].reset(change);
    }
  }

  void landmark_array::remove_prefix(landmark_change & change,
                                     const_string_view prefix,
                                     const_string_view suffix) noexcept
  {
    const line_break_state new_lbr = change.new_lbr_state;
    const const_string_view new_str = suffix;
    u32_t i = 0;
    const u32_t sz = size();
    for (; i < sz && m_landmarks[i].offset() <= prefix.size(); ++i) {
      const u16_t offset = m_landmarks[i].offset();
      J_ASSUME(offset > 0);
      prefix.remove_prefix(offset);
      m_landmarks[i].reset(change);
    }
    J_ASSUME(i < max_landmarks_v);

    // J_DEBUG("LMK #{}@{} PrefixLen={} Suf={} remove prefix LMKCHG[Len={},HBRCHG={}]", i, m_landmarks[i].offset(), prefix.size(), suffix.size(), change.length_delta, change.hard_breaks_delta);
    // J_DEBUG("           HBR {}", m_landmarks[i].hard_breaks_before());
    m_landmarks[i].remove_prefix(
      change,
      prefix,
      suffix.take_prefix(m_landmarks[i].offset() - prefix.size()));
    // J_DEBUG("Afr #{}@{} PrefixLen={} Suf={} remove prefix LMKCHG[Len={},HBRCHG={}]", i, m_landmarks[i].offset(), prefix.size(), suffix.size(), change.length_delta, change.hard_breaks_delta);
    // J_DEBUG("           HBR {}", m_landmarks[i].hard_breaks_before());

    for (u32_t j = i + 1; j < sz; ++j) {
      m_landmarks[j].handle_lbr_state_change(
        change,
        suffix.take_prefix(m_landmarks[j].offset())
      );
    }
    J_ASSERT(!suffix, "Suffix was not consumed.");

    if (i) {
      for (u32_t j = 0; j < max_landmarks_v; ++j, ++i) {
        if (i < max_landmarks_v) {
          m_landmarks[j] = m_landmarks[i];
        } else {
          m_landmarks[j].reset();
        }
      }
    }
    // J_DEBUG("Before redist [{},{},{},{}] OwnOff[{},{},{},{}] OwnSz={}",
    //         m_landmarks[0].hard_breaks_before(), m_landmarks[1].hard_breaks_before(), m_landmarks[2].hard_breaks_before(), m_landmarks[3].hard_breaks_before(),
    //         m_landmarks[0].offset(), m_landmarks[1].offset(), m_landmarks[2].offset(), m_landmarks[3].offset());
    redistribute(new_lbr, new_str);
    // J_DEBUG("After  redist [{},{},{},{}] OwnOff[{},{},{},{}] OwnSz={}",
    //         m_landmarks[0].hard_breaks_before(), m_landmarks[1].hard_breaks_before(), m_landmarks[2].hard_breaks_before(), m_landmarks[3].hard_breaks_before(),
    //         m_landmarks[0].offset(), m_landmarks[1].offset(), m_landmarks[2].offset(), m_landmarks[3].offset());
  }

  void landmark_array::remove_suffix(landmark_change & change,
                                     const_string_view prefix,
                                     const_string_view suffix) noexcept
  {
    J_ASSERT(change.previous_lbr_state == change.new_lbr_state, "LBR state changed when removing suffix.");
    J_ASSERT_NOT_NULL(prefix, suffix);
    const_string_view new_str(prefix);
    line_break_state new_lbr(change.new_lbr_state);
    u32_t i = 0;
    const u32_t sz = size();
    for (; i < sz; ++i) {
      const u16_t offset = m_landmarks[i].offset();
      if (offset <= prefix.size()) {
        prefix.remove_prefix(offset);
        change.previous_lbr_state = change.new_lbr_state = m_landmarks[i].lbr_state_after();
      } else {
        if (prefix) {
          m_landmarks[i].remove_suffix(change, suffix.take_prefix(offset - prefix.size()), prefix);
          prefix.clear();
          ++i;
        }
        break;
      }
    }
    J_ASSERT(!prefix, "Prefix not consumed.");
    J_ASSUME(i > 0);
    for (; i < sz; ++i) {
      m_landmarks[i].reset(change);
    }

    redistribute(new_lbr, new_str);
  }

  void landmark_array::append(landmark_change & change,
                              const_string_view full_text,
                              const_string_view appended) noexcept
  {
    J_ASSERT(change.previous_lbr_state == change.new_lbr_state, "LBR state changed when appending.");
    J_ASSERT_NOT_NULL(full_text, appended);
    J_ASSERT(appended.size() < full_text.size(), "Appended is longer than full text.");
    line_break_state lbr = change.new_lbr_state;
    const u32_t sz = size();
    change.new_lbr_state = change.previous_lbr_state = m_landmarks[sz - 1].lbr_state_after();
    if (sz == max_landmarks_v) {
      m_landmarks[max_landmarks_v - 1].append(change, appended);
    } else {
      m_landmarks[sz].initialize(change, appended);
    }
    redistribute(lbr, full_text);
  }

  void landmark_array::prepend(landmark_change & change,
                               const_string_view full_text,
                               const_string_view prepended) noexcept
  {
    J_ASSERT(change.previous_lbr_state == change.new_lbr_state, "LBR state changed when appending.");
    J_ASSERT_NOT_NULL(full_text, prepended);
    J_ASSERT(prepended.size() < full_text.size(), "Appended is longer than full text.");

    line_break_state lbr = change.new_lbr_state;
    auto original_text = full_text;
    u32_t sz = size();
    full_text.remove_prefix(prepended.size());
    if (sz == max_landmarks_v) {
      m_landmarks[0].prepend(change, prepended, full_text.take_prefix(m_landmarks[0].offset()));
    } else {
      ::j::memmove(m_landmarks + 1, m_landmarks, sizeof(landmark) * sz);
      m_landmarks[0].reset();
      m_landmarks[0].initialize(change, prepended);
      ++sz;
    }
    for (u32_t i = 1; i < sz; ++i) {
      m_landmarks[i].handle_lbr_state_change(change, full_text.take_prefix(m_landmarks[i].offset()));
    }
    J_ASSERT(!full_text, "Text was not consumed");
    redistribute(lbr, original_text);
  }

  landmark_array landmark_array::split(landmark_change & change,
                                       const_string_view prefix,
                                       const_string_view suffix) noexcept
  {
    J_ASSERT_NOT_NULL(prefix, suffix);
    const line_break_state lbr = change.new_lbr_state;
    const const_string_view full_prefix = prefix;
    landmark_array result;
    u32_t write_index = 0, i = 0;
    for (; i < max_landmarks_v && m_landmarks[i]; ++i) {
      const u16_t offset = m_landmarks[i].offset();
      if (offset <= prefix.size()) {
        prefix.remove_prefix(offset);
        change.previous_lbr_state = change.new_lbr_state = m_landmarks[i].lbr_state_after();
      } else {
        if (prefix) {
          result[write_index] = m_landmarks[i];
          m_landmarks[i].remove_suffix(change, suffix.prefix(offset - prefix.size()), prefix);
          change.previous_lbr_state = change.new_lbr_state;
          result[write_index].assign(-change.length_delta, -change.hard_breaks_delta);
          ++i, ++write_index;
        }
        break;
      }
    }
    for (; i < max_landmarks_v && m_landmarks[i]; ++i) {
      result[write_index++] = m_landmarks[i];
      m_landmarks[i].reset(change);
      J_ASSERT(result.size() == write_index, "Unexpected size");
    }
    redistribute(lbr, full_prefix);
    result.redistribute(change.new_lbr_state, suffix);
    return result;
  }

  void J_INTERNAL_LINKAGE landmark_array::redistribute(line_break_state lbr,
                                                       const_string_view str) noexcept
  {
    const auto d = landmark_distribution::distribute_landmarks(str);
    landmark new_landmarks[max_landmarks_v];
    u32_t input_index = 0U;
    u32_t output_index = 0U;
    const u32_t sz = size();
    for (auto l : d) {
      J_ASSUME(l > 0);
      new_landmarks[output_index].set_lbr_state_after(lbr);
      for (; input_index < sz && m_landmarks[input_index].offset() <= l; ++input_index) {
        new_landmarks[output_index] += m_landmarks[input_index];
        l -= m_landmarks[input_index].offset();
        str.remove_prefix(m_landmarks[input_index].offset());
      }
      if (l) {
        landmark_change chg;
        new_landmarks[output_index].append(chg, str.take_prefix(l));
        m_landmarks[input_index].remove_prefix(l, chg.hard_breaks_delta);
      }
      lbr = new_landmarks[output_index].lbr_state_after();
      ++output_index;
    }
    J_ASSERT(!str, "Prefix not consumed");
    J_ASSERT(output_index == d.size(), "Output index out of range");
    for (u32_t i = 0; i < max_landmarks_v; ++i) {
      m_landmarks[i] = new_landmarks[i];
    }
  }

  landmark_distribution landmark_distribution::distribute_landmarks(const_string_view sv) noexcept {

    const u32_t text_sz = sv.size();
    J_ASSUME(text_sz > 0);
    switch (text_sz / 8U) {
    case 0U: case 1U:
      return { { text_sz, 0U, 0U, 0U } };
    case 2U: {
      const u16_t first = 8 + utf8_num_successors(sv.data() + 8);
      return {{
          first,
          text_sz - first,
          0U,
          0U,
        }};
    }
    case 3U: {
      const u16_t succ_first = utf8_num_successors(sv.data() + 8);
      const u16_t succ_second = utf8_num_successors(sv.data() + 16);
      return {{
          8U + succ_first,
          8U + succ_second - succ_first,
          text_sz - 16U - succ_second,
          0U,
        }};
    }
    default: {
      const u32_t offset = text_sz / 4U;
      const u32_t succ_first = utf8_num_successors(sv.data() + offset);
      const u32_t succ_second = utf8_num_successors(sv.data() + 2 * offset);
      const u32_t succ_third = utf8_num_successors(sv.data() + 3 * offset);
      J_ASSUME(offset > 0);
      return {{
          offset + succ_first,
          offset + succ_second - succ_first,
          offset + succ_third - succ_second,
          text_sz - 3 * offset - succ_third,
        }};
    }
    }
  }
}
