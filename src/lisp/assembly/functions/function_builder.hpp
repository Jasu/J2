#pragma once

#include "func_info_builder.hpp"
#include "function.hpp"
#include "lisp/assembly/buffers/buffer_builder.hpp"
#include "lisp/assembly/linking/reloc.hpp"
#include "containers/hash_map_fwd.hpp"
#include "mem/memory_region.hpp"
#include "mem/memory_region_hash.hpp"

J_DECLARE_EXTERN_HASH_MAP(j::mem::const_memory_region, u32_t, HASH(j::mem::memory_region_hash), KEYEQ(j::mem::memory_region_equals));

namespace j::lisp::compilation {
  struct const_table;
}
namespace j::lisp::assembly::inline functions {
  class function_builder final {
  public:
    explicit function_builder(j::mem::bump_pool * J_NOT_NULL temp_pool, strings::const_string_view pkg_name, strings::const_string_view name, compilation::const_table & tbl);

    /// Make the current code in the builder into a buffer.
    ///
    /// \remarks Invalidates the builder.
    [[nodiscard]] function build(j::mem::bump_pool * J_NOT_NULL func_info_pool);

    /// Allocate an empty label.
    J_INLINE_GETTER_NONNULL label_record * allocate_label(const strings::const_string_view & name) {
      return code_builder.allocate_label(name);
    }

    /// Binds label to the current address.
    void push_label(label_record * J_NOT_NULL label) {
      J_ASSERT(!cur_label);
      cur_label = label;
    }

    /// Adds comment and/or operand info to the next instruction.
    void push_addr_info(const metadata_init & ops);

    /// Adds type check trap metadata to the next instruction.
    void push_type_check_info(imm_type_mask expected_type, reg r = {}, id var_name = {}, sources::source_location loc = {});

    u32_t add_constant(j::mem::const_memory_region buf);

    template<Integral T>
    J_INLINE_GETTER u32_t add_constant(T value) {
      return add_constant(j::mem::const_memory_region(&value, sizeof(value)));
    }

    J_INLINE_GETTER u32_t add_constant(const void * value) {
      return add_constant(j::mem::const_memory_region(&value, sizeof(void *)));
    }

    /// Reserves space for a backpatch.
    ///
    /// \param label Label to fill the backpatch slot with.
    J_ALWAYS_INLINE void append_backpatch_rel8(label_record * J_NOT_NULL label, i8_t addend) {
      append_backpatch_rel(label, {.type = reloc_type::rel8, .shift = 0}, addend);
    }

    /// Reserves space for a backpatch.
    ///
    /// \param label Label to fill the backpatch slot with.
    J_ALWAYS_INLINE void append_backpatch_rel16(label_record * J_NOT_NULL label, i16_t addend) {
      append_backpatch_rel(label, {.type = reloc_type::rel16, .shift = 0}, addend);
    }

    /// Reserves space for a backpatch.
    ///
    /// \param label Label to fill the backpatch slot with.
    J_ALWAYS_INLINE void append_backpatch_rel32(label_record * J_NOT_NULL label, i32_t addend) {
      append_backpatch_rel(label, {.type = reloc_type::rel32, .shift = 0}, addend);
    }

    void append_backpatch_rel(label_record * J_NOT_NULL label, reloc_field_options opts, i64_t addend);

    void begin_instruction();

    buffer_builder code_builder;
    buffer_builder constants_builder;
    func_info_builder func_info_builder;
    label_record * cur_label = nullptr;

    hash_map<j::mem::const_memory_region, u32_t, j::mem::memory_region_hash, j::mem::memory_region_equals> constant_map;
  };
}
