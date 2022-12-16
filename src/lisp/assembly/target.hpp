#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::lisp::assembly::disassembly {
  class disassembler;
}

namespace j::lisp::assembly::inline functions {
  class function_builder;
}
namespace j::lisp::assembly {
  struct reg_info;
  struct compiler;

  enum class target_name : u8_t {
    amd64_ia64,
    target_name_max = amd64_ia64,
  };

  /// Target platform.
  class target {
  protected:
    J_A(AI) inline target() noexcept = default;
  public:
    J_A(RNN,NODISC) static const target * get_target(target_name name) noexcept;

    target(const target &) = delete;

    virtual ~target();

    J_A(RNN,NODISC) virtual const reg_info * get_reg_info() const noexcept = 0;

    [[nodiscard]] virtual const j::mem::shared_ptr<compiler> create_compiler(function_builder * J_NOT_NULL buf) const noexcept = 0;

    [[nodiscard]] virtual const J_RETURNS_NONNULL disassembly::disassembler * disassembler() const noexcept = 0;
  };
}
