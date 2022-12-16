#include "lisp/assembly/functions/func_info_builder.hpp"
#include "lisp/common/metadata_init.hpp"
#include "logging/global.hpp"

namespace j::lisp::assembly::inline functions {

  trap_info_rec::trap_info_rec(const trap_info & info) noexcept
    : info(info)
  { }

  func_info_builder::func_info_builder(j::mem::bump_pool * J_NOT_NULL pool, strings::const_string_view pkg_name, strings::const_string_view func_name)
    : temp_pool(pool),
      pkg_name(pool->write_string(pkg_name)),
      func_name(pool->write_string(func_name))
  {
  }

  void func_info_builder::push_metadata(u32_t offset, label_record * label, const metadata_init & ops) {
    u32_t ops_sz = ops.size();
    const u8_t lbl_sz = label ? label->label_size : 0U;
    const u32_t sz = align_up(align_up(sizeof(addr_info_rec) + lbl_sz, 8) + ops_sz, 8);
    addr_info_size += align_up(align_up(sizeof(addr_info) + lbl_sz, 8) + ops_sz, 8);
    temp_pool->align(8U);
    char * wr = (char*)temp_pool->allocate(sz);
    auto rec = ::new (wr) addr_info_rec{
      nullptr,
      {
        offset,
        (bool)ops_sz,
        (u8_t)lbl_sz,
      }
    };
    wr += sizeof(addr_info_rec);
    if (lbl_sz) {
      ::j::memcpy(wr, label + 1, lbl_sz);
      wr += lbl_sz;
    }
    wr = align_up(wr, 8);
    if (ops_sz) {
      ops.write(wr);
      wr = align_up(wr + ops_sz, 8);
    }

    if (last_addr) {
      last_addr->next = rec;
    } else {
      first_addr = rec;
    }
    last_addr = rec;
  }

  void func_info_builder::push_trap_info(const trap_info & info) {
    temp_pool->align(8U);
    trap_info_rec * rec = &temp_pool->emplace<trap_info_rec>(info);
    ++num_traps;
    if (last_trap) {
      last_trap->next = rec;
    } else {
      first_trap = rec;
    }
    last_trap = rec;
  }

  J_RETURNS_NONNULL func_info * func_info_builder::build(j::mem::bump_pool * J_NOT_NULL to, u32_t func_size) const {
    to->align(8U);
    func_info * info = &to->emplace_with_padding<func_info>(
      num_traps * sizeof(trap_info)
      + align_up(pkg_name.size() + 1U + func_name.size() + 1U, 8U)
      + addr_info_size
    );
    info->func_size = func_size;
    info->func_name_size = func_name.size() + 1U + pkg_name.size();
    info->num_traps = num_traps;
    info->addr_info_size = addr_info_size;

    char * wr = reinterpret_cast<char *>(info + 1);

    u16_t num_traps = 0U;
    for (auto trap = first_trap; trap; trap = trap->next, ++num_traps, wr += sizeof(trap_info)) {
      ::j::memcpy(wr, &trap->info, sizeof(trap_info));
    }
    J_ASSUME(num_traps == this->num_traps);

    ::j::memcpy(wr, pkg_name.begin(), pkg_name.size());
    wr += pkg_name.size();
    *wr++ = ':';
    ::j::memcpy(wr, func_name.begin(), func_name.size());
    wr += func_name.size();
    *wr++ = 0U;
    wr = align_up(wr, 8U);

    char * addr_begin = wr;
    for (auto addr = first_addr; addr; addr = addr->next) {
      u32_t sz = addr->info.addr_info_size();
      ::j::memcpy(wr, &addr->info, sz);
      wr = align_up(wr + sz, 8U);
    }
    J_ASSUME(wr - addr_begin == addr_info_size);
    return info;
  }
}
