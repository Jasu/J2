#pragma once

#include "lisp/assembly/disassembly/operand.hpp"
#include "containers/span.hpp"
#include "lisp/sources/source_location.hpp"

namespace j::lisp::assembly::disassembly {
  enum class instruction_category : u8_t {
    unknown,
    stack,
    mov,
    read,
    write,
    cmp,
    bitwise,
    integer,
    jmp,
    conditional_jmp,
    fp,
    errors,
  };

  struct instruction final {
    span<const operand> get_operands() const noexcept {
      return {operands_arr, num_operands};
    }

    J_INLINE_GETTER const void * J_NOT_NULL next() const noexcept {
      return add_bytes(addr, size);
    }

    /// Address of the decoded instruction.
    const void * addr = nullptr;
    /// Label of the address, if any.
    strings::const_string_view label;
    /// Assembler mnemonic of the instruction.
    strings::const_string_view mnemonic;
    /// Comment describing the instruction
    strings::const_string_view comment;
    imm_type_mask allowed_types;
    sources::source_location source_location;
    /// Offset of the decoded instruction.
    u32_t offset = 0U;
    instruction_category category = instruction_category::unknown;
    /// Offset from the label.
    i8_t label_offset = 0;
    /// Size of the instruction in bytes.
    u8_t size = 0U;
    /// Number of instruction operands.
    u8_t num_operands = 0U;
    /// Operands of the instruction.
    operand operands_arr[4] = { };
  };
}
