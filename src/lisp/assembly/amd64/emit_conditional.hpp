#pragma once

#include "lisp/assembly/amd64/inst/generated/jcc.hpp"
#include "lisp/assembly/amd64/inst/generated/jmp.hpp"
#include "lisp/assembly/amd64/inst/generated/cmovcc.hpp"
#include "lisp/assembly/amd64/inst/generated/setcc.hpp"
#include "lisp/assembly/amd64/emitter.hpp"
#include "lisp/assembly/flag_condition.hpp"
#include "lisp/assembly/operand.hpp"
#include "lisp/assembly/functions/function_builder.hpp"

namespace j::lisp::assembly::amd64 {
  inline void emit_conditional_branch(function_builder & buf, test_flag_condition cond, label_record * J_NOT_NULL label) {
    switch (cond) {
    case test_flag_condition::z:
      emit_reljmp(buf, inst::je, label);
      break;
    case test_flag_condition::nz:
      emit_reljmp(buf, inst::jne, label);
      break;
    case test_flag_condition::s:
      emit_reljmp(buf, inst::js, label);
      break;
    case test_flag_condition::ns:
      emit_reljmp(buf, inst::jns, label);
      break;
    case test_flag_condition::s_and_z:
      J_TODO("S and Z");
    case test_flag_condition::s_and_nz:
      J_TODO("S and NZ");
    case test_flag_condition::ns_and_z:
      J_TODO("NS and Z");
    case test_flag_condition::ns_and_nz:
      // !ZF and SF == OF, after test OF is false
      emit_reljmp(buf, inst::jg, label);
      break;
    case test_flag_condition::s_or_z:
      // ZF or SF != OF, after test OF is false
      emit_reljmp(buf, inst::jle, label);
      break;
    case test_flag_condition::s_or_nz:
      J_TODO("S OR NZ");
    case test_flag_condition::ns_or_z:
      J_TODO("NS OR Z");
    case test_flag_condition::ns_or_nz:
      J_TODO("NS OR NZ");
    }
  }

  inline void emit_conditional_branch(function_builder & buf, cmp_flag_condition cond, label_record * J_NOT_NULL label) {
    switch (cond) {
    case cmp_flag_condition::eq:
      emit_reljmp(buf, inst::je, label);
      break;
    case cmp_flag_condition::neq:
      emit_reljmp(buf, inst::jne, label);
      break;
    case cmp_flag_condition::ult:
      emit_reljmp(buf, inst::jb, label);
      break;
    case cmp_flag_condition::ulte:
      emit_reljmp(buf, inst::jbe, label);
      break;
    case cmp_flag_condition::ugt:
      emit_reljmp(buf, inst::ja, label);
      break;
    case cmp_flag_condition::ugte:
      emit_reljmp(buf, inst::jae, label);
      break;
    case cmp_flag_condition::slt:
      emit_reljmp(buf, inst::jl, label);
      break;
    case cmp_flag_condition::slte:
      emit_reljmp(buf, inst::jle, label);
      break;
    case cmp_flag_condition::sgt:
      emit_reljmp(buf, inst::jg, label);
      break;
    case cmp_flag_condition::sgte:
      emit_reljmp(buf, inst::jge, label);
      break;
    }
  }

  inline void emit_conditional_move(function_builder & buf,
                                    test_flag_condition cond,
                                    reg to,
                                    operand from) {
    switch (cond) {
    case test_flag_condition::z:
      emit(buf, inst::cmovz, to, from);
      break;
    case test_flag_condition::nz:
      emit(buf, inst::cmovnz, to, from);
      break;
    case test_flag_condition::s:
      emit(buf, inst::cmovs, to, from);
      break;
    case test_flag_condition::ns:
      emit(buf, inst::cmovns, to, from);
      break;
    case test_flag_condition::s_and_z:
      J_TODO("S and Z");
    case test_flag_condition::s_and_nz:
      J_TODO("S and NZ");
    case test_flag_condition::ns_and_z:
      J_TODO("NS and Z");
    case test_flag_condition::ns_and_nz:
      // !ZF and SF == OF, after test OF is false
      emit(buf, inst::cmovnle, to, from);
      break;
    case test_flag_condition::s_or_z:
      // ZF or SF != OF, after test OF is false
      emit(buf, inst::cmovle, to, from);
      break;
    case test_flag_condition::s_or_nz:
      J_TODO("S OR NZ");
    case test_flag_condition::ns_or_z:
      J_TODO("NS OR Z");
    case test_flag_condition::ns_or_nz:
      J_TODO("NS OR NZ");
    }
  }

  inline void emit_conditional_move(function_builder & buf, cmp_flag_condition cond, reg to, operand from) {
    switch (cond) {
    case cmp_flag_condition::eq:
      emit(buf, inst::cmovz, to, from);
      break;
    case cmp_flag_condition::neq:
      emit(buf, inst::cmovnz, to, from);
      break;
    case cmp_flag_condition::ult:
      emit(buf, inst::cmovc, to, from);
      break;
    case cmp_flag_condition::ulte:
      emit(buf, inst::cmovna, to, from);
      break;
    case cmp_flag_condition::ugt:
      emit(buf, inst::cmova, to, from);
      break;
    case cmp_flag_condition::ugte:
      emit(buf, inst::cmovnc, to, from);
      break;
    case cmp_flag_condition::slt:
      emit(buf, inst::cmovl, to, from);
      break;
    case cmp_flag_condition::slte:
      emit(buf, inst::cmovle, to, from);
      break;
    case cmp_flag_condition::sgt:
      emit(buf, inst::cmovnle, to, from);
      break;
    case cmp_flag_condition::sgte:
      emit(buf, inst::cmovnl, to, from);
      break;
    }
  }

  inline void emit_setcc(function_builder & buf, cmp_flag_condition cond, reg to) {
    switch (cond) {
    case cmp_flag_condition::eq:
      emit(buf, inst::setz, to);
      break;
    case cmp_flag_condition::neq:
      emit(buf, inst::setnz, to);
      break;
    case cmp_flag_condition::ult:
      emit(buf, inst::setc, to);
      break;
    case cmp_flag_condition::ulte:
      emit(buf, inst::setna, to);
      break;
    case cmp_flag_condition::ugt:
      emit(buf, inst::seta, to);
      break;
    case cmp_flag_condition::ugte:
      emit(buf, inst::setnc, to);
      break;
    case cmp_flag_condition::slt:
      emit(buf, inst::setl, to);
      break;
    case cmp_flag_condition::slte:
      emit(buf, inst::setle, to);
      break;
    case cmp_flag_condition::sgt:
      emit(buf, inst::setnle, to);
      break;
    case cmp_flag_condition::sgte:
      emit(buf, inst::setnl, to);
      break;
    }
  }
}
