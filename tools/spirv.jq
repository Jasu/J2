import "gen" as gen;

def sanitize_keywords:
  {
    "return": "ret",
    "volatile": "volatile_",
    "const": "constant",
    "private": "private_",
    "inline": "inline_",
    "switch": "switch_",
    "default": "default_"
  }[.] // .;

def check_capabilities:
  {
    Matrix: true,
    Shader: true,
    Float16: true,
    Float64: true,
    Int8: true,
    Int16: true,
    Int64: true,
    UniformBufferArrayDynamicIndexing: true,
    SampledImageArrayDynamicIndexing: true,
    ClipDistance: true,
    CullDistance: true,
    InputAttachment: true,
    MinLod: true,
    ImageMSArray: true,
    ImageQuery: true,
    InterpolationFunction: true,
    TransformFeedback: true,
    DrawParameters: true
  } as $allowed_capabilities
  | .always_include? or (.capabilities | not) or (.capabilities | any(in($allowed_capabilities)));

def check_extensions:
  {
    SPV_KHR_vulkan_memory_model: true,
    SPV_KHR_terminate_invocation: true,
    SPV_KHR_shader_draw_parameters: true,
    SPV_KHR_float_controls: true,
    SPV_INTEL_shader_integer_functions2: true,
    SPV_GOOGLE_decorate_string: true
  } as $allowed_extensions
  | (.extensions? // [])
  | all(in($allowed_extensions));

def operand_name:
  if .name? then
    if .name == "'D~ref~'" then
      "d_ref"
    elif .name == "'Instruction'" then
      "instruction_id"
    elif .name |contains("Parameter 0") then
      "parameter_types"
    elif .name |contains("Argument 0") then
      "arguments"
    elif .name |contains("Variable, Parent") then
      "pairs"
    elif .name |contains("Operand 1") then
      "operands"
    elif .name |contains("Member 0") then
      "member_types"
    elif .name |contains("The name") then
      "name"
    else
      .name[1:-1] | gsub(" "; "_") | ascii_downcase
    end
  else
    .kind|gen::snake_case
  end;

def type_declarations: map(select(.class == "Type-Declaration"));
def without_type_declarations: map(select(.class != "Type-Declaration"));

def max_opcode: [.[].opcode]|max;
def min_opcode: [.[].opcode]|min;
def opcode_range: (.|max_opcode) - (.|min_opcode) + 1;
def opcode_range_dwords: (((. | opcode_range) + 31) / 32) | floor;
def opcode_masks:
  [.[].opcode - (. | min_opcode)] as $opcodes
  | range(0; ($opcodes | max) / 32 |ceil * 32; 32)
    | . as $start
      | ($opcodes | map(select((. >= $start) and (. < $start + 32))) | map((. - $start)|exp2) | add // 0);

def opcodes_matching_capabilities(caps):
  map(select(.capabilities? // [] | all(caps|contains(.))));
def opcodes_excluding_capabilities(caps):
  map(select(.capabilities? // [] | any(. as $cap|caps|any(. == $cap)) | not));

def filter_opcodes:
  # opcodes_excluding_capabilities(
  #     ["Geometry", "Tesselation", "Pipes", "Groups", "TesselationPointSize",
  #      "SparseResidency", "ImageQuery", "TransformFeedback", "MultiViewport",
  #      "NamedBarrier", "PipeStorage", "GroupNonUniform", "GroupNonUniformVote",
  #      "GroupNonUniformArithmetic", "GroupNonUniformBallot", "GroupNonUniformShuffle",
  #      "GroupNonUniformShuffleRelative", "GroupNonUniformClustered", "GroupNonUniformQuad",
  #      "ShaderLayer", "ShaderViewportIndex", "SubgroupBallotKHR", "SubgroupVoteKHR",
  #      "VariablePointers", "AtomicStorageOps",

  #      "CooperativeMatrixNV",
  #      "DemoteToHelperInvocationEXT",
  #      "SubgroupAvcMotionEstimationINTEL",

  #      "MeshShadingNV",
  #      "RayQueryProvisionalKHR", "RayTracingNV", "RayTracingProvisionalKHR"
  #     ])
  map(select(
            (. | check_capabilities)
            and (. | check_extensions)
            and .class != "Atomic"
            and .class != "Group"
            # TODO - two optional args at end - is this bug in spec or real?
            and .opname != "OpCopyMemory" and .opname != "OpCopyMemorySized"
            and .opname != "OpMemberDecorateStringGOOGLE"
            and .opname != "OpDecorateStringGOOGLE"
          )
     );

def find_enum(kind): map(select(.kind == kind))[0];
def find_opcode(name): map(select(.opname == name))[0];

def emit_enum(name; get_name; get_value; prefix):
  map(select((. | check_capabilities) and (. | check_extensions)))
  | map({
    name: . | get_name | gen::remove_prefix(prefix) | gen::snake_case | sanitize_keywords,
    value: . | get_value | tostring
  }) as $enum
  | ($enum|map(.name|length)|max) as $maxlen
  | [
    "enum class \(name) : u32_t {",
    ($enum|map(.name + (" " * ($maxlen - (.name | length) + 1)) +  "= " + .value + ",")),
    "};"
  ];

def emit_enum_formatter(name; get_name; prefix):
  map(select((. | check_capabilities) and (. | check_extensions)))
  | map({
    name: . | get_name | gen::remove_prefix(prefix) | gen::snake_case | sanitize_keywords,
    l: . | get_name | gen::remove_prefix(prefix)
  }) as $enum
  | ($enum|map(.name|length)|max) as $maxlen
  | ($enum|map(.l|length)|max) as $maxllen
  | [
      "J_A(ND, NODESTROY) const s::formatters::enum_formatter<\(name)> \(name)_formatter{{",
      ($enum|map("{ \(name)::\(.name),\(" " * ($maxlen - (.name|length) + 1)) \"\(.l)\"\(" " * ($maxllen - (.l|length) + 1))},")),
      "}};"
    ];

def operand_type:
  if .kind == "IdResult" or .kind == "IdResultType" or .kind == "LiteralInteger" or .kind == "IdRef" or .kind == "Dim" or .kind == "IdScope" or .kind == "IdMemorySemantics" or .kind == "LiteralString" or .kind == "LiteralContextDependentNumber" or .kind == "LiteralExtInstInteger" or .kind == "LiteralSpecConstantOpInteger" or .kind == "LiteralSpecConstantInteger" then
    "u32_t"
  elif .kind == "PairLiteralIntegerIdRef" then
    "pair<i32_t, u32_t>"
  elif .kind == "PairIdRefIdRef" then
    "pair<u32_t, u32_t>"
  elif .kind == "PairIdRefLiteralInteger" then
    "pair<u32_t, i32_t>"
  else
    "enum " + (.kind | gen::snake_case)
  end;

def emit_operand_field(name):
  if .kind == "Decoration" then
    "enum decoration \(name);", "u32_t m_\(name)_args[1];"
  elif .quantifier? == "*" or .kind == "LiteralString" then
    (. | operand_type) + " m_\(name)[1];"
  elif .quantifier? == "?" then
    (. | operand_type) + " m_\(name);"
  elif name == "default" then
    (. | operand_type) + " default_;"
  else
    (. | operand_type) + " \(name);"
  end;


def emit_operand_getter(name; index):
  if .quantifier? == "*" then
    [
      "[[nodiscard]] span<const \(. | operand_type)> \(name)() const noexcept {",
      "  assert_length(instruction, \(index));",
      "  return span<const \(. | operand_type)>(m_\(name), instruction.word_count() - \(index + 1));",
      "}"
    ]
  elif .kind == "LiteralString" then
    [
      "J_INLINE_GETTER const char * \(name)() const noexcept {",
      "  return instruction.word_count() <= \(index + 1) ? \"\" : reinterpret_cast<const char *>(&m_\(name)[0]);",
      "}"
    ]
  elif .kind == "Decoration" then
    [
      "J_INLINE_GETTER span<const u32_t> decoration_args() const noexcept {",
      "  assert_length(instruction, \(index + 1));",
      "  return span<const u32_t>(m_decoration_args, instruction.word_count() - \(index + 1));",
      "}"
    ]
  elif .quantifier? == "?" then
    [
    "[[nodiscard]] J_INLINE_GETTER " + (. | operand_type) + " \(name)() const noexcept {",
    "  assert_length(instruction, \(index + 1));",
    "  return m_\(name);",
    "}",
    "[[nodiscard]] J_INLINE_GETTER bool has_\(name)() const noexcept {",
    "  return instruction.word_count() > \(index + 1);",
    "}"
  ]
  else
    []
  end;

def insn_class_name:
  "insn_" + gen::remove_prefix("Op") | gen::snake_case;

def emit_opcode_class:
  (.opname | insn_class_name) as $name
  | .operands as $operands
  | if $operands then
      [
        "struct \($name) final {",
        (range(0; $operands|length) | (. as $index | ($operands[$index] | emit_operand_getter(.|operand_name; $index)))),
        "  struct instruction instruction;",
        ($operands | map(emit_operand_field(.|operand_name))),
        "};\n"
      ]
    else
      [
        "struct \($name) final {",
        "  struct instruction instruction;",
        "};\n"
      ]
    end;

def emit_opcode_visitor($name):
  [
    "template<typename Fn>",
    "decltype(auto) \($name)(Fn && fn, const instruction & insn) {",
    "  switch (insn.opcode()) {",
    map(
      (.opname | insn_class_name) as $class_name
      | (.opname | gen::snake_case) as $enum_name
      | [
          "case opcode::\($enum_name):",
          "  return static_cast<Fn &&>(fn)(reinterpret_cast<const \($class_name) &>(insn));"
        ]
    ),
    "  default:",
    "    J_FAIL(\"Unknown opcode.\");",
    "  }",
    "}"
  ];

def emit_opcode_predicate(name):
    (. | max_opcode) as $max_opcode
  | (. | min_opcode) as $min_opcode
  | [
    "inline constexpr bool \(name)(opcode op) noexcept {",
    [
      "constexpr u32_t m[] = {",
      [[. | opcode_masks | gen::hex_dword] | gen::chunk(4; ", ")],
      "};",
      "const u16_t idx = static_cast<u16_t>(op) - \($min_opcode);",
      "return idx <= \($max_opcode - $min_opcode) && (m[idx >> 5] & (1 << (idx & 31)));"
    ],
    "}"
  ];

def emit_instruction_predicate(name):
  [
    "template<typename Insn> J_NO_DEBUG inline constexpr bool \(name) = false;",
    "",
    (.[] | .opname | insn_class_name | "template<> J_NO_DEBUG inline constexpr bool \(name)<\(.)> = true;")
  ];

def emit_opcode_instruction_map(name):
  [
    "template<typename Insn> J_NO_DEBUG inline constexpr opcode \(name) = opcode::op_invalid;",
    map(
      (.opname | insn_class_name) as $class_name
      | (.opname | gen::snake_case) as $enum_name
      | "template<> J_NO_DEBUG inline constexpr opcode \(name)<\($class_name)> = opcode::\($enum_name);"
    )
  ];
