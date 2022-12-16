// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/exprs/write_expr_impl.hpp"
#include "lisp/air/exprs/write_expr.hpp"
#include "lisp/air/exprs/exprs.hpp"
#include "mem/bump_pool.hpp"
#include "lisp/air/exprs/expr_init_data_map.hpp"

namespace j::lisp::air::exprs {
  // Ctor: ().
  // Exprs: nop, fn_rest_arg, and fn_sc_arg
  // 
  // Multiple result specs
  // =====================
  // Range    Range   - fn_rest_arg
  // Tagged   Nil     - nop        
  // Untagged ActRec  - fn_sc_arg  
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad) {
    return ::new (pool.allocate_aligned_zero(pad + 24U, 8U)) exprs::nop{
      {
        .type = t,
        .result = expr_init_data_v[(u8_t)t].result,
      },
    };
  }

  // Ctor: (In).
  // Exprs: do_until, ineg, lnot, to_bool, tag, untag, as_range, as_vec, make_vec, range_length, range_copy, and act_rec_parent
  // Arg #0: In arg0
  //         do_until: branch: Any Any
  //         ineg: value: NotRange I64
  //         lnot: value: Any Any
  //         to_bool: value: Any Any
  //         tag: untagged: Untagged Imm
  //         untag: tagged: Tagged Imm
  //         as_range: vec: NotRange Vec
  //         as_vec: range: Range Range
  //         make_vec: size: NotRange I64
  //         range_length: range: Any Vec|Range
  //         range_copy: range: Untagged|Range Vec|Range
  //         act_rec_parent: act_rec: Untagged ActRec
  // 
  // Multiple result specs
  // =====================
  // Any            Any     - do_until        
  // Any            Vec     - range_copy      
  // NotRange       I64     - ineg            
  // NotRange       Vec     - as_vec          
  // Range          Vec     - as_range        
  // Tagged         Imm     - tag             
  // Tagged         Bool    - lnot and to_bool
  // Untagged       I64     - range_length    
  // Untagged       ActRec  - act_rec_parent  
  // Untagged       Imm     - untag           
  // Untagged|Range Vec     - make_vec        
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 40U, 8U)) exprs::do_until{
      {
        .type = t,
        .num_inputs = 1, .max_inputs = 1,
        .result = expr_init_data_v[(u8_t)t].result,
      },
      // In #0 (Arg #0)
      .branch{arg0, expr_init_data_v[(u8_t)t].args[0]},
    };
    arg0->parent = result;
    arg0->parent_pos = 0;
    return result;
  }

  // Ctor: (Ins).
  // Exprs: progn, prog1, lor, land, vec, and vec_append
  // Arg #0: Ins arg0
  //         progn: forms: Any Any
  //         prog1: forms: Any Any
  //         lor: values: Any Any
  //         land: values: Any Any
  //         vec: elems: Tagged Imm
  //         vec_append: vecs: Untagged Vec
  // 
  // Multiple result specs
  // =====================
  // Any      Any  - progn, prog1, lor, and land
  // Untagged Vec  - vec and vec_append         
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init arg0)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 24U + arg0.num_allocated * 16U, 8U)) exprs::progn{
      {
        .type = t,
        .num_inputs = arg0.num_allocated, .max_inputs = arg0.num_initialized,
        .result = expr_init_data_v[(u8_t)t].result,
      },
    };

    // Ins #0 (Arg #0)
    write_dynamic_inputs(reinterpret_cast<exprs::input*>(result + 1), expr_init_data_v[(u8_t)t].args[0], result, 0, arg0);
    return result;
  }

  // Ctor: (Var).
  // Exprs: lex_rd and lex_rd_bound
  // Arg #0: Var var
  //         lex_rd: var
  //         lex_rd_bound: var
  // 
  // Has Data
  // ========
  // Data: 16 bytes in lex_rd, lex_rd_bound
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var)
  {
    return ::new (pool.allocate_aligned_zero(pad + 48U, 8U)) exprs::lex_rd{
      {
        .type = t,
        .result = val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}},
      },
      // Const #0 (Arg #0) Var
      .var = var,
    };
  }

  // Ctor: (Id).
  // Exprs: sym_val_rd
  // Arg #0: Id var
  //         sym_val_rd: var
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    id var)
  {
    return ::new (pool.allocate_aligned_zero(pad + 28U, 8U)) exprs::sym_val_rd{
      {
        .type = expr_type::sym_val_rd,
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
      // Const #0 (Arg #0) Id
      .var{var},
    };
  }

  // Ctor: (Imm).
  // Exprs: ld_const
  // Arg #0: Imm const_val
  //         ld_const: const_val
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    lisp_imm const_val)
  {
    return ::new (pool.allocate_aligned_zero(pad + 32U, 8U)) exprs::ld_const{
      {
        .type = expr_type::ld_const,
        .result = val_spec{const_val.type(), possible_reps[const_val.type()]},
      },
      // Const #0 (Arg #0) Imm
      .const_val{const_val},
    };
  }

  // Ctor: (U32).
  // Exprs: fn_arg
  // Arg #0: U32 index
  //         fn_arg: index
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    u32_t index)
  {
    return ::new (pool.allocate_aligned_zero(pad + 28U, 8U)) exprs::fn_arg{
      {
        .type = expr_type::fn_arg,
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
      // Const #0 (Arg #0) U32
      .index = index,
    };
  }

  // Ctor: (In, In).
  // Exprs: iadd, isub, imul, idiv, eq, neq, and range_get
  // Arg #0: In arg0
  //         iadd: lhs: NotRange I64
  //         isub: lhs: NotRange I64
  //         imul: lhs: Untagged I64
  //         idiv: lhs: Untagged I64
  //         eq: lhs: Any Any
  //         neq: lhs: Any Any
  //         range_get: range: Any Vec|Range
  // Arg #1: In arg1
  //         iadd: rhs: NotRange I64
  //         isub: rhs: NotRange I64
  //         imul: rhs: Untagged I64
  //         idiv: rhs: Untagged I64
  //         eq: rhs: Any Any
  //         neq: rhs: Any Any
  //         range_get: index: NotRange I64
  // 
  // Multiple result specs
  // =====================
  // NotRange I64   - iadd and isub
  // Tagged   Imm   - range_get    
  // Tagged   Bool  - eq and neq   
  // Untagged I64   - imul and idiv
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0, expr * arg1)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 56U, 8U)) exprs::iadd{
      {
        .type = t,
        .num_inputs = 2, .max_inputs = 2,
        .result = expr_init_data_v[(u8_t)t].result,
      },
      // In #0 (Arg #0)
      .lhs{arg0, expr_init_data_v[(u8_t)t].args[0]},
      // In #1 (Arg #1)
      .rhs{arg1, expr_init_data_v[(u8_t)t].args[1]},
    };
    arg0->parent = result;
    arg0->parent_pos = 0;
    arg1->parent = result;
    arg1->parent_pos = 1;
    return result;
  }

  // Ctor: (In, Ins).
  // Exprs: call
  // Arg #0: In closure
  //         call: closure: Untagged Closure
  // Arg #1: Ins args
  //         call: args: Tagged Imm
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    expr * closure, inputs_init args)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 40U + args.num_allocated * 16U, 8U)) exprs::call{
      {
        .type = expr_type::call,
        .num_inputs = (u8_t)(1 + args.num_allocated), .max_inputs = (u8_t)(1 + args.num_initialized),
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
      // In #0 (Arg #0)
      .closure{closure, val_spec{imm_type_mask{0x40}, rep_spec{0x2}}},
    };
    closure->parent = result;
    closure->parent_pos = 0;

    // Ins #1 (Arg #1)
    write_dynamic_inputs((reinterpret_cast<exprs::input*>(result + 1) + 1), val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}, result, 1, args);
    return result;
  }

  // Ctor: (In, ImmMask).
  // Exprs: type_assert and dyn_type_assert
  // Arg #0: In value
  //         type_assert: value: Tagged Imm
  //         dyn_type_assert: value: Tagged Imm
  // Arg #1: ImmMask types
  //         type_assert: types
  //         dyn_type_assert: types
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * value, imm_type_mask types)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 42U, 8U)) exprs::type_assert{
      {
        .type = t,
        .num_inputs = 1, .max_inputs = 1,
        .result = val_spec{types, rep_spec{0x1}},
      },
      // In #0 (Arg #0)
      .value{value, val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}},
      // Const #0 (Arg #1) ImmMask
      .types{types},
    };
    value->parent = result;
    value->parent_pos = 0;
    return result;
  }

  // Ctor: (Ins, Id).
  // Exprs: full_call
  // Arg #0: Ins args
  //         full_call: args: Tagged Imm
  // Arg #1: Id fn
  //         full_call: fn
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init args, id fn)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 28U + args.num_allocated * 16U, 8U)) exprs::full_call{
      {
        .type = expr_type::full_call,
        .num_inputs = args.num_allocated, .max_inputs = args.num_initialized,
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
    };

    // Ins #0 (Arg #0)
    write_dynamic_inputs(reinterpret_cast<exprs::input*>(result + 1), val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}, result, 0, args);

    // Const #0 (Arg #1) Id
    *j::add_bytes<id*>(result + 1, args.num_allocated * 16U) = fn;
    return result;
  }

  // Ctor: (Ins, Quasis).
  // Exprs: vec_build
  // Arg #0: Ins elems
  //         vec_build: elems: Any Any
  // Arg #1: Quasis quasi_info
  //         vec_build: quasi_info
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init elems, quasis_t quasi_info)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 40U + elems.num_allocated * 16U, 8U)) exprs::vec_build{
      {
        .type = expr_type::vec_build,
        .num_inputs = elems.num_allocated, .max_inputs = elems.num_initialized,
        .result = val_spec{imm_type_mask{0x100}, rep_spec{0x2}},
      },
    };

    // Ins #0 (Arg #0)
    write_dynamic_inputs(reinterpret_cast<exprs::input*>(result + 1), val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}}, result, 0, elems);

    // Const #0 (Arg #1) Quasis
    ::new (j::add_bytes<quasis_t*>(result + 1, elems.num_allocated * 16U)) quasis_t{quasi_info};

    // @AfterConstruct for vec_build
    auto in_ptr = result->inputs_begin();
    for (u32_t i = 0, max = result->num_inputs; i < max; ++i) {
     in_ptr[i].type = reinterpret_cast<vec_build*>(result)->quasi_info()[i].type == push ? val_imm_tag : val_vec_or_range;
    }
    return result;
  }

  // Ctor: (Ins, VarDefs).
  // Exprs: fn_body and let
  // Arg #0: Ins forms
  //         fn_body: forms: Any Any
  //         let: forms: Any Any
  // Arg #1: VarDefs vars
  //         fn_body: vars
  //         let: vars
  // 
  // Multiple result specs
  // =====================
  // Any    Any  - let    
  // Tagged Imm  - fn_body
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init forms, vars_init vars)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 24U + forms.num_allocated * 16U + air_var_defs_t::get_byte_size(vars.num_vars), 8U)) exprs::fn_body{
      {
        .type = t,
        .num_inputs = forms.num_allocated, .max_inputs = forms.num_initialized,
        .result = (t == expr_type::let ? val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}} : val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}),
      },
    };

    // Ins #0 (Arg #0)
    write_dynamic_inputs(reinterpret_cast<exprs::input*>(result + 1), val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}}, result, 0, forms);

    // Const #0 (Arg #1) VarDefs
    write_air_var_defs(j::add_bytes<air_var_defs_t*>(result + 1, forms.num_allocated * 16U), vars);
    return result;
  }

  // Ctor: (Var, In).
  // Exprs: lex_wr, lex_wr_bound, and act_rec_rd
  // Arg #0: Var var
  //         lex_wr: var
  //         lex_wr_bound: var
  //         act_rec_rd: var
  // Arg #1: In arg1
  //         lex_wr: value: Any Any
  //         lex_wr_bound: value: Any Any
  //         act_rec_rd: act_rec: Untagged ActRec
  // 
  // Has Data
  // ========
  // Data: 0 bytes in act_rec_rd
  // Data: 16 bytes in lex_wr, lex_wr_bound
  // 
  // Multiple result specs
  // =====================
  // Any    Any  - lex_wr and lex_wr_bound
  // Tagged Imm  - act_rec_rd             
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var, expr * arg1)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + (t == expr_type::act_rec_rd ? 48U : 64U), 8U)) exprs::lex_wr{
      {
        .type = t,
        .num_inputs = 1, .max_inputs = 1,
        .result = (t == expr_type::act_rec_rd ? val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}} : val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}}),
      },
      // In #0 (Arg #1)
      .value{arg1, (t == expr_type::act_rec_rd ? val_spec{imm_type_mask{0x400}, rep_spec{0x2}} : val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}})},
      // Const #0 (Arg #0) Var
      .var = var,
    };
    arg1->parent = result;
    arg1->parent_pos = 0;
    return result;
  }

  // Ctor: (Id, In).
  // Exprs: sym_val_wr
  // Arg #0: Id var
  //         sym_val_wr: var
  // Arg #1: In value
  //         sym_val_wr: value: Tagged Imm
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    id var, expr * value)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 44U, 8U)) exprs::sym_val_wr{
      {
        .type = expr_type::sym_val_wr,
        .num_inputs = 1, .max_inputs = 1,
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
      // In #0 (Arg #1)
      .value{value, val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}},
      // Const #0 (Arg #0) Id
      .var{var},
    };
    value->parent = result;
    value->parent_pos = 0;
    return result;
  }

  // Ctor: (U32, In).
  // Exprs: act_rec_alloc
  // Arg #0: U32 index
  //         act_rec_alloc: index
  // Arg #1: In parent
  //         act_rec_alloc: parent: NotRange Nil|ActRec
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    u32_t index, expr * parent)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 44U, 8U)) exprs::act_rec_alloc{
      {
        .type = expr_type::act_rec_alloc,
        .num_inputs = 1, .max_inputs = 1,
        .result = val_spec{imm_type_mask{0x400}, rep_spec{0x2}},
      },
      // In #0 (Arg #1)
      .parent{parent, val_spec{imm_type_mask{0x408}, rep_spec{0x3}}},
      // Const #0 (Arg #0) U32
      .index = index,
    };
    parent->parent = result;
    parent->parent_pos = 0;
    return result;
  }

  // Ctor: (In, In, In).
  // Exprs: b_if and range_set
  // Arg #0: In arg0
  //         b_if: cond: Any Any
  //         range_set: range: Any Vec|Range
  // Arg #1: In arg1
  //         b_if: then_branch: Any Any
  //         range_set: index: NotRange I64
  // Arg #2: In arg2
  //         b_if: else_branch: Any Any
  //         range_set: value: Tagged Imm
  // 
  // Multiple result specs
  // =====================
  // Any    Any  - b_if     
  // Tagged Imm  - range_set
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0, expr * arg1, expr * arg2)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 72U, 8U)) exprs::b_if{
      {
        .type = t,
        .num_inputs = 3, .max_inputs = 3,
        .result = (t == expr_type::b_if ? val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}} : val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}),
      },
      // In #0 (Arg #0)
      .cond{arg0, (t == expr_type::b_if ? val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}} : val_spec{imm_type_mask{0x900}, rep_spec{0x7}})},
      // In #1 (Arg #1)
      .then_branch{arg1, (t == expr_type::b_if ? val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}} : val_spec{imm_type_mask{0x1}, rep_spec{0x3}})},
      // In #2 (Arg #2)
      .else_branch{arg2, (t == expr_type::b_if ? val_spec{imm_type_mask{0xFFF}, rep_spec{0x7}} : val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}})},
    };
    arg0->parent = result;
    arg0->parent_pos = 0;
    arg1->parent = result;
    arg1->parent_pos = 1;
    arg2->parent = result;
    arg2->parent_pos = 2;
    return result;
  }

  // Ctor: (In, In, Closure).
  // Exprs: closure
  // Arg #0: In act_rec
  //         closure: act_rec: NotRange Nil|ActRec
  // Arg #1: In body
  //         closure: body: Tagged Imm
  // Arg #2: Closure closure
  //         closure: closure
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    expr * act_rec, expr * body, air_closure* closure)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 64U, 8U)) exprs::closure{
      {
        .type = expr_type::closure,
        .num_inputs = 2, .max_inputs = 2,
        .result = val_spec{imm_type_mask{0x40}, rep_spec{0x3}},
      },
      // In #0 (Arg #0)
      .act_rec{act_rec, val_spec{imm_type_mask{0x408}, rep_spec{0x3}}},
      // In #1 (Arg #1)
      .body{body, val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}},
      // Const #0 (Arg #2) Closure
      .closure = closure,
    };
    act_rec->parent = result;
    act_rec->parent_pos = 0;
    body->parent = result;
    body->parent_pos = 1;
    return result;
  }

  // Ctor: (Var, In, In).
  // Exprs: act_rec_wr
  // Arg #0: Var var
  //         act_rec_wr: var
  // Arg #1: In act_rec
  //         act_rec_wr: act_rec: Untagged ActRec
  // Arg #2: In value
  //         act_rec_wr: value: Tagged Imm
  J_A(RNN,NODISC) expr * write_expr(expr_type, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var, expr * act_rec, expr * value)
  {
    exprs::expr *result = ::new (pool.allocate_aligned_zero(pad + 64U, 8U)) exprs::act_rec_wr{
      {
        .type = expr_type::act_rec_wr,
        .num_inputs = 2, .max_inputs = 2,
        .result = val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}},
      },
      // In #0 (Arg #1)
      .act_rec{act_rec, val_spec{imm_type_mask{0x400}, rep_spec{0x2}}},
      // In #1 (Arg #2)
      .value{value, val_spec{imm_type_mask{0x3FF}, rep_spec{0x1}}},
      // Const #0 (Arg #0) Var
      .var = var,
    };
    act_rec->parent = result;
    act_rec->parent_pos = 0;
    value->parent = result;
    value->parent_pos = 1;
    return result;
  }
}
