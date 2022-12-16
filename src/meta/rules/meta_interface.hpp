#pragma once

#include "meta/cpp_codegen.hpp"

namespace j::meta {
  struct attr_value;
  struct module;
  struct struct_def;
  struct enum_def;
}

namespace j::meta::inline rules {
  struct te_type;
  struct nt_data_type;

  struct trs_config final {
    void define(module * J_NOT_NULL mod);
    enum_def * te_nt_data_source_enum;
    struct_def * te_nt_struct;
    struct_def * te_value_struct;
    struct_def * nt_field_struct;
    struct_def * nt_type_struct;
    attr_value nonterminal_data_types{};

    enum_def * test_case_matcher_type = nullptr;
    struct_def * test_suite = nullptr;
    struct_def * test_case = nullptr;
    struct_def * test_case_matcher = nullptr;
    struct_def * test_case_match_term = nullptr;
    struct_def * test_case_match_val_eq = nullptr;

    [[nodiscard]] attr_value nt_data_type(const struct nt_data_type * dt) const noexcept;
    [[nodiscard]] attr_value nt_data_field(const struct nt_data_type * dt, u32_t index) const noexcept;
    /**
     * GetStackTerm
     *
     * `[| s.stack[%(-StackOffset)].e |]`
     */
    codegen_template get_stack_term;

    /**
     * GetInputTerm
     *
     * `[| s.iter.e->input(s.iter.pos + %(1 - Offset)).expr |]`
     */
    codegen_template get_input_term;

    /**
     * GetStackNonTerminal
     *
     * `[| s.nt<%(Type.Name)>(%(StackOffset)) |]`
     */
    codegen_template get_stack_nt;
    /**
     * GetStackNonTerminalField
     *
     * `[| s.field<%(Type.Type)>(%(StackOffset), %(FieldOffset)) |]`
     */
    codegen_template get_stack_nt_field;

    // /**
    //  * `[| write_expr(%(Term.Name), s.pool, 0U%(Args.Map[", " + It.Value])) |]`
    //  */
    codegen_template construct_term;
  };

  struct parser_structs final {
    enum_def * reduction_type = nullptr;
    enum_def * rewrite_action = nullptr;
    struct_def * tables = nullptr;
    struct_def * goto_table = nullptr;
    struct_def * state = nullptr;
    struct_def * non_terminal = nullptr;
    struct_def * rewrite_step = nullptr;
    struct_def * reduce_infos = nullptr;
    struct_def * reduce_group = nullptr;
    struct_def * reduction = nullptr;
    struct_def * precompute = nullptr;
    struct_def * nt_field = nullptr;
    struct_def * nt_data_type = nullptr;
  };

  void register_parser_structs(module * J_NOT_NULL mod);
  attr_value get_parser_tables(module * J_NOT_NULL mod);
}
