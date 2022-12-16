#pragma once

#include "rendering/vulkan/spirv/instructions.hpp"

namespace j::rendering::vulkan::inline spirv {
  template<typename... Insns, typename Fn, typename Result>
  [[nodiscard]] constexpr Result visit_instruction(Fn & fn, const instruction & insn, Result default_result) {
    const opcode op = insn.opcode();
    (void)((opcode_of<Insns> == op ? (default_result = fn(reinterpret_cast<const Insns &>(insn)), true) : false) || ...);
    return default_result;
  }

  template<typename... Insns, typename Fn>
  constexpr void visit_instruction(Fn & fn, const instruction & insn) {
    const opcode op = insn.opcode();
    (void)((opcode_of<Insns> == op ? (fn(reinterpret_cast<const Insns &>(insn)), true) : false) || ...);
  }
}
