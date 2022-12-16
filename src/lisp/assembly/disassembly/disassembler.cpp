#include "lisp/assembly/disassembly/disassembler.hpp"

namespace j::lisp::assembly::disassembly {
  disassembler::~disassembler() {
  }

  instruction disassembler::get_next_instruction(disassembly_state & state) const {
    instruction inst = disassemble_instruction(state.cur, byte_offset(state.cur, state.end));
    const u32_t inst_begin = byte_offset(state.begin, state.cur);
    inst.offset = inst_begin;
    if (J_UNLIKELY(inst.size == 0U)) {
      state.cur = add_bytes(state.cur, 1);
    } else {
      state.cur = inst.next();
    }
    const u32_t inst_end = byte_offset(state.begin, state.cur);
    if (state.cur_label != state.end_label) {
      auto & info = *state.cur_label;
      auto offset = state.cur_label->offset;
      if (offset >= inst_begin && offset < inst_end) {
        inst.label = info.label();
        for (auto & md : info.metadata()) {
          switch (md.key.type) {
          case metadata_type::none:
            break;
          case metadata_type::source_location:
            inst.source_location = md.source_location;
            break;
          case metadata_type::result:
            break;
          case metadata_type::comment:
            if (md.comment_metadata.comment()) {
              inst.comment = md.comment_metadata.comment();
            }
            break;
          case metadata_type::operand:
            if (md.key.operand_index < 4U) {
              inst.operands_arr[md.key.operand_index].name = md.value_metadata.operand_name();
              switch (md.value_metadata.format) {
              case format_hint::none:
                break;
              case format_hint::signed_dec:
                inst.operands_arr[md.key.operand_index].flags.set(operand_flag::is_signed);
                break;
              case format_hint::hex:
                inst.operands_arr[md.key.operand_index].flags.set(operand_flag::hex);
                break;
              case format_hint::bin:
                inst.operands_arr[md.key.operand_index].flags.set(operand_flag::binary);
              }
              break;
            }
          }
        }
        if (offset != inst_begin) {
          inst.label_offset = inst_begin - offset;
        }
        if (state.cur_trap && state.cur_trap->offset == inst_begin) {
          inst.allowed_types = state.cur_trap->expected_type;
        }
      }
      while (state.cur_label != state.end_label && state.cur_label->offset < inst_end) {
        ++state.cur_label;
      }
      while (state.cur_trap != state.end_trap && state.cur_trap->offset < inst_end) {
        ++state.cur_trap;
      }
    }
    return inst;
  }
}
