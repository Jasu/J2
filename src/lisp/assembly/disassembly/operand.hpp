#pragma once

#include "lisp/assembly/disassembly/value.hpp"
#include "lisp/common/id.hpp"
#include "lisp/common/mem_width.hpp"
#include "lisp/common/operand_name.hpp"

namespace j::lisp::assembly::disassembly {
  enum class operand_type : u8_t {
    none,

    immediate,
    memory,
    relative,
  };

  enum class operand_flag : u8_t {
    implicit,
    write,
    read,
    binary,
    hex,
    is_signed,
  };

  using operand_flags = util::flags<operand_flag, u8_t>;
  J_FLAG_OPERATORS(operand_flag, u8_t)

  class operand final {
  public:
    J_INLINE_GETTER bool is_implicit() const noexcept       { return flags.has(operand_flag::implicit); }

    J_INLINE_GETTER bool is_written() const noexcept        { return flags.has(operand_flag::write); }
    J_INLINE_GETTER bool is_read() const noexcept           { return flags.has(operand_flag::read); }
    J_INLINE_GETTER bool is_binary() const noexcept         { return flags.has(operand_flag::binary); }
    J_INLINE_GETTER bool is_hex() const noexcept            { return flags.has(operand_flag::hex); }
    J_INLINE_GETTER bool is_signed() const noexcept         { return flags.has(operand_flag::is_signed); }

    J_INLINE_GETTER bool empty() const noexcept             { return type == operand_type::none; }
    J_INLINE_GETTER explicit operator bool() const noexcept { return type != operand_type::none; }
    J_INLINE_GETTER bool operator!() const noexcept         { return type == operand_type::none; }

    operand_type type:4 = operand_type::none;
    mem_width width:4 = mem_width::none;
    operand_flags flags;

    value base;
    value index;
    union {
      i64_t absolute = 0;
      i64_t scale;
    };
    i32_t displacement = 0;
    operand_name name{};
  };
}
