#include "emit.hpp"

namespace j::lisp::assembly::amd64 {
  namespace {
    J_A(AI,ND,NODISC) inline bool is_reg_extended(reg r) noexcept
    { return r.index() > 7U; }

    J_A(AI,ND,NODISC) inline bool requires_rex_vex_def32(reg r) noexcept {
      return r.index() > 7U
        || (r.width() == mem_width::byte && r.index() >= 4U);
    }

    constexpr inline u8_t rex_prefix_mask = 0b01000000;

    enum w_t : u8_t {
      w0 = 0b0000U,
      w1 = 0b1000U
    };

    enum r_t : u8_t {
      r0 = 0b0000U,
      r1 = 0b0100U,
    };

    enum x_t : u8_t {
      x0 = 0b0000U,
      x1 = 0b0010U,
    };

    enum b_t : u8_t {
      b0 = 0U,
      b1 = J_BIT(0),
    };

    J_A(AI,ND,NODISC) u8_t r(bool is_extended) noexcept
    { return (u8_t)(is_extended ? r1 : r0); }

    J_A(AI,ND,NODISC) u8_t x(bool is_extended) noexcept
    { return (u8_t)(is_extended ? x1 : x0); }

    J_A(AI,ND,NODISC) u8_t b(bool is_extended) noexcept
    { return (u8_t)(is_extended ? b1 : b0); }


    enum class rex_prefix : u8_t {
      rex_w0_r0_x0_b0 = rex_prefix_mask | (u8_t)w0 | (u8_t)r0 | (u8_t)x0 | (u8_t)b0,
      rex_w0_r0_x0_b1 = rex_prefix_mask | (u8_t)w0 | (u8_t)r0 | (u8_t)x0 | (u8_t)b1,
      rex_w0_r0_x1_b0 = rex_prefix_mask | (u8_t)w0 | (u8_t)r0 | (u8_t)x1 | (u8_t)b0,
      rex_w0_r0_x1_b1 = rex_prefix_mask | (u8_t)w0 | (u8_t)r0 | (u8_t)x1 | (u8_t)b1,
      rex_w0_r1_x0_b0 = rex_prefix_mask | (u8_t)w0 | (u8_t)r1 | (u8_t)x0 | (u8_t)b0,
      rex_w0_r1_x0_b1 = rex_prefix_mask | (u8_t)w0 | (u8_t)r1 | (u8_t)x0 | (u8_t)b1,
      rex_w0_r1_x1_b0 = rex_prefix_mask | (u8_t)w0 | (u8_t)r1 | (u8_t)x1 | (u8_t)b0,
      rex_w0_r1_x1_b1 = rex_prefix_mask | (u8_t)w0 | (u8_t)r1 | (u8_t)x1 | (u8_t)b1,
      rex_w1_r0_x0_b0 = rex_prefix_mask | (u8_t)w1 | (u8_t)r0 | (u8_t)x0 | (u8_t)b0,
      rex_w1_r0_x0_b1 = rex_prefix_mask | (u8_t)w1 | (u8_t)r0 | (u8_t)x0 | (u8_t)b1,
      rex_w1_r0_x1_b0 = rex_prefix_mask | (u8_t)w1 | (u8_t)r0 | (u8_t)x1 | (u8_t)b0,
      rex_w1_r0_x1_b1 = rex_prefix_mask | (u8_t)w1 | (u8_t)r0 | (u8_t)x1 | (u8_t)b1,
      rex_w1_r1_x0_b0 = rex_prefix_mask | (u8_t)w1 | (u8_t)r1 | (u8_t)x0 | (u8_t)b0,
      rex_w1_r1_x0_b1 = rex_prefix_mask | (u8_t)w1 | (u8_t)r1 | (u8_t)x0 | (u8_t)b1,
      rex_w1_r1_x1_b0 = rex_prefix_mask | (u8_t)w1 | (u8_t)r1 | (u8_t)x1 | (u8_t)b0,
      rex_w1_r1_x1_b1 = rex_prefix_mask | (u8_t)w1 | (u8_t)r1 | (u8_t)x1 | (u8_t)b1,
    };

