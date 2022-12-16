#include "lisp/assembly/amd64/amd64_disassembler.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wduplicate-enum"
#include <Zydis/Zydis.h>
#pragma clang diagnostic pop

namespace j::lisp::assembly::amd64 {
  namespace d = disassembly;
  namespace {
    d::instruction_category get_category(ZydisDecodedInstruction * J_NOT_NULL insn, const ZydisDecodedOperand * J_AA(NN) operands) {
      switch (insn->mnemonic) {
      case ZYDIS_MNEMONIC_TEST:
      case ZYDIS_MNEMONIC_CMP:
        return d::instruction_category::cmp;
      case ZYDIS_MNEMONIC_JL:
      case ZYDIS_MNEMONIC_JLE:
      case ZYDIS_MNEMONIC_JNB:
      case ZYDIS_MNEMONIC_JNBE:
      case ZYDIS_MNEMONIC_JNL:
      case ZYDIS_MNEMONIC_JNLE:
      case ZYDIS_MNEMONIC_JNO:
      case ZYDIS_MNEMONIC_JNP:
      case ZYDIS_MNEMONIC_JNS:
      case ZYDIS_MNEMONIC_JNZ:
      case ZYDIS_MNEMONIC_JO:
      case ZYDIS_MNEMONIC_JP:
      case ZYDIS_MNEMONIC_JRCXZ:
      case ZYDIS_MNEMONIC_JS:
      case ZYDIS_MNEMONIC_JZ:
        return d::instruction_category::conditional_jmp;
      case ZYDIS_MNEMONIC_JMP:
      case ZYDIS_MNEMONIC_RET:
        return d::instruction_category::jmp;

      case ZYDIS_MNEMONIC_MOVBE:
      case ZYDIS_MNEMONIC_MOVZX:
      case ZYDIS_MNEMONIC_MOV:
        if (operands[0].type == ZYDIS_OPERAND_TYPE_MEMORY) {
          return d::instruction_category::write;
        } else if (operands[1].type == ZYDIS_OPERAND_TYPE_MEMORY) {
          return d::instruction_category::read;
        }
        return d::instruction_category::mov;

      case ZYDIS_MNEMONIC_PUSH:
      case ZYDIS_MNEMONIC_PUSHA:
      case ZYDIS_MNEMONIC_PUSHAD:
      case ZYDIS_MNEMONIC_POP:
      case ZYDIS_MNEMONIC_POPA:
      case ZYDIS_MNEMONIC_POPAD:
        return d::instruction_category::stack;
      case ZYDIS_MNEMONIC_BZHI:
      case ZYDIS_MNEMONIC_BSF:
      case ZYDIS_MNEMONIC_BSR:
      case ZYDIS_MNEMONIC_BSWAP:
      case ZYDIS_MNEMONIC_SHR:
      case ZYDIS_MNEMONIC_SHRD:
      case ZYDIS_MNEMONIC_SHRX:
      case ZYDIS_MNEMONIC_SAR:
      case ZYDIS_MNEMONIC_SARX:
      case ZYDIS_MNEMONIC_SHL:
      case ZYDIS_MNEMONIC_SHLD:
      case ZYDIS_MNEMONIC_SHLX:
      case ZYDIS_MNEMONIC_AND:
      case ZYDIS_MNEMONIC_ANDN:
      case ZYDIS_MNEMONIC_OR:
      case ZYDIS_MNEMONIC_XOR:
      case ZYDIS_MNEMONIC_ROR:
      case ZYDIS_MNEMONIC_RORX:
      case ZYDIS_MNEMONIC_ROL:
        return d::instruction_category::bitwise;
      case ZYDIS_MNEMONIC_ADD:
      case ZYDIS_MNEMONIC_SUB:
      case ZYDIS_MNEMONIC_NEG:
      case ZYDIS_MNEMONIC_MUL:
      case ZYDIS_MNEMONIC_IMUL:
      case ZYDIS_MNEMONIC_MULX:
        return d::instruction_category::integer;
      default:
        return d::instruction_category::unknown;
      }
    }

