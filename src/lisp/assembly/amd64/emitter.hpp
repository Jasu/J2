#pragma once

#include "lisp/assembly/linking/reloc.hpp"
#include "lisp/assembly/amd64/emit.hpp"
#include "exceptions/assert.hpp"
#include "lisp/assembly/functions/function_builder.hpp"

namespace j::lisp::assembly::inline buffers {
  struct label_record;
}

namespace j::lisp::assembly::amd64 {
  void emit_rep(function_builder & buf) {
    buf.begin_instruction();
    buf.code_builder.template append<u8_t>(0xF3U);
  }

  template<typename... Operands>
  void emit(function_builder & buf, const instruction_encodings & encs, Operands ... ops) {
    buf.begin_instruction();
    const operand_mask mask{operand(ops)...};
    const auto * const enc = encs.find_encoding(mask);
    J_REQUIRE(enc != nullptr, "Encoding for {} not found for arguments", encs.name, operand(ops)...);
    const operand operands[]{ops...};
    void * disp = nullptr, * imm = nullptr;
    char * end = emit(buf.code_builder.reserve(16U), enc, operands, &disp, &imm);
    buf.code_builder.commit(end);
    for (auto & op : operands) {
      if (op.is_reloc()) {
        auto reloc_src = op.reloc_source();
        if (op.is_imm()) {
          J_ASSUME_NOT_NULL(imm);
          if (enc->imm_format == imm_format::rel32) {
            i32_t addend = byte_offset(end, imm);
            J_ASSERT(addend == -4);
            buf.code_builder.write_reloc(imm, reloc_src, { reloc_type::rel32 }, -4 + op.imm());
          } else {
            buf.code_builder.write_reloc(imm, reloc_src, { reloc_type::abs64 }, op.imm());
          }
        } else {
          J_ASSUME_NOT_NULL(disp);
          J_ASSERT(op.is_rel());
          buf.code_builder.write_reloc(disp, reloc_src, { reloc_type::rel32 }, byte_offset(end, disp) + op.imm());
        }
      }
    }
  }

  void emit_reljmp(function_builder & buf,
                   const amd64::instruction_encodings & encs,
                   label_record * J_NOT_NULL label) {
    buf.begin_instruction();
    constexpr operand_mask reljmp_mask{operand_type_mask::imm32};
    const auto * const enc = encs.find_encoding(reljmp_mask);
    J_REQUIRE(enc != nullptr, "Encoding for reljmp {} not found", encs.name);
    buf.code_builder.commit(emit_no_imm(buf.code_builder.reserve(16U), enc, { }));

    const u8_t imm_bytes = width_mask_v & (u8_t)enc->imm_format;
    switch (imm_bytes) {
    case 1:
      buf.append_backpatch_rel8(label, -1);
      break;
    case 2:
      buf.append_backpatch_rel16(label, -2);
      break;
    case 4:
      buf.append_backpatch_rel32(label, -4);
      break;
    default:
      J_ASSERT_FAIL("Unsupported imm_bytes", imm_bytes);
    }
  }
}
