#include "lisp/assembly/functions/function_builder.hpp"
#include "containers/hash_map.hpp"
#include "lisp/compilation/const_table.hpp"
#include "lisp/common/metadata_init.hpp"

J_DEFINE_EXTERN_HASH_MAP(j::mem::const_memory_region, u32_t, HASH(j::mem::memory_region_hash), KEYEQ(j::mem::memory_region_equals));

namespace j::lisp::assembly::inline functions {
  function_builder::function_builder(j::mem::bump_pool * J_NOT_NULL temp_pool, strings::const_string_view pkg_name, strings::const_string_view name, compilation::const_table & tbl)
    : code_builder(temp_pool),
      constants_builder(temp_pool),
      func_info_builder(temp_pool, pkg_name, name)
  {
    tbl.add_to(constants_builder);
  }

  [[nodiscard]] function function_builder::build(j::mem::bump_pool * J_NOT_NULL func_info_pool)
  {
    J_ASSERT(!cur_label);
    func_info * info = func_info_builder.build(func_info_pool, code_builder.offset());
    return {
      code_builder.build(),
      constants_builder.build(),
      info,
    };
  }

  u32_t function_builder::add_constant(j::mem::const_memory_region buf) {
    J_ASSERT(buf);
    auto it = constant_map.find(buf);
    if (it != constant_map.end()) {
      return it->second;
    }

    u32_t offset = constants_builder.offset();
    // This region is on heap and can actually be stored in the hash table.
    void * real_region = constants_builder.append(buf);
    constant_map.emplace(j::mem::const_memory_region(real_region, buf.size()), offset);
    return offset;
  }

  void function_builder::append_backpatch_rel(label_record * J_NOT_NULL label, reloc_field_options opts, i64_t addend) {
    code_builder.append_backpatch_rel(label, opts, addend, J_PTR_SZ);
  }

  void function_builder::push_addr_info(const metadata_init & ops) {
    if (cur_label) {
      code_builder.push_label(cur_label);
    }
    func_info_builder.push_metadata(code_builder.offset(), cur_label, ops);
    cur_label = nullptr;
  }

  void function_builder::push_type_check_info(imm_type_mask expected_type, reg r, id var_name, sources::source_location loc) {
    func_info_builder.push_trap_info({
        .offset = code_builder.offset(),
        .type = trap_type::type_check,
        .reg = r,
        .expected_type = expected_type,
        .var_name = var_name,
        .loc = loc,
      });
  }

  void function_builder::begin_instruction() {
    if (cur_label) {
      code_builder.push_label(cur_label);
      func_info_builder.push_metadata(code_builder.offset(), cur_label, {});
      cur_label = nullptr;
    }
  }
}