    d::operand convert_operand(u64_t inst_addr, ZydisDecodedInstruction * J_NOT_NULL insn, ZydisDecodedOperand & zydis_op) {
      d::operand_flags flags;
      if (zydis_op.visibility == ZYDIS_OPERAND_VISIBILITY_IMPLICIT) {
        flags |= d::operand_flag::implicit;
      }
      if (zydis_op.actions & ZYDIS_OPERAND_ACTION_READ) {
        flags |= d::operand_flag::read;
      }
      if (zydis_op.actions & ZYDIS_OPERAND_ACTION_WRITE) {
        flags |= d::operand_flag::read;
      }

      switch (insn->mnemonic) {
      case ZYDIS_MNEMONIC_TEST:
      case ZYDIS_MNEMONIC_AND:
      case ZYDIS_MNEMONIC_OR:
      case ZYDIS_MNEMONIC_XOR:
      case ZYDIS_MNEMONIC_NOT:
      case ZYDIS_MNEMONIC_ANDN:
        flags |= disassembly::operand_flag::binary;
        break;
      default:
        break;
      }
      switch (zydis_op.type) {
      case ZYDIS_OPERAND_TYPE_REGISTER:
        return disassembly::operand{
          .type = disassembly::operand_type::immediate,
          .flags = flags,
          .base = disassembly::value(
            disassembly::reg_tag,
            ZydisRegisterGetString(zydis_op.reg.value)),
        };
      case ZYDIS_OPERAND_TYPE_MEMORY: {
        disassembly::operand_type op_type =
          zydis_op.mem.type == ZYDIS_MEMOP_TYPE_AGEN
          ? disassembly::operand_type::immediate
          : disassembly::operand_type::memory;
        mem_width w = mem_width::none;
        if (zydis_op.mem.type != ZYDIS_MEMOP_TYPE_AGEN) {
          w = width(insn->operand_width);
        }
        disassembly::value base{
          disassembly::reg_tag,
          ZydisRegisterGetString(zydis_op.mem.base)};

        if (zydis_op.mem.index == ZYDIS_REGISTER_NONE) {
          if (zydis_op.mem.disp.has_displacement) {
            return disassembly::operand{
              .type = op_type,
              .width = w,
              .flags = flags,
              .base = base,
              .displacement = (i32_t)zydis_op.mem.disp.value,
            };
          }
          return disassembly::operand{
            .type = op_type,
            .width = w,
            .flags = flags,
            .base = base,
          };
        } else {
          return disassembly::operand{
            .type = op_type,
            .width = w,
            .flags = flags,
            .base = base,
            .index = disassembly::value(
              disassembly::reg_tag,
              ZydisRegisterGetString(zydis_op.mem.index)),
            .scale = zydis_op.mem.scale,
            .displacement = zydis_op.mem.disp.has_displacement
              ? (i32_t)zydis_op.mem.disp.value
              : 0,
          };
        }
      }
      case ZYDIS_OPERAND_TYPE_IMMEDIATE: {
        disassembly::value value;
        u64_t abs = 1;
        disassembly::value_width width = disassembly::value_width::qword;
        switch (zydis_op.size) {
        case 8:
          width = disassembly::value_width::byte;
          break;
        case 16:
          width = disassembly::value_width::word;
          break;
        case 32:
          width = disassembly::value_width::dword;
          break;
        }
        if (zydis_op.imm.is_relative) {
          ZydisCalcAbsoluteAddress(insn, &zydis_op, inst_addr, &abs);
        }
        if (zydis_op.imm.is_signed) {
          value = disassembly::value(
            disassembly::signed_immediate_tag,
            width,
            zydis_op.imm.value.s);
        } else {
          value = disassembly::value(
            disassembly::unsigned_immediate_tag,
            width,
            zydis_op.imm.value.u);
        }
        return disassembly::operand{
          .type = zydis_op.imm.is_relative ? disassembly::operand_type::relative : disassembly::operand_type::immediate,
          .flags = flags,
          .base = value,
          .absolute = (i64_t)abs,
        };
      }
      case ZYDIS_OPERAND_TYPE_UNUSED:
      case ZYDIS_OPERAND_TYPE_POINTER:
        J_THROW("Unsupported operand");
      }
    }
  }


  amd64_disassembler::amd64_disassembler()
    : m_decoder((ZydisDecoder*)::operator new(sizeof(ZydisDecoder)))
  {
    if (!ZYAN_SUCCESS(ZydisDecoderInit(m_decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64))) {
      J_THROW("ZydisDecoderInit failed.");
    }
  }

  amd64_disassembler::~amd64_disassembler() {
    J_ASSUME_NOT_NULL(m_decoder);
    ::operator delete(m_decoder);
  }

  disassembly::instruction amd64_disassembler::disassemble_instruction(
    const void * J_NOT_NULL address,
    u32_t size) const
  {
    J_ASSUME_NOT_NULL(m_decoder);
    if (J_UNLIKELY(!size)) {
      return disassembly::instruction{};
    }
    ZydisDecodedInstruction zydis_instr;

    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT_VISIBLE];
    if (!ZYAN_SUCCESS(ZydisDecoderDecodeFull(m_decoder, address, size, &zydis_instr, operands,
                                             ZYDIS_MAX_OPERAND_COUNT_VISIBLE, ZYDIS_DFLAG_VISIBLE_OPERANDS_ONLY))) {
      if (*(const u8_t *)address == 0xCE) {
        return disassembly::instruction{
          .addr = address,
          .mnemonic = "into",
          .comment = "Illegal in 64-bit, error trap",
          .category = d::instruction_category::errors,
          .size = 1U,
        };
      }
      return disassembly::instruction{};
    }
    disassembly::instruction instr{
      .addr = address,
      .mnemonic = ZydisMnemonicGetString(zydis_instr.mnemonic),
      .category = get_category(&zydis_instr, operands),
      .size = zydis_instr.length,
    };
    for (u32_t i = 0U; i < zydis_instr.operand_count; ++i) {
      if (operands[i].visibility == ZYDIS_OPERAND_VISIBILITY_HIDDEN) {
        continue;
      }
      instr.operands_arr[instr.num_operands++] = convert_operand((u64_t)address, &zydis_instr, operands[i]);
    }
    return instr;
  }
}
