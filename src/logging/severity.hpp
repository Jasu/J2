#pragma once

#include "hzd/types.hpp"

namespace j::logging {
  enum class severity : u8_t {
    /// System is unusable.
    emergency = 0U,
    /// Action must be taken immediately.
    ///
    /// Conventional wisdom is that alert signifies a condition where an issue
    /// has already caused irreparable damage - e.g. file corruption, or bad
    /// sectors in the last live disk in a RAID array.
    alert     = 1U,
    /// Critical conditions.
    ///
    /// Conventional wisdom is that a critical condition might lead to irreparable
    /// damage (i.e. to an alert condition) unless managed - e.g. a disk in a RAID
    /// array has failed.
    critical  = 2U,
    /// Error conditions.
    ///
    /// A transient error not affecting the usability of the system in general.
    error     = 3U,
    /// Warning conditions.
    ///
    /// A significant condition that might lead to an error or cause suboptimal
    /// performance.
    warning   = 4U,
    /// Normal but significant conditions.
    ///
    /// A significant transition in the system state has occurred, but it does not
    /// affect the usability of the system.
    notice    = 5U,
    /// Informational messages.
    ///
    /// General log messages that might be of interest.
    info      = 6U,
    /// Debug-level messages.
    ///
    /// Debug trace output.
    debug     = 7U,
  };
}
