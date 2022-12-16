#pragma once

#include "os/dwarf_common.hpp"
#include "os/dwarf_reader_base.hpp"
#include "containers/span.hpp"

namespace j::os::dwarf {
  struct file_ctx;

  struct abbrev_at final {
    u32_t at;
    u32_t form;
    u64_t implicit_const;
  };

  struct alignas(u64_t) abbrev final {
    u32_t tag;
    bool has_children;
    u8_t num_ats;
    abbrev_at ats_array[0];

    J_A(AI,HIDDEN,NODISC) inline span<const abbrev_at> ats() const noexcept {
      return {ats_array, num_ats};
    }
  };

  struct offset_table final {
    const uword * base = nullptr;

    J_A(AI,HIDDEN,NODISC,ND) inline offset_table() noexcept = default;
    J_A(AI,HIDDEN,NODISC,ND) inline offset_table(const void * base) noexcept : base((const uword*)base) { }

    J_A(AI,HIDDEN,NODISC) inline u32_t get_offset(format fmt, u32_t idx) const noexcept {
      J_ASSERT_NOT_NULL(base);
      return base[idx << (u8_t)fmt];
    }
  };

  struct unit_ctx final {
    format fmt;
    u8_t addr_size;
    u8_t header_size;
    u32_t unit_size;
    const char * unit_begin;
    const char * strings;
    offset_table str_offsets;
    offset_table addr_offsets;
    offset_table rnglist_offsets;
    offset_table loclist_offsets;
    u32_t num_abbrevs;
    const abbrev * abbrevs_array[0];

    J_A(AI,HIDDEN,NODISC,RNN) const char * unit_end() const noexcept {
      return unit_size + unit_begin;
    }

    J_A(AI,HIDDEN,NODISC,RNN) const char * data_begin() const noexcept {
      return unit_begin + header_size;
    }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_info_reader() {
      J_ASSERT(addr_size);
      return {unit_begin + header_size, unit_begin + unit_size};
    }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_info_reader(u32_t offset) {
      J_ASSERT(addr_size && offset >= header_size);
      return {unit_begin + offset, unit_begin + unit_size};
    }

    J_A(AI,NODISC,HIDDEN,ND) inline base_reader get_info_reader(const char * J_AA(NN) offset) {
      J_ASSERT(addr_size && offset >= unit_begin + header_size);
      return {offset, unit_begin + unit_size};
    }

    J_A(AI,HIDDEN,NODISC) inline explicit operator bool() const noexcept {
      return addr_size;
    }

    J_A(AI,HIDDEN,NODISC) inline bool operator!() const noexcept {
      return !addr_size;
    }

    J_A(NODISC,RNN) static unit_ctx * open_unit(file_ctx * J_AA(NN,NOALIAS) ctx, const char * J_AA(NOALIAS) unit_begin);

    J_A(AI,HIDDEN,ND) static void close_unit(unit_ctx * J_AA(NN) ctx) noexcept {
      j::free(ctx);
    }

    J_A(AI,NODISC,RNN) inline const char * cu_offset_to_ptr(u32_t offset) const noexcept { return unit_begin + offset; }
    J_A(AI,NODISC) inline u32_t ptr_to_cu_offset(const char * J_AA(NN) ptr) const noexcept { return ptr - unit_begin; }

    J_A(AI,NODISC,RNN) inline const char * str(u32_t idx) const noexcept { return strings + str_offsets.get_offset(fmt, idx); }
    J_A(AI,NODISC) inline u32_t str_offset(u32_t idx) const noexcept { return str_offsets.get_offset(fmt, idx); }
    J_A(AI,NODISC) inline u32_t addr_offset(u32_t idx) const noexcept { return addr_offsets.get_offset(fmt, idx); }
    J_A(AI,NODISC) inline u32_t rnglist_offset(u32_t idx) const noexcept { return rnglist_offsets.get_offset(fmt, idx); }
    J_A(AI,NODISC) inline u32_t loclist_offset(u32_t idx) const noexcept { return loclist_offsets.get_offset(fmt, idx); }

    J_A(AI,HIDDEN,NODISC) inline const abbrev & get_abbrev(u32_t idx) const noexcept {
      J_ASSERT(idx && idx <= num_abbrevs && abbrevs_array[idx - 1]);
      return *abbrevs_array[idx - 1];
    }
  };
}
