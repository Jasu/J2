# CIR (Constructive IR)

CIR is the last IR stage before platform-specific code.

## Structure

### CIR functions

  - SSA form
  - Parameters
    - Index
    - Register type
    - Optional / rest
  - Closure bindings

## CIR datatypes

CIR datatypes are machine-level datatypes directly translatable to machine code:

  - `i64`, `i32`, `i16`, `i8`
  - `f64`, `f32`

Pointers are represented as `i64`. Note that there is no difference between signed
and unsigned integers - the difference lies in the operations used, not on the type.

## CIR operations

### Function entry and exit

  - `%fn-enter` - Build a stack frame
  - `%fn-leave` - Tear down a stack frame
    Note that `%fn-leave` is separate from `%fn-ret` - this is to enable tail calls.
  - `%fn-arg` - Read argument value
  - `%fn-ret` - Return from function with the value given.

### Function call
  - `%fn-call-full` - Do a full call with the array and size provided.

### Constants / `%mov`
  - `%mov` - Move value from a constant or a register to a register.
    Note that since CIR is SSA, moves between register only duplicate immutable values, rather than
    doing anything useful. `%mov` is used for register moves only during register allocation, when
    real registers are being assigned.

### Comparisons
  - `%eq` - Eq-equality. Note that in most cases, `%b-if` is used instead.
  - `%neq` - Eq-inequality. Note that in most cases, `%b-if` is used instead.

### Branches

  - `%b-goto` - Direct goto to another basic block
  - `%b-if` - Conditional branch to two alternative basic blocks.

### Register allocation

  - `%push` - Push a register to stack
  - `%pop` - Pop a register from stack
  - `%%phi` - Select the value of the inbound branch.
  - `%%spill` - Spill a value from a register to a spill slot.
  - `%%unspill` - Unspill a value from a spill slot to a register.

### Arithmetic

  - `%iadd` - Add two integers
  - `%isub` - Subtract two integers
  - `%shl` - Shift left
  - `%shr` - Shift logical right
  - `%sar` - Shift arithmetic right

### Memory access

  - `%mem-rd` - Read a value (any CIR-supported size) from the address provided. The address may have an index.
  - `%mem-wr` - Write a value (any CIR-supported size) to the address provided. The address may have an index.
