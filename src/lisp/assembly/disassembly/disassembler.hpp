#pragma once

#include "lisp/assembly/disassembly/instruction.hpp"
#include "lisp/assembly/functions/function.hpp"
#include "mem/memory_region.hpp"

namespace j::lisp::assembly::disassembly {
  struct disassembly_state final {
    const void * cur = nullptr;
    const void * begin = nullptr;
    const void * end = nullptr;
    addr_info_iterator cur_label = {};
    addr_info_iterator end_label = {};
    const trap_info * cur_trap = nullptr;
    const trap_info * end_trap = nullptr;
  };

  /// Base class for platform-specific disassemblers.
  class disassembler {
  public:
    virtual ~disassembler();

    disassembler() noexcept = default;

    disassembler(disassembler &&) = delete;
    disassembler(const disassembler &) = delete;
    disassembler & operator=(disassembler &&) = delete;
    disassembler & operator=(const disassembler &) = delete;

    virtual instruction disassemble_instruction(const void * J_NOT_NULL address, u32_t size) const = 0;
    instruction get_next_instruction(disassembly_state & state) const;

    template<typename Fn, typename... Args>
    void disassemble_buffer(const j::mem::memory_region & buffer,
                            const func_info * func,
                            Fn && fn,
                            Args && ... args) const {
      disassembly_state state{
        .cur = buffer.begin(),
        .begin = buffer.begin(),
        .end = func ? add_bytes(buffer.begin(), func->func_size) : buffer.end(),
        .cur_label = func ? func->addr_info().begin() : addr_info_iterator{nullptr},
        .end_label = func ? func->addr_info().end() : addr_info_iterator{nullptr},
        .cur_trap = func->traps().begin(),
        .end_trap = func->traps().end(),
      };
      while (state.cur < state.end) {
        static_cast<Fn &&>(fn)(get_next_instruction(state), static_cast<Args &&>(args)...);
      }
    }

    template<typename Fn, typename... Args>
    void disassemble_buffer(const function & buffer, Fn && fn, Args && ... args) const {
      disassemble_buffer(buffer.code.data, buffer, static_cast<Fn &&>(fn), static_cast<Args &&>(args)...);
    }
  };
}
