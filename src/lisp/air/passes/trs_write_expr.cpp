// GENERATED FILE - EDITS WILL BE LOST.
#include "trs_write_expr.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/air/exprs/expr_init_data_map.hpp"
#include "lisp/air/exprs/write_expr_impl.hpp"
#include "lisp/air/exprs/exprs.hpp"

namespace j::lisp::air::inline passes {
  using namespace j::lisp::air::exprs;
  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input), 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    air_closure* act_rec){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 8, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    *(air_closure**)in = act_rec;

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    air_var* var){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 8, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    *(air_var**)in = var;

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    id arg0){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 4, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    ::new (in) id{arg0};

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    lisp_imm const_val){
    const auto & d = expr_init_data_v[(u8_t)type];
    auto result = d.result;
    if (type == expr_type::ld_const) {
      result = {const_val.type(), possible_reps[const_val.type()]};
    }
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 8, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    ::new (in) lisp_imm{const_val};

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    imm_type_mask value){
    const auto & d = expr_init_data_v[(u8_t)type];
    auto result = d.result;
    if (type == expr_type::type_assert) {
      result.types = value;
    }
    if (type == expr_type::dyn_type_assert) {
      result.types = value;
    }
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 2, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    ::new (in) imm_type_mask{value};

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    u32_t index){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 4, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    *(u32_t*)in = index;

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    quasis_t elems){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + 16, 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    ::new (in) quasis_t{elems};

    // @AfterConstruct for vec_build
    auto in_ptr = e->inputs_begin();
    for (u32_t i = 0, max = e->num_inputs; i < max; ++i) {
     in_ptr[i].type = reinterpret_cast<vec_build*>(e)->quasi_info()[i].type == push ? val_imm_tag : val_vec_or_range;
    }

    return e;
  }

  J_A(RNN,NODISC) expr * trs_construct(expr_type type, j::mem::bump_pool & pool, u8_t num_inputs,
    vars_init forms){
    const auto & d = expr_init_data_v[(u8_t)type];
    expr * e = ::new (pool.allocate_aligned_zero(sizeof(expr) + num_inputs * sizeof(input) + air_var_defs_t::get_byte_size(forms.num_vars), 8U)) exprs::expr{
      .type = type,
      .num_inputs = num_inputs, .max_inputs = num_inputs,
      .result = d.result,
    };

    input * in = e->inputs_begin();
    for (u8_t i = 0; i < num_inputs; ++i, ++in) {
      in->type = d.arg(i);
    }

    write_air_var_defs(in, forms);

    return e;
  }
}
