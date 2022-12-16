import "gen" as gen;
import "spirv" as spv;

(.instructions | spv::filter_opcodes | spv::type_declarations) as $opcodes
| [
    "#pragma once\n",
    "// GENERATED HEADER - Do not edit.",
    "",
    "#include <rendering/vulkan/spirv/instruction.hpp>",
    "#include <containers/range.hpp>",
    "#include <exceptions/assert.hpp>",
    "",
    "namespace j::rendering::vulkan::inline spirv {",
    ($opcodes | spv::emit_opcode_predicate("is_type_predicate")),
    ""
  ] +
  ($opcodes | map(spv::emit_opcode_class)) +
  [
    "}"
  ]
| gen::lines_to_string
