#include "lisp/assembly/amd64/amd64_target.hpp"

#include "lisp/assembly/amd64/amd64_disassembler.hpp"
#include "lisp/assembly/amd64/amd64_compiler.hpp"
#include "mem/shared_ptr.hpp"

namespace j::lisp::assembly::amd64 {
  amd64_target::amd64_target()
    : m_disassembler(new amd64_disassembler())
  { }
  amd64_target::~amd64_target()
  {
    delete m_disassembler;
  }

  const disassembly::disassembler * amd64_target::disassembler() const noexcept {
    return m_disassembler;
  }

  const j::mem::shared_ptr<compiler> amd64_target::create_compiler(function_builder * J_NOT_NULL buf) const noexcept {
    return j::mem::make_shared<amd64_compiler>(buf);
  }
}
