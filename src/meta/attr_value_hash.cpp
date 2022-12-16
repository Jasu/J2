#include "attr_value_hash.hpp"
#include "bits/fixed_bitset_hash.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/rules/rule.hpp"
#include "hzd/crc32.hpp"

namespace j::meta {
  [[nodiscard]] u32_t attr_value_hash::operator()(const attr_value & v) const noexcept {
    switch (v.type) {
    case attr_type_none:
      return 1U;
    case attr_int:
    case attr_bool:
      return crc32(v.type + 7U, v.integer);
    case attr_term:
    case attr_term_member:
    case attr_value_type:
    case attr_attr_def:
    case attr_enum_def:
    case attr_struct_def:
    case attr_fn:
    case attr_nonterminal:
    case attr_gen_file_set:
      return crc32(992 + (u32_t)v.type, v.node->name.begin(), v.node->name.size());
    case attr_rule:
      return crc32(993, v.rule->precedence);
    case attr_enum:
      return crc32(1 | v.enum_val.value, v.enum_val.def->name.begin(), v.enum_val.def->name.size());
    case attr_val_type_set:
    case attr_term_set:
      return v.type ^ (bits::fixed_bitset_hash<4U>{})(v.node_set.bitmask);
    case attr_str:
      return crc32(181, v.string.begin(), v.string.size());
    case attr_id:
      return crc32(crc32(71, v.id.name.name.begin(), v.id.name.name.size()), v.id.name.pkg.begin(), v.id.name.pkg.size());
    case attr_map: {
      u32_t value = 121;
      for (auto & p : *v.map) {
        value ^= crc32(121, p.first.begin(), p.first.size());
        value ^= operator()(p.second);
      }
      return value;
    }
    case attr_tpl:
      return (tpl_hash{})(*v.tpl);
    case attr_struct: {
      u32_t val = crc32(19, v.struct_val.def->name.begin(), v.struct_val.def->name.size());
      for (auto & v : *v.struct_val.values) {
        val = (val << 1U) ^ operator()(v);
      }
      return val;
    }
    case attr_tuple: {
      u32_t val = 3821;
      for (auto & v : *v.tuple) {
        val = (val << 1U) ^ operator()(v);
      }
      return val;
    }
    }
  }
}
