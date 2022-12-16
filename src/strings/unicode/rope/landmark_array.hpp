#pragma once

#include "strings/unicode/rope/landmark.hpp"

namespace j::strings::inline unicode::inline rope {
  class landmark_array final {
  public:
    static constexpr inline u8_t max_landmarks_shift_v = 2;
    static constexpr inline u8_t max_landmarks_v = 1U << max_landmarks_shift_v;

    void initialize(landmark_change & change, const_string_view sv) noexcept;

    void reset() noexcept {
      for (u32_t i = 0; i != max_landmarks_v; ++i) {
        m_landmarks[i].reset();
      }
    }

    void reset(landmark_change & change) noexcept {
      for (u32_t i = 0; i != max_landmarks_v && m_landmarks[i]; ++i) {
        m_landmarks[i].reset(change);
      }
    }

    [[nodiscard]] u32_t size() const noexcept {
      J_ASSERT_NOT_NULL(m_landmarks[0]);
      u32_t i = 1;
      for (; i != max_landmarks_v && m_landmarks[i]; ++i) { }
      J_ASSUME(i > 0);
      J_ASSUME(i <= max_landmarks_v);
      return i;
    }

    void remove_prefix(landmark_change & change,
                       const_string_view prefix,
                       const_string_view suffix) noexcept;

    void remove_suffix(landmark_change & change,
                       const_string_view prefix,
                       const_string_view suffix) noexcept;

    void append(landmark_change & change,
                const_string_view full_text,
                const_string_view appended) noexcept;

    void prepend(landmark_change & change,
                 const_string_view full_text,
                 const_string_view prepended) noexcept;

    landmark_array split(landmark_change & change,
                         const_string_view prefix,
                         const_string_view suffix) noexcept;

    void handle_state_change(landmark_change & change, const_string_view full_text) noexcept;

    J_INLINE_GETTER line_break_state lbr_state_after() const noexcept {
      return back().lbr_state_after();
    }

    J_INLINE_GETTER u16_t num_hard_breaks() const noexcept {
      u16_t result = 0;
      for (u32_t i = 0; i < max_landmarks_v; ++i) {
        result += m_landmarks[i].hard_breaks_before();
      }
      return result;
    }

    J_INLINE_GETTER bool empty() const noexcept
    { return !m_landmarks[0]; }

    J_INLINE_GETTER landmark & back() noexcept
    { return m_landmarks[size() - 1U]; }

    J_INLINE_GETTER const landmark & back() const noexcept
    { return m_landmarks[size() - 1U]; }

    J_INLINE_GETTER_NONNULL landmark * begin() noexcept
    { return m_landmarks; }

    J_INLINE_GETTER_NONNULL const landmark * begin() const noexcept
    { return m_landmarks; }

    J_INLINE_GETTER_NONNULL landmark * end() noexcept {
      J_ASSERT_NOT_NULL(m_landmarks[0]);
      u32_t i = 1;
      for (; i != max_landmarks_v && m_landmarks[i]; ++i) { }
      return m_landmarks + i;
    }

    J_INLINE_GETTER_NONNULL const landmark * end() const noexcept
    { return const_cast<landmark_array*>(this)->end(); }

    J_INLINE_GETTER landmark & operator[](u32_t i) noexcept {
      J_ASSUME(i < max_landmarks_v);
      return m_landmarks[i];
    }

    J_INLINE_GETTER const landmark & operator[](u32_t i) const noexcept {
      J_ASSUME(i < max_landmarks_v);
      return m_landmarks[i];
    }

    J_INLINE_GETTER_NONNULL const landmark * landmarks() const noexcept
    { return m_landmarks; }

    J_INLINE_GETTER bool operator==(const landmark_array & rhs) const noexcept = default;

  private:
    void J_INTERNAL_LINKAGE redistribute(line_break_state lbr,
                                         const_string_view str) noexcept;

    landmark m_landmarks[max_landmarks_v];
  };
}
