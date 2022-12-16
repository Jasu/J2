#include "os/dwarf/die_ctx.hpp"
#include "os/dwarf/dump.hpp"
#include "os/dwarf/die_model.hpp"

namespace j::os::dwarf {
  namespace {
    J_A(NI,COLD,NORET) void throw_invalid_form(u32_t form, const char * J_AA(NN) msg) {
        const char * name = form_name(form);
        if (!name) {
          J_THROW("Unknown DWARF form {#bright_yellow,bold}[{:02X}]{/}.", form);
        }
        J_THROW("{}: {#bright_red,bold}{}{/}", msg, name);
    }

    J_A(NI,MU) void skip_attr_value(u32_t form, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      switch (form) {
      case dw_form_addr:
        ir.consume(unit->addr_size);
        return;
      case dw_form_sec_off:
        ir.consume(unit->fmt == dwarf32 ? 4 : 8);
        return;
      case dw_form_block:
      case dw_form_exprloc:
        ir.consume(ir.read_uleb128());
        return;
      case dw_form_block1:
        ir.consume(ir.read_ubyte());
        return;
      case dw_form_block2:
        ir.consume(ir.read_uhalf());
        return;
      case dw_form_block4:
        ir.consume(ir.read_uword());
        return;
      case dw_form_ref_addr:
      case dw_form_ref_udata:
      case dw_form_sdata:
      case dw_form_udata:
      case dw_form_strp:
      case dw_form_strx:
      case dw_form_line_strp:
      case dw_form_addrx:
      case dw_form_loclistx:
      case dw_form_rnglistx:
        ir.skip_leb128();
        return;
      case dw_form_data1:
      case dw_form_ref1:
      case dw_form_flag:
      case dw_form_addrx1:
      case dw_form_strx1:
        ir.consume(1);
        return;
      case dw_form_ref2:
      case dw_form_data2:
      case dw_form_addrx2:
      case dw_form_strx2:
        ir.consume(2);
        return;
      case dw_form_strx3:
      case dw_form_addrx3:
        ir.consume(3);
        return;
      case dw_form_ref4:
      case dw_form_data4:
      case dw_form_addrx4:
      case dw_form_strx4:
        ir.consume(4);
        return;
      case dw_form_ref8:
      case dw_form_data8:
      case dw_form_ref_sig8:
        ir.consume(8);
        return;
      case dw_form_data16:
        [[unlikely]];
        ir.consume(16);
        return;
      case dw_form_string:
        ir.read_str();
        return;
      case dw_form_impl_const:
      case dw_form_flag_set:
        return;
      case dw_form_indirect:
      case dw_form_strp_sup:
      case dw_form_ref_sup4:
      case dw_form_ref_sup8:
      default:
        [[unlikely]];
        throw_invalid_form(form, "Unsupported DWARF form in skip");
      }
    }

    J_A(NI,MU,NODISC) i64_t read_attr_signed(const abbrev_at & at, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      u32_t form = at.form;
      if (form == dw_form_addr) {
        form = unit->addr_size == 8 ? dw_form_data8 : dw_form_data4;
      } else if (form == dw_form_sec_off) {
        form = unit->fmt == dwarf32 ? dw_form_data4 : dw_form_data8;
      }
      switch (form) {
      case dw_form_addr:
      case dw_form_sec_off:
        J_UNREACHABLE();
      case dw_form_impl_const:
        return at.implicit_const;
      case dw_form_data1:
      case dw_form_flag:
        return ir.read_sbyte();
      case dw_form_data2:
        return ir.read_shalf();
      case dw_form_data4:
        return ir.read_sword();
      case dw_form_data8:
        return ir.read_sdword();
      case dw_form_data16: {
         i64_t result = ir.read_sdword();
         J_REQUIRE(ir.read_sdword() == (result < 0 ? -1 : 0), "16-byte signed too large.");
         return result;
      }
      case dw_form_flag_set:
        return 1;
      default:
        [[unlikely]];
        throw_invalid_form(form, "DWARF form not readable as signed");
      }
    }

    J_A(NI,MU,NODISC) u64_t read_attr_unsigned(const abbrev_at & at, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      u32_t form = at.form;
      if (form == dw_form_addr) {
        form = unit->addr_size == 8 ? dw_form_data8 : dw_form_data4;
      } else if (form == dw_form_sec_off) {
        form = unit->fmt == dwarf32 ? dw_form_data4 : dw_form_data8;
      }
      switch (form) {
      case dw_form_addr:
      case dw_form_sec_off:
        J_UNREACHABLE();
      case dw_form_impl_const:
        return at.implicit_const;
      case dw_form_addrx:
      case dw_form_strx:
        return ir.read_uleb128();
      case dw_form_data1:
      case dw_form_addrx1:
      case dw_form_strx1:
      case dw_form_flag:
        return ir.read_ubyte();
      case dw_form_data2:
      case dw_form_addrx2:
      case dw_form_strx2:
        return ir.read_uhalf();
      case dw_form_addrx3:
      case dw_form_strx3:
        return ir.read_uthreequarters();
      case dw_form_data4:
      case dw_form_addrx4:
      case dw_form_strx4:
        return ir.read_uword();
      case dw_form_data8:
        return ir.read_udword();
      case dw_form_data16: {
         u64_t result = ir.read_udword();
         J_REQUIRE(!ir.read_udword(), "16-byte unsigned too large.");
         return result;
      }
      case dw_form_flag_set:
        return 1;
      default:
        [[unlikely]];
        throw_invalid_form(form, "DWARF form not readable as unsigned");
      }
    }

