import "gen" as gen;
import "spirv" as spv;

(.instructions | spv::filter_opcodes) as $opcodes
| [
  "// GENERATED FILE - Do not edit.\n",
  "#include <rendering/vulkan/spirv/enums.hpp>",
  "#include <strings/formatters/enum_formatter.hpp>",
  "",
  "namespace j::rendering::vulkan::inline spirv {",
  "  namespace s = strings;",
  ($opcodes | spv::emit_enum_formatter("opcode"; .opname; "")),
  "",
  (.operand_kinds | spv::find_enum("StorageClass") | .enumerants | spv::emit_enum_formatter("storage_class"; .enumerant; "")),
  "",
  (.operand_kinds | spv::find_enum("ImageFormat") | .enumerants | spv::emit_enum_formatter("image_format"; .enumerant; "")),
  "",
  (.operand_kinds | spv::find_enum("Decoration") | .enumerants | spv::emit_enum_formatter("decoration"; .enumerant; "")),
  "}"
] | gen::lines_to_string
