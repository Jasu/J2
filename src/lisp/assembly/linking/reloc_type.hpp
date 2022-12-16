#pragma once

#include "hzd/utility.hpp"

namespace j::lisp::assembly::inline linking {
  /// Relocation field type.
  enum class reloc_type : u8_t {
    /// Invalid, uninitialized relocation.
    none,
    /// Relative 8-bit offset.
    ///
    /// \note The offset is from the beginning of the relocation field, not from
    ///       after it. On X86-64, RIP-relative addressing starts from the end,
    ///       so this must be accounted for with addend.
    rel8 = 1,
    /// Relative 16-bit offset.
    ///
    /// \note The offset is from the beginning of the relocation field, not from
    ///       after it. On X86-64, RIP-relative addressing starts from the end,
    ///       so this must be accounted for with addend.
    rel16 = 2,
    /// Relative 32-bit offset.
    ///
    /// \note The offset is from the beginning of the relocation field, not from
    ///       after it. On X86-64, RIP-relative addressing starts from the end,
    ///       so this must be accounted for with addend.
    rel32 = 4,
    /// Absolute 64-bit pointer.
    ///
    /// \note The offset is from the beginning of the relocation field, not from
    ///       after it. On X86-64, RIP-relative addressing starts from the end,
    ///       so this must be accounted for with addend.
    abs64 = 8,
  };
}
