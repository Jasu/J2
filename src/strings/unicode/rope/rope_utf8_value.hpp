#pragma once

#include "strings/unicode/rope/landmark_array.hpp"
#include "strings/unicode/rope/rope_utf8_metrics.hpp"
#include "strings/ropes/rope_string.hpp"
#include "containers/trees/rope_tree_fwd.hpp"
#include "strings/unicode/rope/rope_utf8_description.hpp"

namespace j::strings::inline unicode::inline rope {
  class marker;

  namespace t = j::trees;

  class rope_utf8_value final {
  public:
    J_ALWAYS_INLINE constexpr rope_utf8_value() noexcept = default;

    J_ALWAYS_INLINE rope_utf8_value(utf8_string_view text,
                                 rope_string * J_NOT_NULL str,
                                 landmark_array landmarks = {}) noexcept
      : text(text),
        landmarks(landmarks),
        m_string(str)
    {
      J_ASSERT_NOT_NULL(text);
      J_ASSERT(text.size() < U16_MAX, "Text size out of range.");
      m_string->add_reference();
    }

    J_ALWAYS_INLINE void initialize(utf8_string_view text,
                                 rope_string * J_NOT_NULL str,
                                 landmark_array landmarks = {}) noexcept
    {
      this->text = text;
      this->landmarks = landmarks;
      m_string = str;
      J_ASSERT_NOT_NULL(text);
      J_ASSERT(text.size() < U16_MAX, "Text size out of range.");
      m_string->add_reference();
    }

    rope_utf8_value(rope_utf8_value && rhs) noexcept;

    rope_utf8_value & operator=(rope_utf8_value && rhs) noexcept;

    J_INLINE_GETTER bool is_editable() const noexcept
    { return !m_string->is_shared(); }

    J_INLINE_GETTER u32_t space_before() const noexcept
    { return is_editable() ? text.begin() - m_string->begin() : 0U; }

    J_INLINE_GETTER u32_t space_after() const noexcept
    { return is_editable() ? m_string->end() - text.end() : 0U; }

    void append_in_place(u32_t modification_stamp, const_utf8_string_view sv) noexcept;

    void append_in_place(u32_t modification_stamp, const_utf8_string_view sv1, const_utf8_string_view sv2) noexcept;

    void prepend_in_place(u32_t modification_stamp, const_utf8_string_view sv) noexcept;

    void prepend_in_place(u32_t modification_stamp, const_utf8_string_view sv1, const_utf8_string_view sv2) noexcept;

    J_INLINE_GETTER u16_t size_bytes() const noexcept
    { return text.size(); }

    J_INLINE_GETTER rope_utf8_metrics metrics() const noexcept
    { return rope_utf8_metrics(text.size(), landmarks.num_hard_breaks()); }

    void initialize_state(landmark_change & change) noexcept;

    void update_state(landmark_change & change) noexcept;

    u16_t find_line_break(u16_t line_index) const;

    rope_utf8_value split(landmark_change & change, u64_t at) noexcept;

    void erase_prefix(landmark_change & change, u64_t len) noexcept;

    void erase(landmark_change & change) noexcept;

    void erase_suffix(landmark_change & change, u64_t len) noexcept;

    pair<rope_utf8_value, rope_utf8_metrics> erase_middle(landmark_change & change, u64_t at, u64_t len) noexcept;

    rope_utf8_value(const rope_utf8_value & rhs) noexcept;

    rope_utf8_value & operator=(const rope_utf8_value & rhs) noexcept;

    J_INLINE_GETTER rope_utf8_value * previous() noexcept
    { return ref.previous(); }

    J_INLINE_GETTER const rope_utf8_value * previous() const noexcept
    { return ref.previous(); }

    J_INLINE_GETTER rope_utf8_value * next() noexcept
    { return ref.next(); }

    J_INLINE_GETTER const rope_utf8_value * next() const noexcept
    { return ref.next(); }

    ~rope_utf8_value() {
      if (m_string) {
        m_string->remove_reference();
      }
    }

    static constexpr inline u8_t max_landmarks_shift_v = 2U;
    static constexpr inline u8_t max_landmarks_v = 1U << max_landmarks_shift_v;

    J_INLINE_GETTER marker * markers() noexcept { return m_markers; }
    J_INLINE_GETTER const marker * markers() const noexcept { return m_markers; }

  private:
    void J_INTERNAL_LINKAGE handle_prepend_in_place(u32_t modification_stamp, u32_t size) noexcept;
    void J_INTERNAL_LINKAGE handle_append_in_place(u32_t modification_stamp, u32_t size) noexcept;
    void J_INTERNAL_LINKAGE move_markers_after(u32_t offset, i32_t delta) noexcept;
    void J_INTERNAL_LINKAGE do_erase_prefix(landmark_change & change, u64_t len,
                                            rope_utf8_value * J_RESTRICT previous) noexcept;

  public:
    t::rope_tree_ref<rope_utf8_description> ref;
    J_PACKED utf8_string_view text{const_cast<char *>("")};
    landmark_array landmarks;
    /// Linked list of markers on this node.
    marker * m_markers = nullptr;
    rope_string * m_string = nullptr;

    friend class marker;
  };

  J_INLINE_GETTER rope_utf8_metrics rope_utf8_controller::compute_metrics(
    const rope_utf8_value & value) const noexcept
  { return value.metrics(); }

  J_INLINE_GETTER bool rope_utf8_controller::secondary_key_less_than(
    const rope_utf8_metrics & metrics, hard_line_breaks_key key) const noexcept
  { return metrics.num_hard_breaks < key.value; }

  J_INLINE_GETTER hard_line_breaks_key rope_utf8_controller::secondary_key_diff(
    hard_line_breaks_key key, const rope_utf8_metrics & metrics)
  { return hard_line_breaks_key(key.value - metrics.num_hard_breaks); }

  J_INLINE_GETTER u64_t rope_utf8_controller::secondary_key_to_index(
    hard_line_breaks_key key, const rope_utf8_value & value)
  { return value.find_line_break(key.value); }

  inline landmark_change::landmark_change(const rope_utf8_value * val) noexcept {
    if (val) {
      previous_lbr_state = new_lbr_state = val->landmarks.lbr_state_after();
    }
  }
}
