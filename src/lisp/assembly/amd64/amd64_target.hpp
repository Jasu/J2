#pragma once

#include "lisp/assembly/target.hpp"

namespace j::lisp::assembly::amd64 {
  class amd64_disassembler;

  class amd64_target : public target {
  public:
    amd64_target();
    ~amd64_target();

    const j::mem::shared_ptr<compiler> create_compiler(function_builder * J_NOT_NULL buf) const noexcept override final;
    const J_RETURNS_NONNULL disassembly::disassembler * disassembler() const noexcept override final;
  private:
    amd64_disassembler * m_disassembler;
  };
}
