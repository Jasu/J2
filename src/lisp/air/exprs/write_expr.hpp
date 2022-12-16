#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "lisp/air/exprs/write_utils.hpp"
#include "lisp/air/air_var.hpp"
#include "lisp/air/exprs/quasiquote.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::air::inline functions { struct air_closure; }
namespace j::mem { class bump_pool; }

namespace j::lisp::air::exprs {
  /// Writes exprs constructible as ().
  /// 
  /// Expressions:
  ///   - `nop` - Nop
      /// Does nothing, evaluating to `nil`.
  ///   - `fn_rest_arg` - Rest argument
      /// Evaluates to the range of rest arguments.
  ///   - `fn_sc_arg` - Static chain arg
      /// Evaluates to the topmost activation record of the closure.

  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad);

  /// Writes exprs constructible as (In).
  /// 
  /// Expressions:
  ///   - `do_until` - Do Until
      /// Evaluate `branch` until it evaluates to a truthy value.
  ///   - `ineg` - Integer Negate
      /// Negate an integer.
  ///   - `lnot` - Logical Not
      /// Evaluates to `false` if `value` is truthy, `true` otherwise.
  ///   - `to_bool` - Cast to Bool
      /// Evaluates to `true` if `value` is truthy, `false` otherwise.
  ///   - `tag` - Tag
      /// Tags `untagged`.
  ///   - `untag` - Untag
      /// Untags `tagged`.
  ///   - `as_range` - Cast to Range
      /// Casts `vec` to range (i.e. a pair with `begin` pointer and size.
  ///   - `as_vec` - Cast to Vec
      /// Casts `range` to vec.
  ///   - `make_vec` - Make Vec
      /// Allocate a vec with `size` elements.
  ///   - `range_length` - Range Length
      /// Evaluates to the number of elements in a range or a vec.
  ///   - `range_copy` - Copy Range
      /// Copy `range` as a vec.
  ///   - `act_rec_parent` - Activation Record Parent
      /// Evaluates to the parent of `act_rec`.
  /// 
  /// @param arg0 In #0 renamed from:
  ///             vec in as_range
  ///             range in as_vec, range_length, and range_copy
  ///             act_rec in act_rec_parent
  ///             value in ineg, lnot, and to_bool
  ///             tagged in untag
  ///             branch in do_until
  ///             size in make_vec
  ///             untagged in tag
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0);

  /// Writes exprs constructible as (Ins).
  /// 
  /// Expressions:
  ///   - `progn` - Progn
      /// Evaluate `forms` in sequence, evaluating to the lresult of the last form.
  ///   - `prog1` - Prog1
      /// Evaluate `forms` in sequence, evaluating to the result of the first form.
  ///   - `lor` - Logical Or
      /// Short-circuiting OR for `values`.
  ///   - `land` - Logical And
      /// Short-circuiting AND for `values`.
  ///   - `vec` - Vec
      /// Initialize a vec with `elems` as values.
  ///   - `vec_append` - Vec Append
      /// Append `vecs...` into a single vec.
  /// 
  /// @param arg0 Ins #0 renamed from:
  ///             values in lor and land
  ///             elems in vec
  ///             forms in progn and prog1
  ///             vecs in vec_append
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init arg0);

  /// Writes exprs constructible as (Var).
  /// 
  /// Expressions:
  ///   - `lex_rd` - Read Var.
      /// Evaluates to the value of the lexical variable.
  ///   - `lex_rd_bound` - Read Bound Var
      /// Evaluates to the value of the lexical variable.
  /// 
  /// @param var Var #0
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var);

  /// Writes exprs constructible as (Id).
  /// 
  /// Expressions:
  ///   - `sym_val_rd` - Read Global
      /// Read a symbol value (global).
  /// 
  /// @param var Id #0
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    id var);

  /// Writes exprs constructible as (Imm).
  /// 
  /// Expressions:
  ///   - `ld_const` - Load constant
      /// Evaluates to a constant value.
  /// 
  /// @param const_val Imm #0
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    lisp_imm const_val);

  /// Writes exprs constructible as (U32).
  /// 
  /// Expressions:
  ///   - `fn_arg` - Argument
      /// Evaluates to the argument specified by `index`.
  /// 
  /// @param index U32 #0
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    u32_t index);

  /// Writes exprs constructible as (In, In).
  /// 
  /// Expressions:
  ///   - `iadd` - Integer Add
      /// Add integers given as `values`.
  ///   - `isub` - Integer Subtract
      /// Subtract integers given as `values`.
  ///   - `imul` - Integer Multiply
      /// Multiply integers given as `values`.
  ///   - `idiv` - Integer Divide
      /// Divide the first value by the rest of the values.)
  ///   - `eq` - Eq
      /// Identity equality.
  ///   - `neq` - Not Eq
      /// Identity inequality.
  ///   - `range_get` - Range Get
      /// Evaluates to the `index`:th element in a range or vec.
  /// 
  /// @param arg0 In #0 renamed from:
  ///             range in range_get
  ///             lhs in iadd, isub, imul, idiv, eq, and neq
  /// @param arg1 In #1 renamed from:
  ///             index in range_get
  ///             rhs in iadd, isub, imul, idiv, eq, and neq
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0, expr * arg1);

  /// Writes exprs constructible as (In, Ins).
  /// 
  /// Expressions:
  ///   - `call` - Call Closure
      /// Calls `closure` with `args`.
  /// 
  /// @param closure In #0
  /// @param args    Ins #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * closure, inputs_init args);

  /// Writes exprs constructible as (In, ImmMask).
  /// 
  /// Expressions:
  ///   - `type_assert` - Type assert
      /// Throws if `value` is not in `types`.
  ///   - `dyn_type_assert` - Dyn Type assert
      /// Throws if `value` is not in `types`.
  /// 
  /// @param value In #0
  /// @param types ImmMask #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * value, imm_type_mask types);

  /// Writes exprs constructible as (Ins, Id).
  /// 
  /// Expressions:
  ///   - `full_call` - Full Call
      /// Calls a free function `fn` with `args`.
  /// 
  /// @param args Ins #0
  /// @param fn   Id #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init args, id fn);

  /// Writes exprs constructible as (Ins, Quasis).
  /// 
  /// Expressions:
  ///   - `vec_build` - Quasi
      /// Build a vec with `elems` as values, possibly splatted.)
  /// 
  /// @param elems      Ins #0
  /// @param quasi_info Quasis #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init elems, quasis_t quasi_info);

  /// Writes exprs constructible as (Ins, VarDefs).
  /// 
  /// Expressions:
  ///   - `fn_body` - Fn body
      /// Let-like structure binding function arguments.
  ///   - `let` - Let
      /// Evaluate `forms` with `vars` in lexical environment.
  /// 
  /// @param forms Ins #0
  /// @param vars  VarDefs #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    inputs_init forms, vars_init vars);

  /// Writes exprs constructible as (Var, In).
  /// 
  /// Expressions:
  ///   - `lex_wr` - Write Var
      /// Evaluates to `value`.
  ///   - `lex_wr_bound` - Write Bound Var
      /// Evaluates to `value`.
  ///   - `act_rec_rd` - Read Activation Record
      /// Read a variable from an activation record.
  /// 
  /// @param var  Var #0
  /// @param arg1 In #1 renamed from:
  ///             act_rec in act_rec_rd
  ///             value in lex_wr and lex_wr_bound
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var, expr * arg1);

  /// Writes exprs constructible as (Id, In).
  /// 
  /// Expressions:
  ///   - `sym_val_wr` - Write Global
      /// Write a symbol value (global).
  /// 
  /// @param var   Id #0
  /// @param value In #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    id var, expr * value);

  /// Writes exprs constructible as (U32, In).
  /// 
  /// Expressions:
  ///   - `act_rec_alloc` - Make Activation Record
      /// Allocate an activation record.
  /// 
  /// @param index  U32 #0
  /// @param parent In #1
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    u32_t index, expr * parent);

  /// Writes exprs constructible as (In, In, In).
  /// 
  /// Expressions:
  ///   - `b_if` - If
      /// Evaluate `then_branch` if `cond` is truthy, otherwise evaluate `false_branch`.
  ///   - `range_set` - Range Set
      /// Writes `value` to a range or a vec, evaluating to `value`.
  /// 
  /// @param arg0 In #0 renamed from:
  ///             range in range_set
  ///             cond in b_if
  /// @param arg1 In #1 renamed from:
  ///             index in range_set
  ///             then_branch in b_if
  /// @param arg2 In #2 renamed from:
  ///             value in range_set
  ///             else_branch in b_if
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * arg0, expr * arg1, expr * arg2);

  /// Writes exprs constructible as (In, In, Closure).
  /// 
  /// Expressions:
  ///   - `closure` - Construct Closure
      /// Evaluates to `closure` bound with `act_rec`.
  /// 
  /// @param act_rec In #0
  /// @param body    In #1
  /// @param closure Closure #2
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    expr * act_rec, expr * body, air_closure* closure);

  /// Writes exprs constructible as (Var, In, In).
  /// 
  /// Expressions:
  ///   - `act_rec_wr` - Write Activation Record
      /// Write a variable in an activation record.
  /// 
  /// @param var     Var #0
  /// @param act_rec In #1
  /// @param value   In #2
  J_A(RNN,NODISC) expr * write_expr(expr_type t, j::mem::bump_pool & pool, u32_t pad,
                                    air_var* var, expr * act_rec, expr * value);
}