    template<typename... Ts>
    [[nodiscard]] inline constexpr rex_prefix encode(Ts... ts) noexcept {
      return (rex_prefix)(rex_prefix_mask | ((u8_t)ts | ...));
    }

    [[nodiscard]] inline constexpr u8_t encode_rex_m(w_t w, const operand & m_op) noexcept {
      u8_t result = rex_prefix_mask | (u8_t)w;
      if (m_op.is_rel()) {
        return result;
      }
      switch (m_op.type()) {
      case operand_type::reg:
        return result | (u8_t)b(is_reg_extended(m_op.reg()));
      case operand_type::mem8:
      case operand_type::mem16:
      case operand_type::mem32:
      case operand_type::mem64:
        result |= (u8_t)b(is_reg_extended(m_op.base()));
        if (m_op.index()) {
          result |= (u8_t)x(is_reg_extended(m_op.index()));
        }
        return result;
      default:
        J_UNREACHABLE();
      }
    }

    [[nodiscard]] inline constexpr u8_t encode_rex_r(w_t w, const operand & r_op) noexcept {
      return rex_prefix_mask | (u8_t)w | (u8_t)r(is_reg_extended(r_op.reg()));
    }

    [[nodiscard]] inline constexpr u8_t encode_rex_b(w_t w, const operand & b_op) noexcept {
      return rex_prefix_mask | (u8_t)w | (u8_t)b(is_reg_extended(b_op.reg()));
    }

    [[nodiscard]] inline constexpr u8_t encode_rex_rm(w_t w, const operand & r_op, const operand & m_op) noexcept {
      return encode_rex_m(w, m_op) | (u8_t)r(is_reg_extended(r_op.reg()));
    }

