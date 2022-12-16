#pragma once

#include "hzd/concepts.hpp"

namespace j::strings::inline formatting {
  class formatted_sink;
}

namespace j::meta::inline rules {
  struct rule;
  struct nonterminal;
}
namespace j::meta {
  struct node;
  struct fn;
  struct term;
  struct term_member;
  struct val_type;
  struct attr_def;
  struct enum_def;
  struct struct_def;
  struct generated_file_set;

  struct attr_value;
  struct module;

  enum J_A(FLAGS) node_type : u16_t {
    node_none             = 0b000000000U,
    node_term             = 0b000000001U,
    node_term_member      = 0b000000010U,
    node_value_type       = 0b000000100U,
    node_attr_def         = 0b000001000U,
    node_enum             = 0b000010000U,
    node_struct_def       = 0b000100000U,
    node_nonterminal      = 0b001000000U,
    node_gen_file_set     = 0b010000000U,
    node_fn               = 0b100000000U,
    node_any              = 0b111111111U,
  };

  template<typename T> concept Node = OneOf<term, term_member, val_type, attr_def, enum_def, struct_def, nonterminal, generated_file_set, fn, node>;

  enum J_A(FLAGS) val_region : u8_t {
    region_none          = 00000U,
    region_input         = 0b001U,
    region_const         = 0b010U,
    region_data          = 0b100U,

    region_ctor          = 0b011U,
    region_data_or_const = 0b110U,
    region_any           = 0b111U,
  };
}
