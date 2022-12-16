# DIE (Debug Information Entry)

- Tree nodes, owning their children. Tree in prefix order.
  - Each DIE has a bit specifying whether it has children or not.
  - List of siblings is terminated by a null entry
  - For faster skipping, `DW_AT_sibling` attribute may point to the next sibling.
    Clang does not seem to generate these.
- Tagged structure, containing a type tag (`DW_TAG_*`) and attributes (`DW_AT_*`) containing values
- Each DIE can only have one copy of an attribute.
- Most of these describe static aspects of the program, such as compilation units, types, functions.
- Usually they live in `.debug_info` or `.debug_info.dwo` section, but they can also live
  in the object files, without being linked to the binary.

# Attribute classes (forms)

- Storage format of an attribute

## Constants `constant`

- 1, 2, 4, 8, or 16 bytes, or variable.
- Variable count format is called `LEB128`, little-endian bytes 128.

## Memory addresses

Static addresses:

- `address` address to memory space of the program
- `addrptr` index into `.debug_addr` containing an address. Per-compilation unit `DW_AT_addr_base`
  needs to be added to the **index**.

Dynamic addresses (for objects whose location may change, e.g. during stack operations)

- `exprloc` DWARF expression that may resolve to an address, or to a value.
- `loclist` references to the old `.debug_loc`, describing movable objects
- `loclistptr` reference to the new `.debug_loclists`, describing movable objects

Static, non-contiguous ranges:

- `rnglist` references to the old `.debug_ranges`
- `rnglistsptr` reference to the new `.debug_rnglists`

## Strings

Null-terminated and printable.

- `string` contains either an inline string, or a location in string table
- `stroffsetptr` Offset to offset in string table (indirection)

## Line numbers `lineptr`

- Points to the `.debug_line_str` section


## DIE references `reference`

Four types:

1. Offset from compilation unit begin, within the same unit,
2. Offset from begin of a any compilation unit,
3. Type definition reference by 64-bit index ("signature"), and
4. Reference to an entry in `.debug_info` of another file.

# Expressions `DW_OP_*`

- Compute either a memory address or a value.
- Postfix stack machine, i.e. like FORTH.

## Constants

- Target machine address `DW_OP_addr`
- Reference to `.debug_addr` (`DW_OP_addrx`). Per-compilation unit `DW_AT_addr_base`
  needs to be added to the **index**.
- Reference to a constant value in `.debug_addr` (`DW_OP_constx`). Per-compilation unit `DW_AT_addr_base`
  needs to be added to the **index**.
  Constants in `.debug_addr` can also be relocated, so this is for those.
- Special opcodes for constants 0-31 (`DW_OP_lit0` .. `DW_OP_lit31`)
- Unsigned and signed constants, 1, 2, 4, or 8 bytes (`DW_OP_const1u`, `DW_OP_const8s`)
- Variable-size unsigned and signed consts in LEB128 format (`DW_OP_constu`, `DW_OP_consts`)

Typed constants can be specified by `DW_OP_const_type`, which specifies a `DW_TAG_base_type`, value size, and
value blob.

# Version differences

## DWARF 5

- `.debug_types` section merged into `.debug_info`
- Supports fusion/fission, i.e. separate debug info files covering multiple binaries.
  This is implemented in sections suffixed with `.dwo`.
- Moved line number info into a separate `.debug_line_str` section.
- `.debug_macinfo` replaced with new format, `.debug_macro`
- `.debug_pubnames` and `.debug_pubtypes` merged into `.debug_names`.
- `.debug_loc` and `.debug_ranges` replaced with new format `.debug_loclists` and `.debug_rnglists`.
- Support tagging tail calls
- Support type info for stack contents
- Added multiple operand forms and a header field that make DWARF 5 backwards-incompatible.
- Changes string length encoding in a backwards-incompatible manner

## DWARF 4

- Added a separate `.debug_types` sectiona, removed in DWARF 5
- Support ISAs that have multiple instructions per byte
- Allow identifying `main()`.
- Better support for packed structs/arrays.
- C++ features, like template parameters and `enum class`.
- Backwards-incompatible due to new forms and changes to location info
