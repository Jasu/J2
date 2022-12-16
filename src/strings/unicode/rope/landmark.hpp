#pragma once

#include "strings/unicode/line_breaks_fwd.hpp"
#include "strings/unicode/utf8_string_view.hpp"
#include "strings/unicode/rope/rope_utf8_metrics.hpp"

namespace j::strings::inline unicode::inline rope {
  class landmark;
  class rope_utf8_value;

  struct landmark_change final {
    line_break_state previous_lbr_state;
    line_break_state new_lbr_state;
    i64_t hard_breaks_delta = 0;
    i64_t length_delta = 0;

    constexpr landmark_change() noexcept = default;

    landmark_change(const rope_utf8_value * v) noexcept;

    J_INLINE_GETTER constexpr bool lbr_state_changed() const noexcept
    { return previous_lbr_state != new_lbr_state; }

    J_INLINE_GETTER constexpr bool should_propagate() const noexcept
    { return lbr_state_changed(); }

    J_INLINE_GETTER constexpr rope_utf8_metrics metrics_delta() const noexcept
    { return rope_utf8_metrics(length_delta, hard_breaks_delta); }

    J_ALWAYS_INLINE constexpr void reset_metrics_delta() noexcept
    { length_delta = hard_breaks_delta = 0; }

    J_ALWAYS_INLINE landmark_change operator+=(const landmark_change & rhs) noexcept {
      previous_lbr_state = rhs.previous_lbr_state;
      new_lbr_state = rhs.new_lbr_state;
      hard_breaks_delta += rhs.hard_breaks_delta;
      length_delta += rhs.length_delta;
      return *this;
    }
  };

  class landmark final {
  public:
    void initialize(landmark_change & change, const_utf8_string_view section) noexcept;

    void append(landmark_change & change, const_utf8_string_view section) noexcept {
      change.previous_lbr_state = m_lbr_state_after;
      do_append(change, section);
    }

    /// Forward the landmark by reading an additional section of text.
    void do_append(landmark_change & change, const_utf8_string_view section) noexcept;

    /// Forward the landmark by reading an additional section of text.
    void prepend(landmark_change & change,
                 const_utf8_string_view section,
                 const_utf8_string_view old_section) noexcept;

    /// Handle a change before the landmark, causing the preceding line break state to change.
    void handle_lbr_state_change(landmark_change & change,
                                 const_utf8_string_view section) noexcept;

    /// Remove a prefix from the landmark.
    void remove_prefix(landmark_change & change,
                       const_utf8_string_view removed_section,
                       const_utf8_string_view remaining_section) noexcept;


    J_ALWAYS_INLINE void assign(u16_t offset, u16_t breaks) noexcept {
      J_ASSUME(offset > 0);
      J_ASSUME(breaks <= offset);
      m_offset = offset;
      m_hard_breaks_before = breaks;
    }

    /// Remove a prefix from the landmark.
    J_ALWAYS_INLINE void remove_prefix(u16_t offset, u16_t breaks) noexcept {
      J_ASSERT(m_offset > offset && m_hard_breaks_before >= breaks && offset >= breaks, "Out of range");
      m_offset -= offset;
      m_hard_breaks_before -= breaks;
    }

    /// Remove a prefix from the landmark.
    J_ALWAYS_INLINE void remove_suffix(
      landmark_change & change,
      const_utf8_string_view removed_section,
      const_utf8_string_view remaining_section
    ) noexcept {
      J_ASSERT_RANGE(1, removed_section.size(), m_offset);
      J_ASSERT(removed_section.size() + remaining_section.size() == m_offset, "Content mismatch");
      return initialize(change, remaining_section);
    }

    /// Get the byte offset from the start of the rope chunk..
    J_INLINE_GETTER constexpr u16_t offset() const noexcept
    { return m_offset; }

    /// Get the number of hard breaks from the previous landmark.
    J_INLINE_GETTER constexpr u16_t hard_breaks_before() const noexcept
    { return m_hard_breaks_before; }

    /// Get the line break state after the landmark.
    J_INLINE_GETTER constexpr line_break_state lbr_state_after() const noexcept
    { return m_lbr_state_after; }

    /// Set the line break state after the landmark.
    J_ALWAYS_INLINE constexpr void set_lbr_state_after(line_break_state lbr) noexcept
    { m_lbr_state_after = lbr; }

    /// Reset the landmark to empty.
    J_ALWAYS_INLINE constexpr void reset() noexcept {
      m_offset = 0U;
      m_hard_breaks_before = 0U;
      m_lbr_state_after.reset_state();
    }

    J_ALWAYS_INLINE constexpr void reset(landmark_change & change) noexcept {
      change.length_delta -= m_offset;
      change.hard_breaks_delta -= m_hard_breaks_before;
      change.previous_lbr_state = m_lbr_state_after;
      reset();
    }

    J_ALWAYS_INLINE constexpr landmark & operator+=(const landmark & rhs) noexcept {
      m_offset += rhs.m_offset;
      m_hard_breaks_before += rhs.m_hard_breaks_before;
      m_lbr_state_after = rhs.m_lbr_state_after;
      return *this;
    }

    /// Get whether the landmark is initialized.
    ///
    /// \note Landmark at position 0 is not initialized - there is no text before
    ///       it, so it would be useless.
    J_INLINE_GETTER constexpr explicit operator bool() const noexcept
    { return m_offset; }

    /// Get whether the landmark is uninitialized.
    ///
    /// \note Landmark at position 0 is not initialized - there is no text before
    ///       it, so it would be useless.
    J_INLINE_GETTER constexpr bool operator!() const noexcept
    { return !m_offset; }

    J_INLINE_GETTER constexpr bool operator==(const landmark & rhs) const noexcept = default;
  private:
    /// Byte offset from the start of the rope chunk.
    u16_t m_offset = 0U;

    /// Number of hard breaks from the previous landmark.
    u16_t m_hard_breaks_before = 0U;

    /// Line break state after the rope chunk.
    line_break_state m_lbr_state_after;
  };

  /// Empty landmark, useful for getting a reference to a landmark before any text is met.
  inline constexpr landmark g_empty_landmark{};
}
