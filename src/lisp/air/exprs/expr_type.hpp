#pragma once
// GENERATED HEADER - EDITS WILL BE LOST.
#include "strings/string_view.hpp"

namespace j::lisp::inline values     { struct lisp_str; }
namespace j::lisp::air {
  enum class expr_type : u8_t {
    /// Nop
    /// 
    /// `nop`s are not moved or optimized away, as they are usually used for placeholders for transformations.
    nop,             // 0
    /// Load constant
    ld_const,        // 1
    /// Argument
    fn_arg,          // 2
    /// Rest argument
    /// 
    /// For the `full_call` calling convention, all arguments are passed in `fn_rest_arg`.
    fn_rest_arg,     // 3
    /// Static chain arg
    fn_sc_arg,       // 4
    /// Fn body
    /// 
    /// Formally evaluates to the last form. This is to simplify the deduction of
    /// function result types. Valid in top-level and in `closure`.
    fn_body,         // 5
    /// Progn
    /// 
    /// An empty `progn` evaluates to `nil`.
    progn,           // 6
    /// Prog1
    /// 
    /// An empty `prog1` evaluates to `nil`.
    prog1,           // 7
    /// Let
    let,             // 8
    /// If
    b_if,            // 9
    /// Do Until
    /// 
    /// Evaluates to the final result of `branch`.
    do_until,        // 10
    /// Read Var.
    lex_rd,          // 11
    /// Read Bound Var
    lex_rd_bound,    // 12
    /// Write Var
    lex_wr,          // 13
    /// Write Bound Var
    lex_wr_bound,    // 14
    /// Read Global
    sym_val_rd,      // 15
    /// Write Global
    sym_val_wr,      // 16
    /// Type assert
    type_assert,     // 17
    /// Dyn Type assert
    /// 
    /// Compiler-generated `type_assert`, that may be moved more freely than user-inserted asserts.
    dyn_type_assert, // 18
    /// Integer Negate
    ineg,            // 19
    /// Integer Add
    iadd,            // 20
    /// Integer Subtract
    isub,            // 21
    /// Integer Multiply
    imul,            // 22
    /// Integer Divide
    idiv,            // 23
    /// Logical Or
    /// 
    /// Evaluates to the value of the first truthy expression, or to `false`.
    lor,             // 24
    /// Logical And
    /// 
    /// Evaluates to the value of the first falsy expression, or to the final expression.
    land,            // 25
    /// Logical Not
    lnot,            // 26
    /// Cast to Bool
    to_bool,         // 27
    /// Eq
    /// 
    /// Note that strings are not `eq`-equal.
    eq,              // 28
    /// Not Eq
    /// 
    /// Note that strings are not `eq`-equal.
    neq,             // 29
    /// Tag
    /// 
    /// Note that boolean, nil, and symbol values are always tagged.
    tag,             // 30
    /// Untag
    /// 
    /// Note that boolean, nil, and symbol values are always tagged.
    untag,           // 31
    /// Cast to Range
    /// 
    /// The result type is still Vec, but its representation will be a range.
    as_range,        // 32
    /// Cast to Vec
    /// 
    /// Allocates a new vec for pure ranges (e.g. rest arguments).
    as_vec,          // 33
    /// Call Closure
    call,            // 34
    /// Full Call
    full_call,       // 35
    /// Vec
    vec,             // 36
    /// Make Vec
    /// 
    /// The elements are initialized to `nil`.
    make_vec,        // 37
    /// Vec Append
    vec_append,      // 38
    /// Quasi
    vec_build,       // 39
    /// Range Length
    range_length,    // 40
    /// Range Get
    range_get,       // 41
    /// Range Set
    range_set,       // 42
    /// Copy Range
    range_copy,      // 43
    /// Make Activation Record
    act_rec_alloc,   // 44
    /// Write Activation Record
    act_rec_wr,      // 45
    /// Read Activation Record
    act_rec_rd,      // 46
    /// Activation Record Parent
    act_rec_parent,  // 47
    /// Construct Closure
    /// 
    /// Note that `body` is only used during transformation passes. When compiling,
    /// the function is read from `closure`.
    closure,         // 48
    none,
  };

  inline constexpr i32_t max_expr_type_len_v = 15;
  inline constexpr i32_t max_expr_short_name_len_v = 7;
  inline constexpr u8_t num_expr_types_v = (u8_t)expr_type::none;
  inline constexpr u8_t num_expr_types_with_none_v = (u8_t)expr_type::none + 1U;

  extern constinit strings::const_string_view expr_type_names[num_expr_types_with_none_v];
  extern constinit strings::const_string_view expr_type_short_names[num_expr_types_with_none_v];
  extern constinit const lisp_str * expr_type_lisp_names[num_expr_types_with_none_v];

  J_INLINE_GETTER constexpr bool is_branch(expr_type t, u8_t i) noexcept {
    return t == expr_type::do_until || t == expr_type::land || t == expr_type::lor || (t == expr_type::b_if && i > 0);
  }

  J_INLINE_GETTER constexpr bool has_branches(expr_type t) noexcept {
    return t == expr_type::do_until || t == expr_type::land || t == expr_type::lor || t == expr_type::b_if;
  }

  J_INLINE_GETTER constexpr bool has_fixed_branches(expr_type t) noexcept {
    return t == expr_type::do_until || t == expr_type::b_if;
  }

  J_INLINE_GETTER constexpr bool is_progn_like(expr_type t) noexcept {
    return t == expr_type::progn || t == expr_type::let || t == expr_type::fn_body;
  }

  J_INLINE_GETTER constexpr bool is_lex_read(expr_type t) noexcept {
    return t == expr_type::lex_rd || t == expr_type::lex_rd_bound;
  }

  J_INLINE_GETTER constexpr bool is_lex_write(expr_type t) noexcept {
    return t == expr_type::lex_wr || t == expr_type::lex_wr_bound;
  }
}