    const char * read_str(u32_t form, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      u32_t idx;
      switch (form) {
      case dw_form_string:
        return ir.read_str();
      case dw_form_strp:
        return unit->strings + ir.read_uleb128();
      case dw_form_strx1:
        idx = ir.read_ubyte();
        break;
      case dw_form_strx2:
        idx = ir.read_uhalf();
        break;
      case dw_form_strx3:
        idx = ir.read_uthreequarters();
        break;
      case dw_form_strx4:
        idx = ir.read_uword();
        break;
      case dw_form_strx:
        idx = ir.read_uleb128();
        break;
      default:
        return nullptr;
      }
      return unit->str(idx);
    }

    J_A(NI,MU,NODISC) const char * read_attr_ref(u32_t form, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      u32_t offset;
      switch (form) {
      case dw_form_ref_udata:
        offset = ir.read_uleb128();
        break;
      case dw_form_ref1:
        offset = ir.read_ubyte();
        break;
      case dw_form_ref2:
        offset = ir.read_uhalf();
        break;
      case dw_form_ref4:
        offset = ir.read_uword();
        break;
      case dw_form_ref8:
        offset = ir.read_udword();
        break;
      default:
        [[unlikely]];
        throw_invalid_form(form, "DWARF form not readable as ref");
      }
      return unit->unit_begin + offset;
    }

    void read_attr(const abbrev_at & at, const attr_spec * J_AA(NN) spec, u32_t count, base_reader & J_AA(NOALIAS) ir, unit_ctx * J_AA(NN,NOALIAS) unit) {
      for (u32_t i = 0; i < count; ++i) {
        if (spec[i].attr != at.at) {
          continue;
        }
        switch (spec[i].type) {
        case attr_type_str:
          *spec[i].ptr.str = read_str(at.form, ir, unit);
          return;
        case attr_type_i32:
          switch (at.form) {
          case dw_form_ref1:
          case dw_form_ref2:
          case dw_form_ref4:
          case dw_form_ref8:
          case dw_form_ref_udata:
            break;
          default:
            *spec[i].ptr.i32 = read_attr_signed(at, ir, unit);
            return;
          }
          break;
        case attr_type_u32:
          *spec[i].ptr.u32 = read_attr_unsigned(at, ir, unit);
          return;
        case attr_type_bool:
          *spec[i].ptr.boolean = read_attr_unsigned(at, ir, unit);
          return;
        case attr_type_ref:
          *spec[i].ptr.ref = dwarf_ref(read_attr_ref(at.form, ir, unit));
          return;
        }
      }
      skip_attr_value(at.form, ir, unit);
    }

    J_A(NODISC) u32_t die_depth(const die_ctx * parent) noexcept {
      u32_t result = 0;
      for (; parent; parent = parent->parent, ++result) { }
      return result;
    }
  }

  void die_ctx::skip_attrs(base_reader & r) {
    for (auto & at : abbrev->ats()) {
      skip_attr_value(at.form, r, unit);
    }
  }

  void die_ctx::read_attrs(base_reader & r, const attr_spec * J_AA(NN) spec, u32_t count) {
    // J_DEBUG("Reading attrs");
    // dump_tag(0, abbrev->tag);
    // J_DEBUG("Req:");
    // for (u32_t i = 0; i!= count; ++i) {
    //   dump_attr(0, spec[i].attr, 0);
    // }
    for (auto & at : abbrev->ats()) {
      // dump_attr(0, at.at, at.form);
      read_attr(at, spec, count, r, unit);
    }
  }

  void die_reader::open(die_ctx * J_AA(NN) target, unit_ctx * J_AA(NN) u) {
    open(target, u, u->data_begin());
  }

  void die_reader::open(die_ctx * J_AA(NN) target, unit_ctx * J_AA(NN) u, const char * J_AA(NN) at) {
    die = target;
    target->parent = nullptr;
    target->unit = u;
    target->abbrev = nullptr;
    unit = u;
    ir = base_reader(at, u->unit_end());
  }


 void die_reader::skip_children() {
    J_ASSERT_NOT_NULL(die);
    if (!die->abbrev->has_children) {
      return;
    }
    die_ctx ch_ctx;
    enter_children(&ch_ctx);
    while (next()) {
      ch_ctx.skip_attrs(ir);
      skip_children();
    }
  }

  void die_reader::enter_children(die_ctx * J_AA(NN) target) {
    J_ASSERT(die && die->abbrev->has_children);
    target->parent = die;
    target->unit = unit;
    target->die_begin = nullptr;
    die = target;
  }

  J_A(NODISC) u32_t die_reader::next() {
    J_ASSERT_NOT_NULL(die);
    die->die_begin = ir.offset;
    u32_t abbrev_code = ir.read_uleb128();
    if (!abbrev_code) {
      die = die->parent;
      return 0;
    }
    die->abbrev = &unit->get_abbrev(abbrev_code);
    return die->abbrev->tag;
  }
}
