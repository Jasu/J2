#pragma once

#include "lisp/common/metadata_init.hpp"
#include "lisp/cir/ops/ops.hpp"
#include "lisp/assembly/compiler.hpp"

namespace j::lisp::cir_to_mc {
  namespace {
    namespace c = cir;
    namespace o = cir::ops::defs;
    namespace a = assembly;

    [[maybe_unused]] inline void emit_untag_subtract(a::compiler * compiler, const c::op * J_NOT_NULL o, a::reg to, a::operand from, bool use_lea, imm_tag tag, const char * J_NOT_NULL info) {
      if (use_lea) {
        compiler->emit_lea(to, a::mem64(from.reg(), -(i8_t)tag), with_metadata(mdi::comment(info), o->metadata()));
      } else {
        compiler->emit_isub(to, from, a::imm8((u8_t)tag), with_metadata(mdi::comment(info), o->metadata()));
      }
    }

    [[maybe_unused]] inline void emit_tag_add(a::compiler * compiler, const c::op * J_NOT_NULL o, a::reg to, a::operand from, bool use_lea, imm_tag tag, const char * J_NOT_NULL info) {
      if (use_lea) {
        compiler->emit_lea(to, a::mem64(from.reg(), (i8_t)tag), with_metadata(mdi::comment(info), o->metadata()));
      } else {
        compiler->emit_or(to, from, a::imm8((u8_t)tag), with_metadata(mdi::comment(info), o->metadata()));
      }
    }
  }
}
