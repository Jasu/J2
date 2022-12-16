#pragma once

#include "mem/bump_pool.hpp"
#include "lisp/assembly/functions/func_info.hpp"
#include "lisp/assembly/buffers/buffer_builder.hpp"

namespace j::mem {
  class bump_pool;
}
namespace j::lisp::inline common {
  struct metadata_init;
}

namespace j::lisp::assembly::inline functions {
  struct addr_info_rec final {
    addr_info_rec * next = nullptr;
    addr_info info;
  };

  struct trap_info_rec final {
    trap_info_rec(const trap_info & info) noexcept;

    trap_info_rec * next = nullptr;
    trap_info info;
  };

  struct func_info_builder final {
    func_info_builder(j::mem::bump_pool * J_NOT_NULL pool, strings::const_string_view pkg_name, strings::const_string_view func_name);

    void push_metadata(u32_t offset, label_record * label, const metadata_init & md);

    void push_trap_info(const trap_info & info);

    J_RETURNS_NONNULL func_info * build(j::mem::bump_pool * J_NOT_NULL target, u32_t func_size) const;
  private:
    j::mem::bump_pool * temp_pool = nullptr;
  public:
    u32_t addr_info_size = 0;
    u32_t num_traps = 0U;
  private:
    strings::const_string_view pkg_name;
    strings::const_string_view func_name;
    addr_info_rec * first_addr = nullptr;
    addr_info_rec * last_addr = nullptr;
    trap_info_rec * first_trap = nullptr;
    trap_info_rec * last_trap = nullptr;
  };
}
