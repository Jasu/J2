#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "lisp/air/air_var.hpp"
#include "lisp/air/exprs/expr.hpp"
#include "lisp/air/exprs/quasiquote.hpp"
#include "lisp/values/lisp_imm.hpp"

namespace j::lisp::air::inline functions { struct air_closure; }

namespace j::lisp::air::exprs {

  /// `nop` - Nop
  /// 
  /// Does nothing, evaluating to `nil`.
  /// 
  /// `nop`s are not moved or optimized away, as they are usually used for placeholders for transformations.
  struct nop final : expr { };

  /// `ld_const` - Load constant
  /// 
  /// Evaluates to a constant value.
  struct ld_const final : expr {
    /// Const Imm const_val - Constant
    lisp_imm const_val;
  };

  /// `fn_arg` - Argument
  /// 
  /// Evaluates to the argument specified by `index`.
  struct fn_arg final : expr {
    /// Const U32 index - Arg Idx
    u32_t index;
  };

  /// `fn_rest_arg` - Rest argument
  /// 
  /// Evaluates to the range of rest arguments.
  /// 
  /// For the `full_call` calling convention, all arguments are passed in `fn_rest_arg`.
  struct fn_rest_arg final : expr { };

  /// `fn_sc_arg` - Static chain arg
  /// 
  /// Evaluates to the topmost activation record of the closure.
  struct fn_sc_arg final : expr { };

  /// `fn_body` - Fn body
  /// 
  /// Let-like structure binding function arguments.
  /// 
  /// Formally evaluates to the last form. This is to simplify the deduction of
  /// function result types. Valid in top-level and in `closure`.
  struct fn_body final : expr {
    /// Ins forms: Any
    J_A(AI,ND,NODISC) inline input_span forms() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins forms: Any
    J_A(AI,ND,NODISC) inline const_input_span forms() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }

    /// Const VarDefs vars - Args
    [[nodiscard]] inline air_var_ptrs_t vars() noexcept {
      return *j::add_bytes<air_var_defs_t*>(this, 24U + max_inputs * 16U);
    }

