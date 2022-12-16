#pragma once

#include "lisp/assembly/disassembly/disassembler.hpp"

extern "C" {
  struct ZydisDecoder_;
}

namespace j::lisp::assembly::amd64 {
  class amd64_disassembler final : public disassembly::disassembler {
  public:
    amd64_disassembler();
    ~amd64_disassembler();

    disassembly::instruction disassemble_instruction(
      const void * J_NOT_NULL address,
      u32_t size) const override;
  private:
    ZydisDecoder_ * m_decoder;
  };
}
