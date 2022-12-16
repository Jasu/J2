# Lisp

## Types

### Integer (`int`)

A 63-bit signed integer. Encoded as `value << 1` - i.e. bit 0 is clear.

### Float (`float`)

A single-precision float. Encoded as `value << 32 | 0b0001`.

### Boolean (`bool`)

Either `0b1_00000011` for true, or `0b0_00000011` for false.

### Nil (`nil`)

Encoded as `0b00000111`.

### Function

Tagged pointer to the function code, with `full call` calling convention.

### String

### Symbol

### Vec

### Rec
