# AIR (Analytical IR)

AIR is the first IR after AST. It is intended for transformations leveraging the
language-level properties (as opposed to low-level execution environment related
transformations.)

Functions are represented as an expression tree, in non-SSA, non-CFG form.

All symbols are resolved, i.e. the package and export are identified. Imported
symbols are not necessarily compiled to AIR yet - that is done recursively as needed.

## Structure

### AIR variables

  - Name - Symbol ID
  - Flags:
    - Is global? I.e. whether is defined at top-level. Otherwise, it is bound with let or
      as a function argument.
    - Is const? I.e. whether the variable is unassigable.
      The value targeted can be modified though - unless it is a number, boolean, or nil.
    - Is readonly? I.e. whether the value can be assigned by us.
      Readonly-exported globals have this set true, as do const variables.

### AIR objects (non-immediates)

  - Struct definition
  - Object type
  - Type-specific information
    - Vec
  - Lifetime
    - Start: top-level, let-bound, let-bound re-entrant
    - End: never, unknown
  - List of slots (immediates) referencing the value
  - Whether the object is defined to be immutable

### AIR functions

  - Name (id, i.e. a symbol)
  - Type: normal, macro
  - Parameters
    - Index
    - Name
    - Types
    - Optional / rest
  - Closure bindings
  - Docstring
  - Recursive: leaf, full, tail, unknown
  - Lifetime
    - Start: top-level, let-bound, let-bound re-entrant
    - End: never, unknown
  - List of other functions referencing the function
    - Call
    - Reference
  - List of slots (immediates) referencing the value
  - Expression tree

### Value types

Value types available are the Lisp immediate types:

  - `i64`
  - `f32`
  - `boolean`
  - `nil`
  - `sym_id` (including keywords)
  - `string_ref`
  - `vector_ref`
  - `function_ref`
  - `object_ref`

All of these values fit in a register and are not modifiable - the targets of
references may be, however.

### AIR nodes

AIR operations consist of operands, variables brought to scope, and optionally branches.
Operands of the node are unconditionally executed one or more times. Depending on the result,
zero or one branches of the node may be executed, possibly multiple times in case of loops.

The result value of the node depends on the specific node.

#### Function parameters and the like
  - `@fn-arg PARAM`
    **Number of operands:** *0*
    **Result:** the value of the argument.

  - `@current-error`
    **Number of operands:** *0*
    **Result:** the error that was caught


#### Errors
  - `@throw VALUE` Throw a value.
    **Number of operands:** *1* - The value to throw.
    **Result:** Does not return.

#### Function call
  - `@call-0 FN [ARGS...] SUCCESS-EXIT [ERROR-EXIT]` Call a function with values specified, discarding its result.
    **Number of inputs:** *1..N* - The function value and optionally its arguments.
    **Number of outputs:** *0* - In the error case the thrown error
    **Number of lexical ops:** *0*
    **Number of exits:** *1..2* - Normal return and an error return.
  - `@call-1 FN [ARGS...] SUCCESS-EXIT [ERROR-EXIT]` Call a function with values specified, discarding its result.
    **Number of inputs:** *1..N* - The function value and optionally its arguments.
    **Number of outputs:** 1 - The return value of the function.
    **Number of lexical ops:** *0*
    **Number of exits:** *1..2* - Normal return and an error return.

#### Lexical bindings
  - `@lex-capture FUNCTION NAMES...` Capture a lexical closure from the function specified.
    **Number of operands:** *0*
    **Result:** The closure.
  - `@lex-write NAME VALUE`
    **Number of operands:** *1* - The value to assign to lexical variable `NAME`.
    **Result:** None
  - `@lex-read NAME`
    **Number of operands:** *0*
    **Result:** *1* - The current value of the lexical variable `NAME`.

#### Symbols
  - `@symbol-value NAME`
    **Number of operands:** *0*
    **Result:** *1* - The current value of symbol `NAME`.
  - `@set-symbol-value NAME VALUE`
    **Number of operands:** *1* - The value to assign to symbol value `NAME`.
    **Result:** None

#### Creating values
  - `@ld-const CONSTANT`
    **Number of operands:** *0*
    **Result:** the constant (`lisp_imm`).

#### Control flow
  - `@b-if INPUT THEN-BRANCH ELSE-BRANCH`
    **Number of operands:** *1* The operand is checked for truthyness,
    **Branches:** *2* `then` and `else`
    **Result:** The result of the branch taken.
  - `@l-do-until BODY`
    **Number of operands:** *0*
    **Branches:** *1* Body - evaluated successively until it evaluates to a truthy value.
    **Result:** Result of the last iteration of BODY.

#### Arithmetic
  - `@iadd LHS RHS`
    **Number of operands:** *2*
    **Result:** `lhs + rhs`

  - `@isub LHS RHS`
    **Number of operands:** *2*
    **Result:** `lhs - rhs`

#### Comparison
  - `@eq LHS RHS`
    **Number of operands:** *2*
    **Result:** `LHS == RHS` as raw register comparison.
  - `@neq LHS RHS`
    **Number of operands:** *2*
    **Result:** `LHS != RHS` as raw register comparison.

#### ...
  - Cast operations
    - 'as-i64 'as-f32
  - Arithmetic operations
    - '/ '* '% 'neg 'abs 'max 'min
  - Bitwise operations
    - 'band 'bor 'bxor 'bnot
  - Boolean operations
    - 'and 'or 'xor 'not
  - Vec operations
    - 'nappend 'nmap 'clear 'push 'pop 'shift 'unshift 'insert 'erase
    - 'append 'map 'filter 'map


## Design decision notes

  - `@fn-arg` is an expression instead of arguments being defined implicitly.
    This might not have been a good idea after all.
  - Loop primitive `@l-do-until` was chosen, since:
    1. `do` loops always execute the body, generating simpler SSA.
    2. `until` loop ends with the condition evaluating to a non-falsy value.
       This allows making the result of the loop expression the value it last evaluated,
       while still being useful.
