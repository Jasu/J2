#pragma once

#include "logging/global.hpp"
#include "os/dwarf_forms.hpp"
#include "os/dwarf_tag.hpp"
#include "os/dwarf_reader_base.hpp"
#include "os/dwarf_attr.hpp"

namespace j::os::dwarf {
  namespace {
#define J_NAME_STR(N, V, ...) [V] = #N,
    J_A(ND) const char * const tag_names[]{
      J_WITH_DWARF_TAGS(J_NAME_STR)
    };

    J_A(ND) const char * const attr_names[]{
      J_WITH_DWARF_ATTRIBUTES(J_NAME_STR)
    };

    J_A(ND) const char * const form_names[]{
      J_WITH_DWARF_FORMS(J_NAME_STR)
    };
#undef J_NAME_STR

    J_A(AI,ND,NODISC) inline const char * tag_name(u32_t idx) noexcept {
      return (idx < J_ARRAY_SIZE(tag_names) && tag_names[idx]) ? tag_names[idx] : nullptr;
    }

    J_A(AI,ND,NODISC) inline const char * attr_name(u32_t idx) noexcept {
      return (idx < J_ARRAY_SIZE(attr_names) && attr_names[idx]) ? attr_names[idx] : nullptr;
    }

    J_A(AI,ND,NODISC) inline const char * form_name(u32_t idx) noexcept {
      return (idx < J_ARRAY_SIZE(form_names) && form_names[idx]) ? form_names[idx] : nullptr;
    }

    J_A(NI) void dump_name(u32_t idx, const char * J_AA(NN,NOALIAS) format, const char * J_AA(NOALIAS) name, u32_t max_len, u32_t prefix_len) noexcept {
      if (J_UNLIKELY(!name)) {
        J_DEBUG_INL("{#error}MISSINGNO[{:02X}]{/}{indent}", idx, max_len - 13 + prefix_len);
      } else {
        J_DEBUG_INL(format, name);
        J_DEBUG_INL("{indent}", max_len - j::strlen(name));
      }
    }

    J_A(NI) void dump_indent(i32_t depth) noexcept {
      J_DEBUG_INL("{indent}", depth << 1);
    }

    J_A(NI,MU) void dump_tag(i32_t depth, u32_t idx) noexcept {
      dump_indent(depth);
      dump_name(idx, "{#bright_green,bold}DW_TAG_{}{/}", tag_name(idx), 24 + 1, 7);
    }
    J_A(NI,MU) void dump_tag(i32_t depth, u32_t abbrev_code, u32_t idx, bool is_skip) noexcept {
      dump_indent(depth);
      J_DEBUG_INL("{#[bright_blue,bright_yellow]}{#bold}[{/}{:02}{#bold}]{/}{/} ", (i32_t)is_skip, abbrev_code);
      dump_name(idx, "{#bright_green,bold}DW_TAG_{}{/}", tag_name(idx), 24 + 1, 7);
    }
    J_A(NI,MU) void begin_dump_attr(i32_t depth, u32_t attr_idx, u32_t form_idx) noexcept {
      dump_indent(depth + 1);
      dump_name(attr_idx, "{#bright_magenta,bold}AT_{}{/}", attr_name(attr_idx), 15 + 1, 3);
      dump_name(form_idx, "{#bright_cyan}FORM_{}{/}", form_name(form_idx), 10, 5);
    }

    J_A(AI,ND,MU) inline void dump_attr(i32_t depth, u32_t attr_idx, u32_t form_idx) noexcept {
      begin_dump_attr(depth, attr_idx, form_idx);
      J_DEBUG("");
    }
  }
}
