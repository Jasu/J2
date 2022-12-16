#include "os/dwarf/unit_ctx.hpp"
#include "os/dwarf_unit.hpp"
#include "os/dwarf_tag.hpp"
#include "os/dwarf/die_ctx.hpp"
#include "os/dwarf_forms.hpp"
#include "os/dwarf/file_ctx.hpp"

namespace j::os::dwarf {
  namespace {
    struct J_AT(HIDDEN) abbrev_rec {
      u32_t abbrev_code;
      u32_t tag;
      bool has_children;
      u8_t num_ats = 0;
      abbrev_rec * next = nullptr;
    };

    struct J_AT(HIDDEN) at_rec {
      u32_t at;
      u32_t form;
      u64_t implicit_const;
      at_rec * next = nullptr;
    };
  }

  J_A(NODISC,RNN) unit_ctx * unit_ctx::open_unit(file_ctx * J_AA(NN,NOALIAS) ctx, const char * J_AA(NOALIAS) unit_begin) {
    if (!unit_begin) {
      unit_begin = ctx->debug_info.begin;
    }
    base_reader info_reader = ctx->debug_info.get_reader(unit_begin);

    format fmt = dwarf32;
    u32_t unit_length = info_reader.read_uword();
    if (unit_length == U32_MAX) {
      unit_length = info_reader.read_udword();
      fmt = dwarf64;
    }
    bool is_ok = unit_length;
    is_ok &= info_reader.read_uhalf() == 5;
    u8_t unit_type = info_reader.read_ubyte();
    is_ok &= unit_type == ut_compile;
    u8_t addr_size = info_reader.read_ubyte();
    is_ok &= addr_size == 4 || addr_size == 8;
    u32_t abbrev_offset = fmt == dwarf64 ? info_reader.read_udword() : info_reader.read_uword();

    switch (unit_type) {
    case ut_skeleton:
    case ut_split_compile:
      info_reader.consume(8);
      break;
    case ut_type:
    case ut_split_type:
      info_reader.consume(fmt == dwarf32 ? 12 : 16);
      break;
    default: break;
    }
    u8_t header_size = info_reader.offset - unit_begin;

    u32_t max_abbrev_code = 0, num_abbrevs = 0, num_ats = 0;
    abbrev_rec *first_abbrev = nullptr, **cur_abbrev = &first_abbrev;
    at_rec *first_at = nullptr, **cur_at = &first_at;

    if (is_ok) {
      base_reader abbrev_reader = ctx->debug_abbrev.get_reader(abbrev_offset);
      while(u32_t abbrev_code = abbrev_reader.read_abbrev_code()) {
        if (abbrev_code > max_abbrev_code) {
          max_abbrev_code = abbrev_code;
        }
        ++num_abbrevs;
        u32_t tag = abbrev_reader.read_uleb128();
        bool has_children = abbrev_reader.read_ubyte();
        *cur_abbrev = J_ALLOCA_NEW(abbrev_rec){abbrev_code, tag, has_children};
        for (u32_t at, form;;) {
          at = abbrev_reader.read_uleb128();
          form = abbrev_reader.read_uleb128();
          if (!at && !form) {
            break;
          }
          J_ASSERT(at && form);
          u64_t implicit_const = form == dw_form_impl_const ? abbrev_reader.read_uleb128() : 0;
          ++num_ats;
          ++(*cur_abbrev)->num_ats;
          *cur_at = J_ALLOCA_NEW(at_rec){at, form, implicit_const};
          cur_at = &(*cur_at)->next;
        }
        cur_abbrev = &(*cur_abbrev)->next;
      }
    }

    u32_t sz = sizeof(unit_ctx) + J_PTR_SZ * max_abbrev_code
      + sizeof(abbrev) * num_abbrevs
      + sizeof(abbrev_at) * num_ats;
    char * region = (char*)j::allocate_zero(sz);
    unit_ctx * result = (unit_ctx*)region;
    const abbrev ** abbrev_ptrs = result->abbrevs_array;
    const char * wr = region + sizeof(unit_ctx) + J_PTR_SZ * max_abbrev_code;
    result->fmt = fmt;
    result->addr_size = addr_size;
    result->header_size = header_size;
    result->unit_size = unit_length + (fmt == dwarf32 ? 4 : 12);
    result->unit_begin = unit_begin;
    result->strings = ctx->debug_str.begin;
    result->num_abbrevs = max_abbrev_code;
    for (;first_abbrev; first_abbrev = first_abbrev->next) {
      abbrev * abbr_wr = (abbrev*)wr;
      wr += sizeof(abbrev);
      abbrev_ptrs[first_abbrev->abbrev_code - 1] = abbr_wr;
      abbr_wr->tag = first_abbrev->tag;
      abbr_wr->has_children = first_abbrev->has_children;
      abbr_wr->num_ats = first_abbrev->num_ats;
      for (u32_t i = 0, max = first_abbrev->num_ats; i != max; ++i, wr += sizeof(abbrev_at), first_at = first_at->next) {
        *(abbrev_at*)wr = { first_at->at, first_at->form, first_at->implicit_const };
      }
    }
    J_ASSERT(!first_at && !first_abbrev);

    if (is_ok) {
      die_ctx die;
      die_reader rd;
      rd.open(&die, result);
      u32_t tag = rd.next();
      if (tag != dw_tag_compile_unit) {
        is_ok = false;
      } else {
        i32_t str_offsets_base = -1, addr_base = -1, rnglist_base = -1, loclist_base = -1;
        rd.read_attrs({{dw_at_stroff_b, &str_offsets_base}, {dw_at_addr_b, &addr_base}, {dw_at_rnglists_b, &rnglist_base}, {dw_at_loclists_b, &loclist_base}});
        if (str_offsets_base >= 0 && ctx->debug_str_offsets) {
          result->str_offsets.base = (const uword*)(ctx->debug_str_offsets.begin + str_offsets_base);
        }
        if (addr_base >= 0 && ctx->debug_addr) {
          result->addr_offsets.base = (const uword*)(ctx->debug_addr.begin + addr_base);
        }
        if (rnglist_base >= 0 && ctx->debug_rnglists) {
          result->rnglist_offsets.base = (const uword*)(ctx->debug_rnglists.begin + rnglist_base);
        }
        if (loclist_base >= 0 && ctx->debug_loclists) {
          result->loclist_offsets.base = (const uword*)(ctx->debug_loclists.begin + loclist_base);
        }
      }
    }

    if (!is_ok) {
      result->addr_size = 0;
    }

    return result;
  }
}