    /// Const VarDefs vars - Args
    [[nodiscard]] inline const_air_var_ptrs_t vars() const noexcept {
      return *j::add_bytes<const air_var_defs_t*>(this, 24U + max_inputs * 16U);
    }
  };

  /// `progn` - Progn
  /// 
  /// Evaluate `forms` in sequence, evaluating to the lresult of the last form.
  /// 
  /// An empty `progn` evaluates to `nil`.
  struct progn final : expr {
    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline input_span forms() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline const_input_span forms() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `prog1` - Prog1
  /// 
  /// Evaluate `forms` in sequence, evaluating to the result of the first form.
  /// 
  /// An empty `prog1` evaluates to `nil`.
  struct prog1 final : expr {
    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline input_span forms() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline const_input_span forms() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `let` - Let
  /// 
  /// Evaluate `forms` with `vars` in lexical environment.
  struct let final : expr {
    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline input_span forms() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins forms: Any - Forms
    J_A(AI,ND,NODISC) inline const_input_span forms() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }

    /// Const VarDefs vars - Lexvars
    [[nodiscard]] inline air_var_ptrs_t vars() noexcept {
      return *j::add_bytes<air_var_defs_t*>(this, 24U + max_inputs * 16U);
    }

    /// Const VarDefs vars - Lexvars
    [[nodiscard]] inline const_air_var_ptrs_t vars() const noexcept {
      return *j::add_bytes<const air_var_defs_t*>(this, 24U + max_inputs * 16U);
    }
  };

  /// `b_if` - If
  /// 
  /// Evaluate `then_branch` if `cond` is truthy, otherwise evaluate `false_branch`.
  struct b_if final : expr {
    /// In cond: Any - Predicate
    struct input cond;

    /// In then_branch: Any - Then
    struct input then_branch;

    /// In else_branch: Any - Else
    struct input else_branch;
  };

  /// `do_until` - Do Until
  /// 
  /// Evaluate `branch` until it evaluates to a truthy value.
  /// 
  /// Evaluates to the final result of `branch`.
  struct do_until final : expr {
    /// In branch: Any - Loop Body
    struct input branch;
  };

  /// `lex_rd` - Read Var.
  /// 
  /// Evaluates to the value of the lexical variable.
  struct lex_rd final : expr {
    /// Const Var var - Src Var
    air_var* var;

    /// Data UseDefs defs
    span<expr*> defs;
  };

  /// `lex_rd_bound` - Read Bound Var
  /// 
  /// Evaluates to the value of the lexical variable.
  struct lex_rd_bound final : expr {
    /// Const Var var - Src Var
    air_var* var;

    /// Data UseDefs defs
    span<expr*> defs;
  };

  /// `lex_wr` - Write Var
  /// 
  /// Evaluates to `value`.
  struct lex_wr final : expr {
    /// In value: Any - Src
    struct input value;

    /// Const Var var - Dst Var
    air_var* var;

    /// Data UseDefs uses
    span<expr*> uses;
  };

  /// `lex_wr_bound` - Write Bound Var
  /// 
  /// Evaluates to `value`.
  struct lex_wr_bound final : expr {
    /// In value: Any - Src
    struct input value;

    /// Const Var var - Dst Var
    air_var* var;

    /// Data UseDefs uses
    span<expr*> uses;
  };

  /// `sym_val_rd` - Read Global
  /// 
  /// Read a symbol value (global).
  struct sym_val_rd final : expr {
    /// Const Id var - Src Var
    id var;
  };

  /// `sym_val_wr` - Write Global
  /// 
  /// Write a symbol value (global).
  struct sym_val_wr final : expr {
    /// In value: Tagged Imm - Src
    struct input value;

    /// Const Id var - Dst Var
    id var;
  };

  /// `type_assert` - Type assert
  /// 
  /// Throws if `value` is not in `types`.
  struct type_assert final : expr {
    /// In value: Tagged Imm
    struct input value;

    /// Const ImmMask types - AllowList
    imm_type_mask types;
  };

  /// `dyn_type_assert` - Dyn Type assert
  /// 
  /// Throws if `value` is not in `types`.
  /// 
  /// Compiler-generated `type_assert`, that may be moved more freely than user-inserted asserts.
  struct dyn_type_assert final : expr {
    /// In value: Tagged Imm
    struct input value;

    /// Const ImmMask types - AllowList
    imm_type_mask types;
  };

  /// `ineg` - Integer Negate
  /// 
  /// Negate an integer.
  struct ineg final : expr {
    /// In value: I64 - Integer
    struct input value;
  };

  /// `iadd` - Integer Add
  /// 
  /// Add integers given as `values`.
  struct iadd final : expr {
    /// In lhs: I64 - Integers
    struct input lhs;

    /// In rhs: I64
    struct input rhs;
  };

  /// `isub` - Integer Subtract
  /// 
  /// Subtract integers given as `values`.
  struct isub final : expr {
    /// In lhs: I64 - Integers
    struct input lhs;

    /// In rhs: I64
    struct input rhs;
  };

  /// `imul` - Integer Multiply
  /// 
  /// Multiply integers given as `values`.
  struct imul final : expr {
    /// In lhs: I64 - Integers
    struct input lhs;

    /// In rhs: I64
    struct input rhs;
  };

  /// `idiv` - Integer Divide
  /// 
  /// Divide the first value by the rest of the values.)
  struct idiv final : expr {
    /// In lhs: I64 - Integers
    struct input lhs;

    /// In rhs: I64
    struct input rhs;
  };

  /// `lor` - Logical Or
  /// 
  /// Short-circuiting OR for `values`.
  /// 
  /// Evaluates to the value of the first truthy expression, or to `false`.
  struct lor final : expr {
    /// Ins values: Any - Forms
    J_A(AI,ND,NODISC) inline input_span values() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins values: Any - Forms
    J_A(AI,ND,NODISC) inline const_input_span values() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `land` - Logical And
  /// 
  /// Short-circuiting AND for `values`.
  /// 
  /// Evaluates to the value of the first falsy expression, or to the final expression.
  struct land final : expr {
    /// Ins values: Any - Forms
    J_A(AI,ND,NODISC) inline input_span values() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins values: Any - Forms
    J_A(AI,ND,NODISC) inline const_input_span values() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `lnot` - Logical Not
  /// 
  /// Evaluates to `false` if `value` is truthy, `true` otherwise.
  struct lnot final : expr {
    /// In value: Any - Input
    struct input value;
  };

  /// `to_bool` - Cast to Bool
  /// 
  /// Evaluates to `true` if `value` is truthy, `false` otherwise.
  struct to_bool final : expr {
    /// In value: Any - Input
    struct input value;
  };

  /// `eq` - Eq
  /// 
  /// Identity equality.
  /// 
  /// Note that strings are not `eq`-equal.
  struct eq final : expr {
    /// In lhs: Any - Lhs
    struct input lhs;

    /// In rhs: Any - Rhs
    struct input rhs;
  };

  /// `neq` - Not Eq
  /// 
  /// Identity inequality.
  /// 
  /// Note that strings are not `eq`-equal.
  struct neq final : expr {
    /// In lhs: Any - Lhs
    struct input lhs;

    /// In rhs: Any - Rhs
    struct input rhs;
  };

  /// `tag` - Tag
  /// 
  /// Tags `untagged`.
  /// 
  /// Note that boolean, nil, and symbol values are always tagged.
  struct tag final : expr {
    /// In untagged: Untagged Imm - Untagged
    struct input untagged;
  };

  /// `untag` - Untag
  /// 
  /// Untags `tagged`.
  /// 
  /// Note that boolean, nil, and symbol values are always tagged.
  struct untag final : expr {
    /// In tagged: Tagged Imm - Tagged
    struct input tagged;
  };

  /// `as_range` - Cast to Range
  /// 
  /// Casts `vec` to range (i.e. a pair with `begin` pointer and size.
  /// 
  /// The result type is still Vec, but its representation will be a range.
  struct as_range final : expr {
    /// In vec: NotRange Vec - Vec
    struct input vec;
  };

  /// `as_vec` - Cast to Vec
  /// 
  /// Casts `range` to vec.
  /// 
  /// Allocates a new vec for pure ranges (e.g. rest arguments).
  struct as_vec final : expr {
    /// In range: Range Range - Range
    struct input range;
  };

  /// `call` - Call Closure
  /// 
  /// Calls `closure` with `args`.
  struct call final : expr {
    /// In closure: Untagged Closure - Fn
    struct input closure;

    /// Ins args: Tagged Imm - Args
    [[nodiscard]] inline input_span args() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs - 1);
    }

    /// Ins args: Tagged Imm - Args
    [[nodiscard]] inline const_input_span args() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs - 1);
    }
  };

  /// `full_call` - Full Call
  /// 
  /// Calls a free function `fn` with `args`.
  struct full_call final : expr {
    /// Ins args: Tagged Imm - Fn Args
    J_A(AI,ND,NODISC) inline input_span args() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins args: Tagged Imm - Fn Args
    J_A(AI,ND,NODISC) inline const_input_span args() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }

    /// Const Id fn - Fn Name
    [[nodiscard]] inline id fn() const noexcept {
      return *j::add_bytes<const id*>(this, 24U + max_inputs * 16U);
    }
  };

  /// `vec` - Vec
  /// 
  /// Initialize a vec with `elems` as values.
  struct vec final : expr {
    /// Ins elems: Tagged Imm - Elements
    J_A(AI,ND,NODISC) inline input_span elems() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins elems: Tagged Imm - Elements
    J_A(AI,ND,NODISC) inline const_input_span elems() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `make_vec` - Make Vec
  /// 
  /// Allocate a vec with `size` elements.
  /// 
  /// The elements are initialized to `nil`.
  struct make_vec final : expr {
    /// In size: I64 - Size
    struct input size;
  };

  /// `vec_append` - Vec Append
  /// 
  /// Append `vecs...` into a single vec.
  struct vec_append final : expr {
    /// Ins vecs: Untagged Vec - Vecs
    J_A(AI,ND,NODISC) inline input_span vecs() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins vecs: Untagged Vec - Vecs
    J_A(AI,ND,NODISC) inline const_input_span vecs() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }
  };

  /// `vec_build` - Quasi
  /// 
  /// Build a vec with `elems` as values, possibly splatted.)
  struct vec_build final : expr {
    /// Ins elems: Any - Elements
    J_A(AI,ND,NODISC) inline input_span elems() noexcept {
      return input_span(reinterpret_cast<exprs::input*>(this + 1), num_inputs);
    }

    /// Ins elems: Any - Elements
    J_A(AI,ND,NODISC) inline const_input_span elems() const noexcept {
      return const_input_span(reinterpret_cast<const exprs::input*>(this + 1), num_inputs);
    }

    /// Const Quasis quasi_info
    [[nodiscard]] inline quasis_t quasi_info() const noexcept {
      return *j::add_bytes<const quasis_t*>(this, 24U + max_inputs * 16U);
    }
  };

  /// `range_length` - Range Length
  /// 
  /// Evaluates to the number of elements in a range or a vec.
  struct range_length final : expr {
    /// In range: Any Vec|Range - Range
    struct input range;
  };

  /// `range_get` - Range Get
  /// 
  /// Evaluates to the `index`:th element in a range or vec.
  struct range_get final : expr {
    /// In range: Any Vec|Range - Source Range
    struct input range;

    /// In index: I64 - Index
    struct input index;
  };

  /// `range_set` - Range Set
  /// 
  /// Writes `value` to a range or a vec, evaluating to `value`.
  struct range_set final : expr {
    /// In range: Any Vec|Range - Target Range
    struct input range;

    /// In index: I64 - Index
    struct input index;

    /// In value: Tagged Imm - Value
    struct input value;
  };

  /// `range_copy` - Copy Range
  /// 
  /// Copy `range` as a vec.
  struct range_copy final : expr {
    /// In range: Untagged|Range Vec|Range - Source Range
    struct input range;
  };

  /// `act_rec_alloc` - Make Activation Record
  /// 
  /// Allocate an activation record.
  struct act_rec_alloc final : expr {
    /// In parent: NotRange Nil|ActRec - Parent Scope
    struct input parent;

    /// Const U32 index - ActRec Index
    u32_t index;
  };

  /// `act_rec_wr` - Write Activation Record
  /// 
  /// Write a variable in an activation record.
  struct act_rec_wr final : expr {
    /// In act_rec: Untagged ActRec - Activation Rec
    struct input act_rec;

    /// In value: Tagged Imm - Value
    struct input value;

    /// Const Var var - Bound Lexvar
    air_var* var;
  };

  /// `act_rec_rd` - Read Activation Record
  /// 
  /// Read a variable from an activation record.
  struct act_rec_rd final : expr {
    /// In act_rec: Untagged ActRec - Activation Rec
    struct input act_rec;

    /// Const Var var - Bound Lexvar
    air_var* var;
  };

  /// `act_rec_parent` - Activation Record Parent
  /// 
  /// Evaluates to the parent of `act_rec`.
  struct act_rec_parent final : expr {
    /// In act_rec: Untagged ActRec - Activation Rec
    struct input act_rec;
  };

  /// `closure` - Construct Closure
  /// 
  /// Evaluates to `closure` bound with `act_rec`.
  /// 
  /// Note that `body` is only used during transformation passes. When compiling,
  /// the function is read from `closure`.
  struct closure final : expr {
    /// In act_rec: NotRange Nil|ActRec - Activation Rec
    struct input act_rec;

    /// In body: Tagged Imm - Closure Body
    struct input body;

    /// Const Closure closure - Closure Info
    air_closure* closure;
  };
}
