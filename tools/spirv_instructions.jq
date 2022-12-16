import "gen" as gen;
import "spirv" as spv;

(.instructions | spv::filter_opcodes) as $opcodes
| [
  "#pragma once",
  "// GENERATED FILE - Do not edit.",
  "",
  "#include <containers/pair.hpp>",
  "#include <containers/span.hpp>",
  "#include <exceptions/assert_lite.hpp>",
  "#include <rendering/vulkan/spirv/instruction.hpp>",
  "",
  "namespace j::rendering::vulkan::inline spirv {",
  "  inline void assert_length(const instruction & insn, u32_t len) noexcept {",
  "    J_ASSERT(insn.word_count() > len, \"Truncated instruction\");",
  "  }"
  ] +
  ($opcodes | map(spv::emit_opcode_class)) +
  [
    "",
    ($opcodes | spv::type_declarations | spv::emit_opcode_predicate("is_type_declaration")),
    "",
    ($opcodes | spv::type_declarations | spv::emit_instruction_predicate("is_type_declaration_v")),
    "",
    ($opcodes | spv::emit_opcode_instruction_map("opcode_of")),
    "}"
  ]
| gen::lines_to_string
