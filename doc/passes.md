# Compilation passes

Compilation passes are evaluated on a per-function, not per module, basis. This is because
translating AST may invoke macros defined elsewhere, requiring recursive compilation.

## Function compilation stages

  1. Macroexpand (Initial AST -> Final AST)
    - Expand all macros until no unexpanded macros remain.
    - This may trigger compilation of other functions.
  2. AST to AIR (Final AST -> AIR)
  3. AIR to CIR
  4. CIR to machine code

## AST to AIR conversion

First, the AIR CFG is created:

  1. Function entry basic block is inserted, with an `@fn-entry` op.
  2. For each function parameter, the parameter is read into a lexical variable with `@fn-arg` and `@lex-bind`.
  3. The function body is transformed as a `progn` expression.
  4. The result of the function body is returned with `@fn-ret`

Then, the following passes are run on the AIR CFG:

  - `dce_pass` - Dead code elimination
    Ops that have no side effects and whose result is not used are removed.

## AIR to CIR conversion

  1. Dominance frontiers of the AIR CFG are computed
    - This implies constructing a reverse-postorder numbering for the CFG, and computing immediate dominators
  2. Lexical bindings and writes are converted to SSA variables
  3. Phi-functions are inserted at dominance frontiers
  4. Variables are renamed
  5. Ops are mapped to their CIR counterparts

Then, the following passes are run on the CIR SSA:

  - TBD

Then,

  1. Individual ops are numbered by reverse post order numbering
  2. Live ranges are determined
  3. Registers are allocated
  4. Phi nodes are eliminated

## CIR to machine code

  1. CIR code is walked op by op, in its post order numbering.