    [[nodiscard]] inline constexpr u8_t encode_rex_rm_w0(const operand & r_op, const operand & m_op) noexcept
    { return encode_rex_rm(w0, r_op, m_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_rm_w1(const operand & r_op, const operand & m_op) noexcept
    { return encode_rex_rm(w1, r_op, m_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_m_w0(const operand & m_op) noexcept
    { return encode_rex_m(w0, m_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_m_w1(const operand & m_op) noexcept
    { return encode_rex_m(w1, m_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_r_w0(const operand & r_op) noexcept
    { return encode_rex_r(w0, r_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_r_w1(const operand & r_op) noexcept
    { return encode_rex_r(w1, r_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_b_w0(const operand & r_op) noexcept
    { return encode_rex_b(w0, r_op); }

    [[nodiscard]] inline constexpr u8_t encode_rex_b_w1(const operand & r_op) noexcept
    { return encode_rex_b(w1, r_op); }

    [[nodiscard]] inline constexpr char * write_rex_w1(char * J_NOT_NULL ptr) noexcept {
      *ptr++ = rex_prefix_mask | (u8_t)w1;
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_rm_w0(char * J_NOT_NULL ptr,
                                                                const operand & r_op,
                                                                const operand & m_op) noexcept {
      *ptr++ = encode_rex_rm_w0(r_op, m_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_rm_w1(char * J_NOT_NULL ptr,
                                                                const operand & r_op,
                                                                const operand & m_op) noexcept {
      *ptr++ = encode_rex_rm_w1(r_op, m_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_r_w0(char * J_NOT_NULL ptr,
                                                                const operand & r_op) noexcept {
      *ptr++ = encode_rex_r_w0(r_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_r_w1(char * J_NOT_NULL ptr,
                                                                const operand & r_op) noexcept {
      *ptr++ = encode_rex_r_w1(r_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_b_w0(char * J_NOT_NULL ptr,
                                                                const operand & r_op) noexcept {
      *ptr++ = encode_rex_b_w0(r_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_b_w1(char * J_NOT_NULL ptr,
                                                                const operand & r_op) noexcept {
      *ptr++ = encode_rex_b_w1(r_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_m_w0(char * J_NOT_NULL ptr,
                                                                const operand & m_op) noexcept {
      *ptr++ = encode_rex_m_w0(m_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr char * write_rex_m_w1(char * J_NOT_NULL ptr,
                                                                const operand & m_op) noexcept {
      *ptr++ = encode_rex_m_w1(m_op);
      return ptr;
    }

    [[nodiscard]] inline constexpr bool requires_rex_r(const operand & r_op) noexcept
    { return requires_rex_vex_def32(r_op.reg()); }


    [[nodiscard]] inline constexpr bool requires_rex_m(const operand & rm_op) noexcept {
      if (rm_op.is_rel()) {
        return false;
      }
      return rm_op.is_mem()
        ? (is_reg_extended(rm_op.index()) || is_reg_extended(rm_op.base()))
        : requires_rex_vex_def32(rm_op.reg());
    }

    [[nodiscard]] inline constexpr bool requires_rex_rm(const operand & r_op,
                                                   const operand & rm_op) noexcept
    { return requires_rex_r(r_op) || requires_rex_m(rm_op); }


    inline J_RETURNS_NONNULL char * emit_rex(char * J_NOT_NULL ptr,
                                      const instruction_encoding * J_NOT_NULL enc,
                                      const operand * operands)
    {
      if (enc->has_rex_w) {
        switch (enc->operand_format) {
        case operand_format::none:
        case operand_format::implicit:
          return write_rex_w1(ptr);
        case operand_format::m:
        case operand_format::m_implicit:
          return write_rex_m_w1(ptr, operands[0]);
        case operand_format::ri:
        case operand_format::ri_implicit:
          return write_rex_b_w1(ptr, operands[0]);
        case operand_format::implicit_ri:
        case operand_format::implicit_m:
          return write_rex_b_w1(ptr, operands[1]);
        case operand_format::implicit2_m:
          return write_rex_b_w1(ptr, operands[2]);
        case operand_format::r:
          return write_rex_r_w1(ptr, operands[0]);
        case operand_format::mr_implicit:
        case operand_format::mr:
          return write_rex_rm_w1(ptr, operands[1], operands[0]);
        case operand_format::rm:
          return write_rex_rm_w1(ptr, operands[0], operands[1]);
        }
      }
      switch (enc->operand_format) {
      case operand_format::none:
      case operand_format::implicit:
        break;
      case operand_format::m:
      case operand_format::m_implicit:
        if (requires_rex_m(operands[0])) {
          return write_rex_m_w0(ptr, operands[0]);
        }
        break;
      case operand_format::ri:
      case operand_format::ri_implicit:
        if (requires_rex_r(operands[0])) {
          return write_rex_b_w0(ptr, operands[0]);
        }
        break;
      case operand_format::implicit_ri:
        if (requires_rex_r(operands[1])) {
          return write_rex_b_w0(ptr, operands[1]);
        }
        break;
      case operand_format::implicit2_m:
        if (requires_rex_r(operands[2])) {
          return write_rex_b_w0(ptr, operands[0]);
        }
        break;
      case operand_format::implicit_m:
        if (requires_rex_r(operands[0])) {
          return write_rex_b_w0(ptr, operands[0]);
        }
        break;
      case operand_format::r:
        if (requires_rex_r(operands[0])) {
          return write_rex_r_w0(ptr, operands[0]);
        }
        break;
      case operand_format::mr:
      case operand_format::mr_implicit:
        if (requires_rex_rm(operands[1], operands[0])) {
          return write_rex_rm_w0(ptr, operands[1], operands[0]);
        }
        break;
      case operand_format::rm:
        if (requires_rex_rm(operands[0], operands[1])) {
          return write_rex_rm_w0(ptr, operands[0], operands[1]);
        }
        break;
      }
      return ptr;
    }

    [[nodiscard]] inline constexpr bool requires_sib(const operand & rm) noexcept {
      return rm.is_mem() && (rm.index() || rm.base().is(preg::rsp) || rm.base().is(preg::r12));
    }

    [[nodiscard]] inline constexpr u8_t encode_sib(const operand & rm) noexcept {
      u8_t result = ((u8_t)normalize_scale(rm.scale(), rm.width()) - (u8_t)mem_scale::byte) << 6U | (rm.base().index() & 0x7U);
      const reg index = rm.index();
      return result | (index ? (index.index() & 0x7U) << 3U : 0b100000U);
    }

    J_INLINE_GETTER_NONNULL constexpr char * maybe_write_sib(char * J_NOT_NULL ptr, const operand & rm) noexcept {
      if (requires_sib(rm)) {
        *ptr++ = encode_sib(rm);
      }
      return ptr;
    }

  enum class mod : u8_t {
    indirect        = 0b00000000U,
    indirect_disp8  = 0b01000000U,
    indirect_disp32 = 0b10000000U,
    direct          = 0b11000000U,
  };

  [[nodiscard]] inline constexpr u8_t encode_modm(const operand & rm, u8_t reg) noexcept {
    J_ASSERT(!rm.is_imm());
    J_ASSERT(rm.type() != operand_type::none);
    u8_t result = reg << 3;
    switch (rm.type()) {
    case operand_type::reg:
      return result | (u8_t)mod::direct | (rm.reg().index() & 7U);
    case operand_type::mem8:
    case operand_type::mem16:
    case operand_type::mem32:
    case operand_type::mem64: {
      if (rm.disp() == 0U && !rm.base().is(preg::rbp) && !rm.base().is(preg::r13)) {
        result |= (u8_t)mod::indirect;
      } else if (bits::fits_i8(rm.disp())) {
        result |= (u8_t)mod::indirect_disp8;
      } else {
        J_ASSERT(bits::fits_i32(rm.disp()));
        result |= (u8_t)mod::indirect_disp32;
      }
      if (rm.index() || rm.base().is(preg::rsp) || rm.base().is(preg::r12)) {
        return result | 0b100U;
      } else {
        return result | (rm.base().index() & 7U);
      }
    }
    case operand_type::rel8:
    case operand_type::rel16:
    case operand_type::rel32:
    case operand_type::rel64:
      return result | 0b101U;
    default: J_UNREACHABLE();
    }
  }

  [[nodiscard]] inline constexpr bool requires_disp(const operand & rm) noexcept {
    return rm.is_rel()
      || (rm.is_mem() && (rm.disp() || rm.base().is(preg::rbp) || rm.base().is(preg::r13)));
  }

    [[nodiscard]] inline J_RETURNS_NONNULL constexpr char * write_disp(char * J_AA(NN,NOALIAS) ptr, const operand & J_AA(NOALIAS) rm, void ** J_AA(NOALIAS)  disp_addr) noexcept {
    if (disp_addr) {
      *disp_addr = ptr;
    }
    const i32_t disp = rm.disp();
    if (!rm.is_rel() && bits::fits_i8(disp)) {
      *ptr++ = (u8_t)disp;
      return ptr;
    } else {
      J_MEMCPY_INLINE(ptr, &disp, 4);
      return ptr + 4;
    }
  }

    [[nodiscard]] inline J_RETURNS_NONNULL constexpr char * maybe_write_disp(char * J_AA(NOALIAS,NN) ptr, const operand & J_AA(NOALIAS) rm, void ** J_AA(NOALIAS) disp_addr) noexcept {
    return requires_disp(rm) ? write_disp(ptr, rm, disp_addr) : ptr;
  }

  inline u8_t encode_modrm(const operand & J_AA(NOALIAS) r, const operand & J_AA(NOALIAS) rm) noexcept {
    J_ASSERT(r.type() == operand_type::reg);
    return encode_modm(rm, r.reg().index() & 7U);
  }

  inline J_RETURNS_NONNULL char * write_modrm(char * J_AA(NOALIAS,NN)  ptr,
                                              const operand & J_AA(NOALIAS) r,
                                              const operand & J_AA(NOALIAS) rm) noexcept
  {
    *ptr++ = encode_modrm(r, rm);
    return ptr;
  }

    inline J_RETURNS_NONNULL char * write_modm(char * J_AA(NOALIAS,NN) ptr,
                                               const operand & J_AA(NOALIAS) rm,
                                               u8_t r) noexcept
    {
      *ptr++ = encode_modm(rm, r);
      return ptr;
    }
  }

  J_RETURNS_NONNULL char * emit_no_imm(
    char *J_AA(NOALIAS,NN) ptr,
    const instruction_encoding * J_AA(NOALIAS,NN) enc,
    const operand * J_AA(NOALIAS) operands,
    void **J_AA(NOALIAS)  disp_addr)
  {
    switch (enc->mandatory_prefix) {
    case mandatory_prefix::has_f2:
      *ptr++ = 0xF2U;
      break;
    case mandatory_prefix::has_f3:
      *ptr++ = 0xF3U;
      break;
    case mandatory_prefix::has_66:
      *ptr++ = 0x66U;
      break;
    case mandatory_prefix::none: break;
    }
    ptr = emit_rex(ptr, enc, operands);
    const u8_t * opcode = enc->opcode;
    switch (enc->opcode_format) {
    case opcode_format::plain_3:
      *ptr++ = *opcode++;
      [[fallthrough]];
    case opcode_format::plain_2:
      *ptr++ = *opcode++;
      [[fallthrough]];
    case opcode_format::plain_1:
      *ptr++ = *opcode++;
      break;
    case opcode_format::plus_reg_2:
      *ptr++ = *opcode++;
      [[fallthrough]];
    case opcode_format::plus_reg_1:
      *ptr++ = *opcode++ | (operands[0].reg().index() & 7U);
    }

    switch (enc->operand_format) {
    case operand_format::none:
    case operand_format::r:
    case operand_format::ri:
    case operand_format::ri_implicit:
    case operand_format::implicit_ri:
    case operand_format::implicit:
      break;
    case operand_format::m:
    case operand_format::m_implicit:
      ptr = write_modm(ptr, operands[0], enc->modrm_forced_r);
      ptr = maybe_write_sib(ptr, operands[0]);
      ptr = maybe_write_disp(ptr, operands[0], disp_addr);
      break;
    case operand_format::implicit2_m:
      ptr = write_modm(ptr, operands[2], enc->modrm_forced_r);
      ptr = maybe_write_sib(ptr, operands[2]);
      ptr = maybe_write_disp(ptr, operands[2], disp_addr);
      break;
    case operand_format::implicit_m:
      ptr = write_modm(ptr, operands[1], enc->modrm_forced_r);
      ptr = maybe_write_sib(ptr, operands[1]);
      ptr = maybe_write_disp(ptr, operands[1], disp_addr);
      break;
    case operand_format::mr:
    case operand_format::mr_implicit:
      ptr = write_modrm(ptr, operands[1], operands[0]);
      ptr = maybe_write_sib(ptr, operands[0]);
      ptr = maybe_write_disp(ptr, operands[0], disp_addr);
      break;
    case operand_format::rm:
      ptr = write_modrm(ptr, operands[0], operands[1]);
      ptr = maybe_write_sib(ptr, operands[1]);
      ptr = maybe_write_disp(ptr, operands[1], disp_addr);
      break;
    }
    return ptr;
  }

  J_RETURNS_NONNULL char * emit(char * J_AA(NOALIAS,NN) ptr,
                                const instruction_encoding * J_AA(NOALIAS,NN) enc,
                                const operand * J_AA(NOALIAS) operands,
                                void ** J_AA(NOALIAS) disp_addr,
                                void ** J_AA(NOALIAS) imm_addr)
  {
    ptr = emit_no_imm(ptr, enc, operands, disp_addr);
    if (enc->imm_format == imm_format::none) {
      return ptr;
    }
    if (imm_addr) {
      *imm_addr = ptr;
    }
    switch (enc->operand_format) {
    case operand_format::none:
      break;
    case operand_format::r:
    case operand_format::ri:
    case operand_format::implicit:
    case operand_format::m:
      ++operands;
      break;
    case operand_format::ri_implicit:
    case operand_format::implicit_ri:
    case operand_format::rm:
    case operand_format::mr:
      operands += 2;
      break;
    case operand_format::mr_implicit:
    case operand_format::m_implicit:
    case operand_format::implicit2_m:
    case operand_format::implicit_m:
      J_UNREACHABLE();
    }
    // Abusing endianness and the fact that a 64-bit value will fit there anyway.
    *(i64_t*)ptr = operands->imm();
    return ptr + (width_mask_v & (u8_t)enc->imm_format);
  }
}
