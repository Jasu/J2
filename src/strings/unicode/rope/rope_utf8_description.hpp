#pragma once

#include "containers/trees/rope_tree_fwd.hpp"
#include "strings/unicode/rope/rope_utf8_metrics.hpp"
#include "strings/unicode/rope/landmark.hpp"

namespace j::strings::inline unicode::inline rope {
  class rope_utf8_value;

  struct rope_utf8_controller final {
    rope_utf8_metrics compute_metrics(const rope_utf8_value & value) const noexcept;
    bool secondary_key_less_than(const rope_utf8_metrics & metrics,
                                 hard_line_breaks_key key) const noexcept;
    hard_line_breaks_key secondary_key_diff(hard_line_breaks_key key,
                                            const rope_utf8_metrics & metrics);
    u64_t secondary_key_to_index(hard_line_breaks_key key, const rope_utf8_value & value);
  };

  struct rope_utf8_description final {
    using value_t = rope_utf8_value;
    using controller_t = rope_utf8_controller;
    using key_t = u64_t;
    using key_diff_t = i64_t;
    using metrics_t = rope_utf8_metrics;
    using state_diff_t = landmark_change;
    static constexpr inline u8_t max_size_v = 4U;
  };
}
