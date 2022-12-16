#pragma once

namespace j::services::detail {
  /// Attribute specifying implements.
  template<typename Interface>
  struct implements {
    int priority = 0;

    constexpr implements() noexcept = default;

    explicit constexpr implements(int priority) noexcept : priority(priority) {
    }

    constexpr implements operator()(int priority) const noexcept {
      return implements(priority);
    }
  };
}
