# Calling conventions

## Full call

This is the default calling convention used when nothing is known of the target function.

`%rdi` contains the number of arguments, untagged.
`%rsi` contains a pointer to the first argument. All arguments are passed in tagged encoding.

Return value is returned in `%rax`.

## Standard call

Follows the C calling convention. Rest argument is passed as two arguments, an untagged number
containing the number of arguments, and a pointer to the first rest argument. All arguments are
passed in target encoding.

Return value is returned in `%rax`.
