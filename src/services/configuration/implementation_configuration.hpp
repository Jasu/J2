#pragma once

namespace j::services::configuration {
  /// Configuration for a service implementing an interface.
  struct implementation_configuration final {
    /// Prirority of the implementation in the list of implementations.
    int priority;

    bool operator==(const implementation_configuration & rhs) const noexcept;
    bool operator!=(const implementation_configuration & rhs) const noexcept;
    bool operator<(const implementation_configuration & rhs) const noexcept;
    bool operator<=(const implementation_configuration & rhs) const noexcept;
    bool operator>(const implementation_configuration & rhs) const noexcept;
    bool operator>=(const implementation_configuration & rhs) const noexcept;
  };
}
