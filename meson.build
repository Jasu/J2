project('j', ['c', 'cpp'], default_options: ['cpp_std=c++2a'])
add_project_arguments(
  '-DJ_SRC_DIR="' + meson.current_source_dir() / 'src"',
  '-DJ_RES_DIR="' + meson.current_source_dir() / 'res"',
  language: ['cpp'])

c_compiler = meson.get_compiler('c')
cpp_compiler = meson.get_compiler('cpp')

is_clang = c_compiler.get_id() == 'clang' and cpp_compiler.get_id() == 'clang'
is_debug = get_option('buildtype') == 'debug' or get_option('buildtype') == 'debugoptimized'
extensive_debug = is_debug and get_option('extensive-debug-build')

if get_option('disable-no-debug')
  add_project_arguments('-DJ_CONFIG_DISABLE_NO_DEBUG', language: ['c', 'cpp'])
endif

if get_option('disable-hidden-visibility')
  add_project_arguments('-DJ_CONFIG_DISABLE_VISIBILITY_HIDDEN', language: ['c', 'cpp'])
elif not extensive_debug
  add_global_arguments('-fvisibility-inlines-hidden', '-fvisibility-inlines-hidden-static-local-var', language: ['c', 'cpp'])
endif

# Linking a static library to a dynamic PIC library -> have to compile with PIC.
# Static libs are not compiled with PIC enabled by default in CMake, though they
# are in Meson. At least the version 0.54.3 does not propagate the build option
# to the subprojects, so set the variable manually.
default_cmake_opts = ['-DCMAKE_POSITION_INDEPENDENT_CODE=True']

add_project_arguments('-I' + join_paths(meson.current_build_dir(), 'generated/'),
                      '-I' + meson.current_source_dir() / 'src/',
                      # '-fdebug-prefix-map=' + meson.current_source_dir() / '/src/=../',
                      language : ['c', 'cpp'])


if get_option('enable-pprof').enabled()
  add_global_arguments('-fno-omit-frame-pointer', language : ['c', 'cpp'])
else
  add_global_arguments('-momit-leaf-frame-pointer', language : ['c', 'cpp'])
endif

add_project_arguments(
  '-fwrapv',

  # Allow C99 array designators
  '-Wno-c99-designator',

  '-Wno-non-virtual-dtor',
  # Emitting warnings for typeid(*shared_ptr) is beyond stupid.
  '-Wno-potentially-evaluated-expression',
  '-Wextra',
  # Extra semi colons
  '-Wextra-semi',
  '-Wunreachable-code',
  '-Wtautological-compare',
  '-Wunused-label',
  '-Wzero-as-null-pointer-constant',
  '-Warray-bounds-pointer-arithmetic',
  '-Wbad-function-cast',
  '-Wimplicit-fallthrough',
  '-Wint-in-bool-context',
  '-Wmisleading-indentation',
  language: ['cpp'])

add_global_link_arguments(
  '-Wl,--no-ld-generated-unwind-info',
  language: ['c', 'cpp'])

add_global_arguments(
  # CPU_SPECIFIC("haswell", 'V', "+cmov,+mmx,+sse,+sse2,+sse3,+ssse3,+sse4.1,+sse4.2,+movbe,+popcnt,+f16c,+avx,+fma,+bmi,+lzcnt,+avx2")
  # Required: POPCNT, BMI, LZCNT, SSE4.2 (for CRC32)
  '-march=tigerlake',
  '-mtune=native',

  '-ftls-model=local-dynamic',
  '-fno-threadsafe-statics',

  '-fmerge-all-constants',
  '-fno-direct-access-external-data',
  '-ffinite-loops',
  '-fno-semantic-interposition',
  '-fzero-initialized-in-bss',
  language : ['c', 'cpp'])

if is_clang
  if cpp_compiler.has_argument('-fpch-instantiate-templates')
    add_project_arguments('-fpch-instantiate-templates', language: ['cpp'])
  endif

  # if cpp_compiler.has_argument('-fpch-codegen')
  #   add_project_arguments('-fpch-codegen', language: ['cpp'])
  # endif


  if is_debug
    if extensive_debug
      add_global_arguments('-fstandalone-debug', language: ['c', 'cpp'])
    else
      add_global_arguments('-gno-column-info', language: ['c', 'cpp'])
      add_project_arguments('-fpch-debuginfo', language: ['cpp'])
    endif

    add_global_arguments(
      #'-ggdb',
      # '-gdwarf-5',
      '-gdwarf-5',
      '-ginline-line-tables',
      # '-Xclang', '-gsimple-template-names=mangled',
      # '-fsplit-dwarf-inlining',
      # '-gsplit-dwarf=split',
      #'-gpubnames',
      # '-gdwarf32',
      language : ['c', 'cpp'])

    if not get_option('enable-pprof').enabled() and not extensive_debug
      add_global_arguments('-Xclang', '-debug-info-kind=constructor', language : ['c', 'cpp'])
    endif

  endif


  add_project_arguments(
    '-fstrict-enums',
    '-fstrict-vtable-pointers',
    '-felide-constructors',

    '-Wextra-semi-stmt',
    '-Wunreachable-code-aggressive',
    '-Wunreachable-code-loop-increment',
    '-Wweak-template-vtables',
    '-Wassign-enum',
    '-Wbitfield-enum-conversion',
    '-Wclass-varargs',
    '-Wconditional-uninitialized',
    '-Wdelete-non-abstract-non-virtual-dtor',
    '-Wduplicate-enum',
    '-Wduplicate-method-match',
    '-Wempty-translation-unit',
    '-Wfor-loop-analysis',
    '-Wheader-hygiene',
    '-Wmissing-variable-declarations',
    '-Wmove',
    '-Wrange-loop-analysis',
    '-Wself-assign-overloaded',
    '-Wunneeded-internal-declaration',
    language: ['cpp'])
endif

if get_option('enable-build-time-tracing').enabled() or (get_option('enable-build-time-tracing').auto() and is_clang)
  add_project_arguments('-ftime-trace', language : ['c', 'cpp'])
endif

if get_option('enable-asan').enabled() or (get_option('enable-asan').auto() and is_clang)
  add_project_arguments('-fsanitize-address-use-after-scope', '-fsanitize=address', '-fno-omit-frame-pointer', language : ['c', 'cpp'])
  add_project_link_arguments('-fsanitize=address', language: ['c', 'cpp'])
endif

if get_option('enable-msan').enabled() or (get_option('enable-msan').auto() and is_clang)
  add_project_arguments('-fsanitize=memory', '-fsanitize-memory-use-after-dtor', '-fno-omit-frame-pointer', language : ['c', 'cpp'])
  add_project_link_arguments('-fsanitize=memory', '-fsanitize-memory-use-after-dtor', language: ['c', 'cpp'])
endif

# System dependencies
# ===================
dependencies = []
dependencies += dependency('xcb')
dependencies += dependency('xcb-xinput')
dependencies += dependency('xcb-xkb')
dependencies += dependency('xcb-errors')
dependencies += dependency('xkbcommon')
dependencies += dependency('xkbcommon-x11')
dependencies += dependency('fontconfig')
dependencies += dependency('freetype2')
dependencies += dependency('harfbuzz')
dependencies += dependency('vulkan')
dependencies += dependency('libunwind-generic')
dependencies += dependency('libuv')
# Meson-provided dependency for adding pthread.
dependencies += dependency('threads')

# Unicode Database
# ================
ucd_grapheme_property_tgt = custom_target(
  'GraphemeBreakProperty.txt',
  input: 'ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'auxiliary/GraphemeBreakProperty.txt'],
  output: 'GraphemeBreakProperty.txt')
ucd_emoji_data_tgt = custom_target(
  'emoji-data.txt',
  input: 'ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'emoji/emoji-data.txt'],
  output: 'emoji-data.txt')
ucd_line_break_tgt = custom_target(
  'LineBreak.txt',
  input: 'ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'LineBreak.txt'],
  output: 'LineBreak.txt')
ucd_east_asian_width_txt_tgt = custom_target(
  'EastAsianWidth.txt',
  input: 'ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'EastAsianWidth.txt'],
  output: 'EastAsianWidth.txt')
ucd_derived_general_category_tgt = custom_target(
  'DerivedGeneralCategory.txt',
  input: 'ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'extracted/DerivedGeneralCategory.txt'],
  output: 'DerivedGeneralCategory.txt')
ucd_grapheme_properties_hpp_tgt = custom_target(
  'grapheme_properties.hpp',
  input: ucd_grapheme_property_tgt,
  depend_files: files('tools/unicode_properties.py'),
  command: [
    'sh',
    '-c',
    'python "$0" "$1" "$2" && mkdir -p "$3/generated/strings/unicode/" && cp "$2" "$3/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_properties.py'),
    '@INPUT@',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'grapheme_properties.hpp')

ucd_east_asian_width_hpp_tgt = custom_target(
  'east_asian_width.hpp',
  input: ucd_east_asian_width_txt_tgt,
  depend_files: files('tools/unicode_east_asian_width.py', 'tools/ucd_utils.py'),
  command: [
    'sh',
    '-c',
    'python "$0" "hpp" "$1" "$2" && mkdir -p "$3/generated/strings/unicode/" && cp "$2" "$3/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_east_asian_width.py'),
    '@INPUT@',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'east_asian_width.hpp')

ucd_east_asian_width_cpp_tgt = custom_target(
  'east_asian_width.cpp',
  input: ucd_east_asian_width_txt_tgt,
  depend_files: files('tools/unicode_east_asian_width.py', 'tools/ucd_utils.py'),
  command: [
    'sh',
    '-c',
    'python "$0" "cpp" "$1" "$2" && mkdir -p "$3/generated/strings/unicode/" && cp "$2" "$3/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_east_asian_width.py'),
    '@INPUT@',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'east_asian_width.cpp')

ucd_emoji_data_hpp_tgt = custom_target(
  'emoji_data.hpp',
  input: ucd_emoji_data_tgt,
  depends: [ucd_emoji_data_tgt],
  depend_files: files('tools/unicode_properties.py'),
  command: [
    'sh',
    '-c',
    'python "$0" "$1" "$2" && mkdir -p "$3/generated/strings/unicode/" && cp "$2" "$3/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_properties.py'),
    '@INPUT@',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'emoji_data.hpp')
ucd_line_break_property_hpp_tgt = custom_target(
  'line_break_property.hpp',
  input: ucd_line_break_tgt,
  depends: [ucd_derived_general_category_tgt],
  depend_files: files('tools/unicode_line_break.py', 'tools/ucd_utils.py'),
  command: [
    'sh',
    '-c',
    'python "$0" hpp "$1" "$2" "$3" && mkdir -p "$4/generated/strings/unicode/" && cp "$3" "$4/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_line_break.py'),
    '@INPUT@',
    ucd_derived_general_category_tgt,
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'line_break_property.hpp')

ucd_line_break_property_cpp_tgt = custom_target(
  'line_break_property.cpp',
  input: ucd_line_break_tgt,
  depends: [ucd_derived_general_category_tgt],
  depend_files: files('tools/unicode_line_break.py', 'tools/ucd_utils.py'),
  command: [
    'sh',
    '-c',
    'python "$0" cpp "$1" "$2" "$3" && mkdir -p "$4/generated/strings/unicode/" && cp "$3" "$4/generated/strings/unicode/"',
    join_paths(meson.current_source_dir(), 'tools/unicode_line_break.py'),
    '@INPUT@',
    ucd_derived_general_category_tgt,
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  output: 'line_break_property.cpp')

# RapidYAML dependency
# ====================
cmake = import('cmake')

rapidyaml_opts = cmake.subproject_options()
rapidyaml_opts.add_cmake_defines({
  'CMAKE_POSITION_INDEPENDENT_CODE': true,
  # Unless specified, C4 will use C++11 without any feature detection.
  'C4_CXX_STANDARD': '20',
  # Disable sanitizers - for some reason, this is enabled but does not seem to have any effect
  # when invoking CMake through Meson.
  'C4_SANITIZE': 'OFF',
  'C4_ASAN': 'OFF',
  'C4_MSAN': 'OFF',
})
rapidyaml_proj = cmake.subproject(
  'rapidyaml',
  options: rapidyaml_opts)
# default_cmake_opts + [
#     # Unless specified, C4 will use C++11 without any feature detection.
#     '-DC4_CXX_STANDARD=20',
#     # Disable sanitizers - for some reason, this is enabled but does not seem to have any effect
#     # when invoking CMake through Meson.
#     '-DC4_SANITIZE=OFF',
#     '-DC4_ASAN=OFF',
#     '-DC4_MSAN=OFF',
#   ])

# c4core_tgt = rapidyaml_proj.target('c4core')
# rapidyaml_tgt = rapidyaml_proj.target('ryml')
# # Don't use the include dirs from the CMake module - this includes lots of unused include paths.
# rapidyaml_inc = include_directories('subprojects/rapidyaml/src',
#                                     'subprojects/rapidyaml/ext/c4core/src',
#                                     'subprojects/rapidyaml/ext/c4core/src/c4/ext',
#                                     is_system: true)
# #'subprojects/rapidyaml/ext/c4core/ext',
# rapidyaml_dep = declare_dependency(
#   link_with: rapidyaml_tgt,
#   include_directories: rapidyaml_inc
# )

dependencies += rapidyaml_proj.dependency('c4core')
dependencies += rapidyaml_proj.dependency('ryml')
# dependencies += zydis_proj.dependency('Zydis')

# dependencies += rapidyaml_dep

# Zydis dependency
# ================

zydis_opts = cmake.subproject_options()
zydis_opts.add_cmake_defines(
  {
    'CMAKE_POSITION_INDEPENDENT_CODE': true,
    'ZYDIS_FEATURE_FORMATTER': 'OFF',
    'ZYDIS_FEATURE_ENCODER': 'OFF',
    'ZYDIS_FEATURE_AVX512': 'OFF',
    'ZYDIS_FEATURE_KNC': 'OFF',
    'ZYDIS_BUILD_EXAMPLES': 'OFF',
    'ZYDIS_BUILD_SHARED_LIB': 'ON',
    'ZYCORE_BUILD_SHARED_LIB': 'ON',
    'ZYDIS_BUILD_TOOLS': 'OFF',
  })
zydis_proj = cmake.subproject('zydis', options: zydis_opts)
dependencies += zydis_proj.dependency('Zycore')
dependencies += zydis_proj.dependency('Zydis')

# Google Double Conversion dependency
# ===================================
double_conversion_inc = include_directories(
  'ext/double-conversion/double-conversion',
  is_system: true
)
double_conversion_lib = library(
  'double-conversion',
  [
    'ext/double-conversion/double-conversion/bignum.cc',
    'ext/double-conversion/double-conversion/bignum-dtoa.cc',
    'ext/double-conversion/double-conversion/cached-powers.cc',
    'ext/double-conversion/double-conversion/double-to-string.cc',
    'ext/double-conversion/double-conversion/fast-dtoa.cc',
    'ext/double-conversion/double-conversion/fixed-dtoa.cc',
    'ext/double-conversion/double-conversion/string-to-double.cc',
    'ext/double-conversion/double-conversion/strtod.cc'
  ],
  include_directories: double_conversion_inc,
  cpp_args: [
    '-Wno-extra-semi-stmt',
    '-Wno-zero-as-null-pointer-constant',
  ]
)
double_conversion_dep = declare_dependency(link_with: double_conversion_lib, include_directories: double_conversion_inc)
dependencies += double_conversion_dep

# Termpaint dependency
# ====================
ttyrescue_nolibc_exe = executable('ttyrescue_nolibc', 'ext/termpaint/ttyrescue_nolibc.c',
  c_args: ['-fno-asynchronous-unwind-tables', '-fno-ident', '-nostdlib', '-static', '-Os',
              '-fvisibility=hidden', '-std=gnu11'],
  link_args: ['-fno-asynchronous-unwind-tables', '-fno-ident', '-nostdlib', '-static', '-Os',
              '-fvisibility=hidden', '-std=gnu11'])

ttyrescue_nolibc_stripped = custom_target('ttyrescue_nolibc_stripped',
  input: ttyrescue_nolibc_exe,
  output: 'ttyrescue_nolibc_stripped',
  command: [find_program('strip'), '--strip-all', '--remove-section=.comment',
            '--remove-section=.note', '--remove-section=.eh_frame_hdr',
            '--remove-section=.eh_frame', '--remove-section=.note.gnu.gold-version',
            '--remove-section=.note.gnu.build-id',
            '@INPUT@', '-o', '@OUTPUT@'])

ttyrescue_nolibc_inc = custom_target('ttyrescue_nolibc_inc',
  input: ttyrescue_nolibc_stripped,
  output: ['ttyrescue_nolibc.inc'],
  command: [find_program('./ext/termpaint/filetoinc.py'), 'ttyrescue_blob', '@INPUT@', '@OUTPUT0@'])

termpaint_cflags = [
  '-DTERMPAINT_RESCUE_FEXEC',
  '-DTERMPAINT_EXPORT_SYMBOLS',
  '-DTERMPAINT_RESCUE_EMBEDDED',
  '-DTERMPAINT_RESCUE_PATH="/usr/libexec"',
  '-Wno-string-plus-int'
]
termpaint_files = [
  'ext/termpaint/termpaint.c',
  'ext/termpaint/termpaint_event.c',
  'ext/termpaint/termpaint_input.c',
  'ext/termpaint/termpaintx.c',
  'ext/termpaint/termpaintx_ttyrescue.c',
  'ext/termpaint/ttyrescue.c',
  ttyrescue_nolibc_inc,
]
termpaint_inc = include_directories('./ext/termpaint/')
termpaint_lib = library(
  'termpaint', termpaint_files,
  include_directories: termpaint_inc,
  dependencies: c_compiler.find_library('rt', required : false),
  c_args: termpaint_cflags)

dependencies += declare_dependency(
  include_directories: termpaint_inc,
  link_with: termpaint_lib
)

# Shaders
# =======
shaders = [
  'res/box_renderer.vert', 'res/box_renderer.frag',
  'res/text_renderer.vert', 'res/text_renderer.frag',
  'res/color_space_slider.vert', 'res/color_space_slider.frag',
  'res/color_space_preview.frag',
]

glslc = find_program('glslc', required: true)
foreach shader : shaders
custom_target(
  shader.underscorify(),
    input: shader,
    output: '@PLAINNAME@.spv',
    command: [
      glslc,
      '@INPUT@',
      '-mfmt=bin',
      '--target-env=vulkan1.1',
      '-fentry-point=main',
      '-O',
      '-g',
      '-o', '@OUTPUT@',
    ],
    install: true,
    install_dir: '.'
  )
endforeach

# NASM generator (for Assembly test fixtures)
# ===========================================
nasm = find_program('nasm', required: true)
nasm_gen = generator(
  nasm,
  output: '@BASENAME@.o',
  arguments:
    ['-i', join_paths(meson.current_source_dir(), 'test/lisp/assembly'),
     '-f', 'elf64',
     '-O0',
     '@INPUT@',
     '-o', '@OUTPUT@'
    ]
)


# SPIR-V generated headers
# ========================
jq = find_program('jq', required: true)
spirv_enums = custom_target(
  'spirv_enums.hpp',
  input: 'ext/SPIRV-Headers/include/spirv/unified1/spirv.core.grammar.json',
  output: 'spirv-enums.hpp',
  command: [
    'sh',
    '-c',
    'jq -r -L ' + join_paths(meson.current_source_dir(), 'tools') + ' -f ' + join_paths(meson.current_source_dir(), 'tools/spirv_enums.jq') + ' @INPUT@ >"$0"'
    + ' && mkdir -p "$1/generated/rendering/vulkan/spirv/"'
    + ' && cp "$0" "$1/generated/rendering/vulkan/spirv/enums.hpp"',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  depend_files: files('tools/spirv_enums.jq', 'tools/spirv.jq', 'tools/gen.jq'),
)

spirv_formatters = custom_target(
  'spirv_formatters.cpp',
  input: 'ext/SPIRV-Headers/include/spirv/unified1/spirv.core.grammar.json',
  output: 'spirv_formatters.cpp',
  capture: true,
  command: [
    jq,
    '-r',
    '-L', join_paths(meson.current_source_dir(), 'tools'),
    '-f', join_paths(meson.current_source_dir(), 'tools/spirv_formatters.jq'),
    '@INPUT@'
  ],
  depend_files: files('tools/spirv_formatters.jq', 'tools/spirv.jq', 'tools/gen.jq'),
)

spirv_types = custom_target(
  'spirv_types.hpp',
  input: 'ext/SPIRV-Headers/include/spirv/unified1/spirv.core.grammar.json',
  output: 'spirv-types.hpp',
  command: [
    'sh',
    '-c',
    'jq -r -L ' + join_paths(meson.current_source_dir(), 'tools') + ' -f ' + join_paths(meson.current_source_dir(), 'tools/spirv_types.jq') + ' @INPUT@ >"$0"'
    + ' && mkdir -p "$1/generated/rendering/vulkan/spirv/"'
    + ' && cp "$0" "$1/generated/rendering/vulkan/spirv/types.hpp"',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  depend_files: files('tools/spirv_types.jq', 'tools/spirv.jq', 'tools/gen.jq'),
)

spirv_instructions = custom_target(
  'spirv_instructions.hpp',
  input: 'ext/SPIRV-Headers/include/spirv/unified1/spirv.core.grammar.json',
  output: 'spirv-instructions.hpp',
  command: [
    'sh',
    '-c',
    'jq -r -L ' + join_paths(meson.current_source_dir(), 'tools') + ' -f ' + join_paths(meson.current_source_dir(), 'tools/spirv_instructions.jq') + ' @INPUT@ >"$0"'
    + ' && mkdir -p "$1/generated/rendering/vulkan/spirv/"'
    + ' && cp "$0" "$1/generated/rendering/vulkan/spirv/instructions.hpp"',
    '@OUTPUT@',
    '@OUTDIR@',
  ],
  depend_files: files('tools/spirv_instructions.jq', 'tools/spirv.jq', 'tools/gen.jq'),
)

# Expr def reader
# ===============
re2c = find_program('re2c', required: true)
expr_reader = custom_target(
  'read_expr.cpp',
  input: 'src/meta/expr.re.cpp',
  output: 'expr.rd.cpp',
  command: [
    'sh', '-c',
    're2c -Wempty-character-class -Wmatch-empty-string -Wswapped-range -Wundefined-control-flow -Wnondeterministic-tags -Wuseless-escape --case-ranges "$0" -o "$1"',
    '@INPUT@', '@OUTPUT@',
  ],
)
meta_reader = custom_target(
  'read_meta.cpp',
  input: 'src/meta/meta.re.cpp',
  output: 'meta.rd.cpp',
  command: [
    'sh', '-c',
    're2c -Wempty-character-class -Wmatch-empty-string -Wswapped-range -Wundefined-control-flow -Wnondeterministic-tags -Wuseless-escape --case-ranges "$0" -o "$1"',
    '@INPUT@', '@OUTPUT@'
  ],
)

rule_reader = custom_target(
  'parse_rule_re.cpp',
  input: 'src/meta/rules/parse_rule.re.cpp',
  output: 'parse_rule_re.cpp',
  command: [
    'sh', '-c',
    're2c -Wempty-character-class -Wmatch-empty-string -Wswapped-range -Wundefined-control-flow -Wnondeterministic-tags -Wuseless-escape --case-ranges "$0" -o "$1"',
    '@INPUT@', '@OUTPUT@'
  ],
)

attr_value_reader = custom_target(
  'read_attr_value.cpp',
  input: 'src/meta/attr_value.re.cpp',
  output: 'read_attr_value.cpp',
  command: [
    'sh', '-c',
    'mkdir -p "$2/generated/meta/" && re2c -Wempty-character-class -Wmatch-empty-string -Wswapped-range -Wundefined-control-flow -Wnondeterministic-tags -Wuseless-escape --case-ranges "$0" -o "$1"',
    '@INPUT@', '@OUTPUT@', '@OUTDIR@'
  ],
)

# Lisp reader
# ===========
lisp_reader = custom_target(
  'read.cpp',
  input: 'src/lisp/reader/read.re.cpp',
  output: 'read.cpp',
  command: [
    'sh', '-c',
    'mkdir -p "$2/generated/lisp/reader" && re2c --case-ranges --reusable -f -ci "$0" -o "$1" -t "$2/generated/lisp/reader/read_types.hpp"',
    '@INPUT@', '@OUTPUT@', '@OUTDIR@'
  ],
)

# Regex reader
# ============
regex_reader = custom_target(
  'read_regex.cpp',
  input: 'src/strings/regex/read.re.cpp',
  output: 'read_regex.cpp',
  command: [re2c, '--utf8 --reusable', '@INPUT@', '-o', '@OUTPUT@'],
)

# /proc/*/maps reader
# ===================
maps_reader = custom_target(
  'maps.re.cpp',
  input: 'src/mem/maps.re.cpp',
  output: 'maps.cpp',
  command: [re2c, '--reusable', '@INPUT@', '-o', '@OUTPUT@'],
)

# J Library sources
# =================

src = []

src += lisp_reader
src += expr_reader
src += meta_reader
src += rule_reader
src += attr_value_reader
src += regex_reader
src += maps_reader
src += spirv_enums
src += spirv_types
src += spirv_formatters
src += spirv_instructions
src += ucd_grapheme_properties_hpp_tgt
src += ucd_emoji_data_hpp_tgt
src += ucd_line_break_property_hpp_tgt
src += ucd_line_break_property_cpp_tgt
src += ucd_east_asian_width_hpp_tgt
src += ucd_east_asian_width_cpp_tgt

src += 'src/ansi/sgr_styled_sink.cpp'

src += 'src/attributes/attribute_map.cpp'

src += 'src/backtrace/backtrace.cpp'
src += 'src/backtrace/dump_backtrace.cpp'
src += 'src/backtrace/stored_backtrace.cpp'

src += 'src/colors/color_registry.cpp'
src += 'src/colors/rgb.cpp'

src += 'src/cli/application.cpp'
src += 'src/cli/arguments.cpp'
src += 'src/cli/cli_configuration_pass.cpp'
src += 'src/cli/command.cpp'
src += 'src/cli/command_definition.cpp'
src += 'src/cli/help_command.cpp'
src += 'src/cli/parse.cpp'

src += 'src/commands/dump_cir.cpp'
src += 'src/commands/meta.cpp'
src += 'src/commands/repl.cpp'
src += 'src/commands/termpaint_test.cpp'
src += 'src/commands/test_color_spaces.cpp'
src += 'src/commands/test_elf.cpp'
src += 'src/commands/test_heap.cpp'
src += 'src/commands/test_regex.cpp'

src += 'src/containers/common.cpp'
src += 'src/containers/detail/chunk.cpp'
src += 'src/containers/detail/chunk2.cpp'
src += 'src/containers/detail/resizeable_region.cpp'
src += 'src/containers/hash_map.cpp'
src += 'src/containers/obstack.cpp'
src += 'src/containers/trees/binary_tree_node.cpp'
src += 'src/containers/ptr_set.cpp'

src += 'src/debug/counters.cpp'
src += 'src/debug/timing.cpp'

src += 'src/events/assert.cpp'
src += 'src/events/debug_formatters.cpp'
src += 'src/events/event_loop.cpp'
src += 'src/events/handle.cpp'
src += 'src/events/poll_handle.cpp'
src += 'src/events/prepare_handle.cpp'
src += 'src/events/requests.cpp'
src += 'src/events/request_queue.cpp'
src += 'src/events/signal_handle.cpp'
src += 'src/events/stream_handle.cpp'

src += 'src/exceptions/exceptions.cpp'
src += 'src/exceptions/formatters.cpp'
src += 'src/exceptions/terminate_handler.cpp'

src += 'src/files/fs.cpp'
src += 'src/files/formatters.cpp'
src += 'src/files/memory_mapping.cpp'
src += 'src/files/ofile.cpp'

src += 'src/files/fds/adopt.cpp'
src += 'src/files/fds/fd.cpp'
src += 'src/files/fds/fd_info.cpp'
src += 'src/files/fds/file_type.cpp'
src += 'src/files/fds/io.cpp'
src += 'src/files/fds/open.cpp'
src += 'src/files/fds/standard_streams.cpp'
src += 'src/files/fds/stat.cpp'

src += 'src/files/dirs/dir.cpp'

src += 'src/files/paths/path.cpp'

src += 'src/fonts/font.cpp'

src += 'src/fonts/rasterizing/font_rasterizer.cpp'
src += 'src/fonts/rasterizing/font_image_source_handler.cpp'
src += 'src/fonts/rasterizing/rasterizer_font.cpp'

src += 'src/fonts/rasterizing/freetype/ft_error_formatter.cpp'
src += 'src/fonts/rasterizing/freetype/ft_rasterizer.cpp'
src += 'src/fonts/rasterizing/freetype/ft_font.cpp'
src += 'src/fonts/rasterizing/freetype/ft_sized_font.cpp'

src += 'src/fonts/selection/flags.cpp'
src += 'src/fonts/selection/font_selector.cpp'

src += 'src/fonts/selection/fontconfig/fc_font_selector.cpp'

src += 'src/fonts/shaping/shaper.cpp'
src += 'src/fonts/shaping/shaped_string.cpp'

src += 'src/fonts/utils/test_selector.cpp'
src += 'src/fonts/utils/test_shaping.cpp'

src += 'src/geometry/vec_formatters.cpp'
src += 'src/geometry/perimeter_formatters.cpp'
src += 'src/geometry/rect_formatters.cpp'

src += 'src/graphviz/common.cpp'
src += 'src/graphviz/common_attributes.cpp'
src += 'src/graphviz/edge_attributes.cpp'
src += 'src/graphviz/graph.cpp'
src += 'src/graphviz/graph_attributes.cpp'
src += 'src/graphviz/node_attributes.cpp'
src += 'src/graphviz/serialization.cpp'
src += 'src/graphviz/table.cpp'

src += 'src/hzd/mem.cpp'
src += 'src/hzd/demangle.cpp'
src += 'src/hzd/amd64/cpuid.cpp'

src += 'src/input/formatters.cpp'

src += 'src/lisp/ast_to_air/ast_to_air.cpp'

src += 'src/lisp/air/air_package.cpp'

src += 'src/lisp/common/debug_formatters.cpp'
src += 'src/lisp/common/metadata.cpp'
src += 'src/lisp/common/metadata_init.cpp'
src += 'src/lisp/common/operand_name.cpp'

src += 'src/lisp/air/debug/expr_formatter.cpp'
src += 'src/lisp/air/debug/graphviz_dump.cpp'
src += 'src/lisp/air/debug/graphviz_format.cpp'
src += 'src/lisp/air/debug/debug_info_map.cpp'
src += 'src/lisp/air/debug/closure_formatters.cpp'
src += 'src/lisp/air/debug/value_formatters.cpp'

src += 'src/lisp/air/exprs/expr.cpp'
src += 'src/lisp/air/exprs/expr_data.cpp'
src += 'src/lisp/air/exprs/expr_info.cpp'
src += 'src/lisp/air/exprs/expr_size_data.cpp'
src += 'src/lisp/air/exprs/expr_type.cpp'
src += 'src/lisp/air/exprs/visit.cpp'
src += 'src/lisp/air/exprs/write_expr.cpp'

src += 'src/meta/attr_value.cpp'
src += 'src/meta/attr_value_hash.cpp'
src += 'src/meta/attr_value_interface.cpp'
src += 'src/meta/attr_value_interfaces.cpp'
src += 'src/meta/attr_context.cpp'
src += 'src/meta/attrs.cpp'
src += 'src/meta/basic_node_set.cpp'
src += 'src/meta/code_writer.cpp'
src += 'src/meta/comments.cpp'
src += 'src/meta/cpp_codegen.cpp'
src += 'src/meta/dump.cpp'
src += 'src/meta/errors.cpp'
src += 'src/meta/expr.cpp'
src += 'src/meta/expr_fn.cpp'
src += 'src/meta/expr_scope.cpp'
src += 'src/meta/fn.cpp'
src += 'src/meta/formatters.cpp'
src += 'src/meta/gen_file.cpp'
src += 'src/meta/module.cpp'
src += 'src/meta/node.cpp'
src += 'src/meta/term.cpp'
src += 'src/meta/term_member.cpp'
src += 'src/meta/value_types.cpp'

src += 'src/meta/rules/aliases.cpp'
src += 'src/meta/rules/ambient_patterns.cpp'
src += 'src/meta/rules/args.cpp'
src += 'src/meta/rules/dump.cpp'
src += 'src/meta/rules/grammar.cpp'
src += 'src/meta/rules/graphviz_dump.cpp'
src += 'src/meta/rules/id_resolver.cpp'
src += 'src/meta/rules/lifecycle.cpp'
src += 'src/meta/rules/meta_interface.cpp'
src += 'src/meta/rules/nonterminal.cpp'
src += 'src/meta/rules/nt_data_type.cpp'
src += 'src/meta/rules/parse_rule.cpp'
src += 'src/meta/rules/parse_test_case.cpp'
src += 'src/meta/rules/pat_item.cpp'
src += 'src/meta/rules/pat_set.cpp'
src += 'src/meta/rules/pattern_tree.cpp'
src += 'src/meta/rules/reduction.cpp'
src += 'src/meta/rules/terminals.cpp'
src += 'src/meta/rules/term_expr.cpp'
src += 'src/meta/rules/test_case.cpp'
src += 'src/meta/rules/test_id_resolver.cpp'

src += 'src/os/elf_reader.cpp'
src += 'src/os/dwarf_reader.cpp'
src += 'src/os/dwarf/die_ctx.cpp'
src += 'src/os/dwarf/die_model.cpp'
src += 'src/os/dwarf/file_ctx.cpp'
src += 'src/os/dwarf/unit_ctx.cpp'

src += 'src/lisp/air/passes/act_rec_compute_pass.cpp'
src += 'src/lisp/air/passes/act_rec_convert_pass.cpp'
src += 'src/lisp/air/passes/air_pass.cpp'
src += 'src/lisp/air/passes/basic_validation_pass.cpp'
src += 'src/lisp/air/passes/const_fold.cpp'
src += 'src/lisp/air/passes/convert_reps_pass.cpp'
src += 'src/lisp/air/passes/count_exprs.cpp'
src += 'src/lisp/air/passes/dce_pass.cpp'
src += 'src/lisp/air/passes/lex_vars_pass.cpp'
src += 'src/lisp/air/passes/reaching_defs_pass.cpp'
src += 'src/lisp/air/passes/propagate_barriers_pass.cpp'
src += 'src/lisp/air/passes/propagate_reps_pass.cpp'
src += 'src/lisp/air/passes/propagate_types_pass.cpp'
src += 'src/lisp/air/passes/reassociate_pass.cpp'
src += 'src/lisp/air/passes/simplify_pass.cpp'
src += 'src/lisp/air/passes/trs_write_expr.cpp'
src += 'src/lisp/air/passes/trs_simplify_pass.cpp'

src += 'src/lisp/air/values/rep.cpp'
 #src += 'src/lisp/air/values/val_spec.cpp'

src += 'src/lisp/air_to_cir/air_to_cir.cpp'

src += 'src/lisp/assembly/asm_context.cpp'
src += 'src/lisp/assembly/compiler.cpp'
src += 'src/lisp/assembly/debug_formatters.cpp'
src += 'src/lisp/assembly/operand.cpp'
src += 'src/lisp/assembly/target.cpp'

src += 'src/lisp/assembly/buffers/buffer.cpp'
src += 'src/lisp/assembly/buffers/buffer_builder.cpp'

src += 'src/lisp/assembly/disassembly/disassembler.cpp'
src += 'src/lisp/assembly/disassembly/formatters.cpp'

src += 'src/lisp/assembly/functions/addr_info.cpp'
src += 'src/lisp/assembly/functions/func_info.cpp'
src += 'src/lisp/assembly/functions/func_info_builder.cpp'
src += 'src/lisp/assembly/functions/function_builder.cpp'
src += 'src/lisp/assembly/functions/trap_handler.cpp'

src += 'src/lisp/assembly/linking/reloc.cpp'

src += 'src/lisp/assembly/amd64/amd64_compiler.cpp'
src += 'src/lisp/assembly/amd64/debug_formatters.cpp'
src += 'src/lisp/assembly/amd64/emit.cpp'
src += 'src/lisp/assembly/amd64/amd64_target.cpp'
src += 'src/lisp/assembly/amd64/amd64_disassembler.cpp'

src += 'src/lisp/assembly/amd64/abi/ia64_target.cpp'

src += 'src/lisp/cir/cir_context.cpp'

src += 'src/lisp/cir/debug/graphviz_dump.cpp'
src += 'src/lisp/cir/debug/input_formatter.cpp'
src += 'src/lisp/cir/debug/live_range_dump.cpp'
src += 'src/lisp/cir/debug/live_range_formatters.cpp'
src += 'src/lisp/cir/debug/op_formatter.cpp'
src += 'src/lisp/cir/debug/use_formatter.cpp'

src += 'src/lisp/cir/liveness/live_range.cpp'
src += 'src/lisp/cir/liveness/live_ranges.cpp'
src += 'src/lisp/cir/liveness/live_range_tracker.cpp'

src += 'src/lisp/cir/locs/debug_formatters.cpp'
src += 'src/lisp/cir/locs/reg_map.cpp'

src += 'src/lisp/cir/ops/input.cpp'
src += 'src/lisp/cir/ops/op.cpp'
src += 'src/lisp/cir/ops/op_data.cpp'
src += 'src/lisp/cir/ops/use.cpp'

src += 'src/lisp/cir/passes/cir_pass.cpp'
src += 'src/lisp/cir/passes/def_use_pass.cpp'
src += 'src/lisp/cir/passes/full_call_pass.cpp'
src += 'src/lisp/cir/passes/legalize_pass.cpp'
src += 'src/lisp/cir/passes/number_ops_pass.cpp'
src += 'src/lisp/cir/passes/remove_empty_bbs.cpp'
src += 'src/lisp/cir/passes/reverse_postorder.cpp'

src += 'src/lisp/cir/reg_alloc/reg_alloc.cpp'
src += 'src/lisp/cir/reg_alloc/resolve_pass.cpp'
src += 'src/lisp/cir/reg_alloc/loc_manager.cpp'

src += 'src/lisp/cir/ssa/bb_builder.cpp'
src += 'src/lisp/cir/ssa/ssa_builder.cpp'

src += 'src/lisp/cir/target/amd64.cpp'
src += 'src/lisp/cir/target/cir_target.cpp'

src += 'src/lisp/cir_to_mc/cir_to_mc.cpp'

src += 'src/lisp/compilation/compilation_errors.cpp'
src += 'src/lisp/compilation/compilation_error_formatter.cpp'
src += 'src/lisp/compilation/compile_defun.cpp'
src += 'src/lisp/compilation/compile_symbol.cpp'
src += 'src/lisp/compilation/compile_var.cpp'
src += 'src/lisp/compilation/const_table.cpp'
src += 'src/lisp/compilation/context.cpp'
src += 'src/lisp/compilation/link_function.cpp'
src += 'src/lisp/compilation/macroexpand.cpp'
src += 'src/lisp/compilation/pkg_context.cpp'
src += 'src/lisp/compilation/source_compilation.cpp'

src += 'src/lisp/env/debug_formatters.cpp'
src += 'src/lisp/env/compilation_options.cpp'
src += 'src/lisp/env/context.cpp'
src += 'src/lisp/env/environment.cpp'
src += 'src/lisp/env/env_compiler.cpp'
src += 'src/lisp/env/name_table.cpp'
src += 'src/lisp/env/static_ids.cpp'

src += 'src/lisp/mem/debug_formatters.cpp'
src += 'src/lisp/mem/heap.cpp'
src += 'src/lisp/mem/heap_arena.cpp'
src += 'src/lisp/mem/heap_ctrl_value.cpp'

src += 'src/lisp/mem/code_heap/code_heap.cpp'
src += 'src/lisp/mem/code_heap/subheap.cpp'

# src += 'src/lisp/packages/load_fn_builder.cpp'
src += 'src/lisp/packages/pkg.cpp'
src += 'src/lisp/packages/package_import_table.cpp'
src += 'src/lisp/packages/symbol_table.cpp'
src += 'src/lisp/packages/parse_import.cpp'
src += 'src/lisp/packages/parse_tlf.cpp'

src += 'src/lisp/reader/build_ast.cpp'
src += 'src/lisp/reader/formatters.cpp'
src += 'src/lisp/reader/state.cpp'
src += 'src/lisp/reader/interactive_reader.cpp'

src += 'src/lisp/sources/debug_formatters.cpp'
src += 'src/lisp/sources/source.cpp'
src += 'src/lisp/sources/source_manager.cpp'
src += 'src/lisp/sources/include_dirs.cpp'

src += 'src/lisp/symbols/symbol.cpp'

src += 'src/lisp/values/debug_formatters.cpp'
src += 'src/lisp/values/equality.cpp'
src += 'src/lisp/values/imm_type.cpp'
src += 'src/lisp/values/lisp_object.cpp'
src += 'src/lisp/values/type_formatters.cpp'

src += 'src/math/fixed_formatters.cpp'
src += 'src/math/interval_formatters.cpp'

src += 'src/mem/any.cpp'
src += 'src/mem/mem.cpp'
src += 'src/mem/memmap.cpp'
src += 'src/mem/buffer_builder.cpp'
src += 'src/mem/bump_pool.cpp'
src += 'src/mem/page_range.cpp'
src += 'src/mem/page_pool.cpp'
src += 'src/mem/ring_buffer.cpp'
src += 'src/mem/rw_buffer.cpp'
src += 'src/mem/shared_holder.cpp'

src += 'src/mem/debug/dump_memory_map.cpp'
src += 'src/mem/debug/region_formatters.cpp'
src += 'src/mem/debug/memory_mapping_info_formatter.cpp'
src += 'src/mem/debug/styles.cpp'

src += 'src/mem/vmem/vmem.cpp'

src += 'src/misc/dump_phdr.cpp'

src += 'src/os/dl.cpp'

src += 'src/processes/process_info.cpp'

src += 'src/properties/assert.cpp'
src += 'src/properties/access_by_path.cpp'
src += 'src/properties/property_variant.cpp'
src += 'src/properties/visit_variant.cpp'
src += 'src/properties/path.cpp'

src += 'src/properties/access/bool_access.cpp'
src += 'src/properties/access/default_bool_accesses.cpp'
src += 'src/properties/access/default_float_accesses.cpp'
src += 'src/properties/access/default_int_accesses.cpp'
src += 'src/properties/access/default_list_accesses.cpp'
src += 'src/properties/access/default_map_accesses.cpp'
src += 'src/properties/access/default_nil_accesses.cpp'
src += 'src/properties/access/default_string_accesses.cpp'
src += 'src/properties/access/enum_access.cpp'
src += 'src/properties/access/float_access.cpp'
src += 'src/properties/access/int_access.cpp'
src += 'src/properties/access/list_access.cpp'
src += 'src/properties/access/list_access_definition.cpp'
src += 'src/properties/access/map_access_definition.cpp'
src += 'src/properties/access/registry.cpp'
src += 'src/properties/access/string_access.cpp'
src += 'src/properties/access/set_access.cpp'
src += 'src/properties/access/set_access_definition.cpp'
src += 'src/properties/access/typed_access.cpp'

src += 'src/properties/classes/object_access.cpp'

src += 'src/properties/conversions/conversion_collection.cpp'
src += 'src/properties/conversions/conversion.cpp'

src += 'src/properties/detail/value.cpp'

src += 'src/properties/formatters/path.cpp'
src += 'src/properties/formatters/visit_path.cpp'

src += 'src/properties/visiting/detail.cpp'
src += 'src/properties/visiting/visit_path.cpp'

src += 'src/properties/wrappers/reference_wrapper_registration.cpp'
src += 'src/properties/wrappers/variant_wrapper.cpp'
src += 'src/properties/wrappers/stored_wrapper.cpp'
src += 'src/properties/wrappers/wrapper.cpp'
src += 'src/properties/wrappers/wrapper_definition.cpp'

src += 'src/serialization/backend.cpp'

src += 'src/serialization/yaml/csubstr.cpp'
src += 'src/serialization/yaml/yaml_backend.cpp'
src += 'src/serialization/yaml/value_wrapper.cpp'

src += 'src/services/container.cpp'
src += 'src/services/exceptions.cpp'
src += 'src/services/metadata.cpp'
src += 'src/services/services.cpp'
src += 'src/services/service_definition.cpp'
src += 'src/services/service_reference.cpp'
src += 'src/services/stored_service_reference.cpp'

src += 'src/services/arg/service_argument.cpp'

src += 'src/services/configuration/configuration_overlay.cpp'
src += 'src/services/configuration/configuration_pass.cpp'
src += 'src/services/configuration/configuration_union.cpp'
src += 'src/services/configuration/file_configuration_pass.cpp'
src += 'src/services/configuration/implementation_configuration.cpp'
src += 'src/services/configuration/interface_configuration.cpp'
src += 'src/services/configuration/static_configuration_pass.cpp'

src += 'src/services/container/interface_container.cpp'
src += 'src/services/container/interface_state.cpp'
src += 'src/services/container/service_instance_state.cpp'
src += 'src/services/container/service_type.cpp'
src += 'src/services/container/service_type_container.cpp'

src += 'src/services/detail/construction_handler.cpp'
src += 'src/services/detail/dependencies_t.cpp'

src += 'src/services/registry/registry.cpp'

src += 'src/tags/formatter.cpp'
src += 'src/tags/tag.cpp'
src += 'src/tags/tag_container.cpp'

src += 'src/logging/logging.cpp'
src += 'src/logging/severity.cpp'

src += 'src/strings/escape.cpp'
src += 'src/strings/format.cpp'
src += 'src/strings/string.cpp'
src += 'src/strings/string_set.cpp'

src += 'src/strings/formatters/boolean.cpp'
src += 'src/strings/formatters/chars.cpp'
src += 'src/strings/formatters/debug_enum_formatter.cpp'
src += 'src/strings/formatters/enum_formatter.cpp'
src += 'src/strings/formatters/floats.cpp'
src += 'src/strings/formatters/hex_string.cpp'
src += 'src/strings/formatters/indent.cpp'
src += 'src/strings/formatters/integers.cpp'
src += 'src/strings/formatters/pointers.cpp'
src += 'src/strings/formatters/rainbow.cpp'
src += 'src/strings/formatters/strings.cpp'
src += 'src/strings/formatters/type_ids.cpp'

src += 'src/strings/formatting/context.cpp'
src += 'src/strings/formatting/formatted_sink.cpp'
src += 'src/strings/formatting/formatter.cpp'
src += 'src/strings/formatting/formatter_registry.cpp'
src += 'src/strings/formatting/number_format.cpp'
src += 'src/strings/formatting/pad.cpp'
src += 'src/strings/formatting/parse_push_style.cpp'
src += 'src/strings/formatting/sink_formatter.cpp'

src += 'src/strings/parsing/parse.cpp'
src += 'src/strings/parsing/parse_int.cpp'
src += 'src/strings/parsing/parse_float.cpp'
src += 'src/strings/parsing/parser.cpp'
src += 'src/strings/parsing/parser_registry.cpp'

src += 'src/strings/regex/ast.cpp'
src += 'src/strings/regex/char_class.cpp'
src += 'src/strings/regex/graphviz.cpp'
src += 'src/strings/regex/nfa.cpp'

src += 'src/strings/styling/style.cpp'
src += 'src/strings/styling/styled_sink.cpp'
src += 'src/strings/styling/styled_string.cpp'
src += 'src/strings/styling/styled_string_sink.cpp'
src += 'src/strings/styling/style_registry.cpp'
src += 'src/strings/styling/string_style.cpp'

src += 'src/strings/unicode/debug_formatters.cpp'

src += 'src/strings/unicode/rope/landmark_array.cpp'
src += 'src/strings/unicode/rope/rope_utf8.cpp'
src += 'src/strings/unicode/rope/rope_utf8_view.cpp'
src += 'src/strings/unicode/rope/rope_utf8_value.cpp'
src += 'src/strings/unicode/rope/debug_formatters.cpp'

src += 'src/rendering/data_types/formatter.cpp'

src += 'src/rendering/images/image_buffer_copy.cpp'
src += 'src/rendering/images/image_buffer_view.cpp'
src += 'src/rendering/images/image_source_handler.cpp'
src += 'src/rendering/images/image_source_manager.cpp'
src += 'src/rendering/images/static_image_source_handler.cpp'

src += 'src/rendering/renderers/text/draw_string_command.cpp'
src += 'src/rendering/renderers/text/text_renderer.cpp'
src += 'src/rendering/renderers/text/text_renderer_string.cpp'
src += 'src/rendering/renderers/text/text_vertex_data_source_handler.cpp'

src += 'src/rendering/renderers/box/box_renderer.cpp'
src += 'src/rendering/renderers/box/box_instance_data_source_handler.cpp'

src += 'src/rendering/texture_atlas/row_allocator.cpp'
src += 'src/rendering/texture_atlas/texture_atlas.cpp'

src += 'src/rendering/vertex_data/vertex_data_source_manager.cpp'
src += 'src/rendering/vertex_data/static_vertex_data_source_handler.cpp'
src += 'src/rendering/vertex_data/vertex_binding_info.cpp'

src += 'src/rendering/vulkan/configuration.cpp'
src += 'src/rendering/vulkan/dump.cpp'
src += 'src/rendering/vulkan/exception.cpp'
src += 'src/rendering/vulkan/instance.cpp'
src += 'src/rendering/vulkan/device.cpp'
src += 'src/rendering/vulkan/utils.cpp'

src += 'src/rendering/vulkan/attributes/descriptor_set_layout_record.cpp'
src += 'src/rendering/vulkan/attributes/vertex_input_state_attributes.cpp'

src += 'src/rendering/vulkan/buffers/base_host_visible_buffer.cpp'
src += 'src/rendering/vulkan/buffers/raw_buffer.cpp'
src += 'src/rendering/vulkan/buffers/suballocation.cpp'

src += 'src/rendering/vulkan/command_buffers/begin_render_pass_command.cpp'
src += 'src/rendering/vulkan/command_buffers/bind_descriptor_sets_command.cpp'
src += 'src/rendering/vulkan/command_buffers/bind_pipeline_command.cpp'
src += 'src/rendering/vulkan/command_buffers/bind_vertex_buffers_command.cpp'
src += 'src/rendering/vulkan/command_buffers/command_buffer.cpp'
src += 'src/rendering/vulkan/command_buffers/command_buffer_builder.cpp'
src += 'src/rendering/vulkan/command_buffers/command_buffer_state.cpp'
src += 'src/rendering/vulkan/command_buffers/command_context.cpp'
src += 'src/rendering/vulkan/command_buffers/copy_from_buffer_to_image_command.cpp'
src += 'src/rendering/vulkan/command_buffers/draw_command.cpp'
src += 'src/rendering/vulkan/command_buffers/draw_command_base.cpp'
src += 'src/rendering/vulkan/command_buffers/draw_indirect_count_command.cpp'
src += 'src/rendering/vulkan/command_buffers/end_render_pass_command.cpp'
src += 'src/rendering/vulkan/command_buffers/image_memory_barrier_command.cpp'
src += 'src/rendering/vulkan/command_buffers/set_viewport_command.cpp'

src += 'src/rendering/vulkan/context/device_context.cpp'
src += 'src/rendering/vulkan/context/device_context_base.cpp'
src += 'src/rendering/vulkan/context/device_context_factory.cpp'
src += 'src/rendering/vulkan/context/surface_context.cpp'

src += 'src/rendering/vulkan/debug/debug_utils.cpp'

src += 'src/rendering/vulkan/descriptors/descriptor_binding_state.cpp'
src += 'src/rendering/vulkan/descriptors/descriptor_pool.cpp'
src += 'src/rendering/vulkan/descriptors/descriptor_pool_description.cpp'
src += 'src/rendering/vulkan/descriptors/descriptor_set.cpp'
src += 'src/rendering/vulkan/descriptors/descriptor_set_layout.cpp'
src += 'src/rendering/vulkan/descriptors/descriptor_set_state.cpp'

src += 'src/rendering/vulkan/geometry/vertex_binding_state.cpp'
src += 'src/rendering/vulkan/geometry/vertex_buffer.cpp'
src += 'src/rendering/vulkan/geometry/vertex_buffer_state.cpp'
src += 'src/rendering/vulkan/geometry/vertex_buffer_suballocation_state.cpp'

src += 'src/rendering/vulkan/images/image.cpp'
src += 'src/rendering/vulkan/images/image_state.cpp'
src += 'src/rendering/vulkan/images/image_view.cpp'
src += 'src/rendering/vulkan/images/image_view_description.cpp'

src += 'src/rendering/vulkan/memory/free_list.cpp'
src += 'src/rendering/vulkan/memory/free_list_allocator.cpp'

src += 'src/rendering/vulkan/operations/copy_image_to_buffer_operation.cpp'
src += 'src/rendering/vulkan/operations/copy_to_vertex_buffer_operation.cpp'
src += 'src/rendering/vulkan/operations/update_descriptor_sets_operation.cpp'

src += 'src/rendering/vulkan/physical_devices/physical_device.cpp'
src += 'src/rendering/vulkan/physical_devices/queue_family_set.cpp'

src += 'src/rendering/vulkan/push_constants/push_constants_command.cpp'
src += 'src/rendering/vulkan/push_constants/push_constant_state.cpp'
src += 'src/rendering/vulkan/push_constants/push_special_constant_command.cpp'

src += 'src/rendering/vulkan/rendering/framebuffer.cpp'
src += 'src/rendering/vulkan/rendering/indirect_buffer.cpp'
src += 'src/rendering/vulkan/rendering/pipeline.cpp'
src += 'src/rendering/vulkan/rendering/pipeline_description.cpp'
src += 'src/rendering/vulkan/rendering/render_frame.cpp'
src += 'src/rendering/vulkan/rendering/render_pass.cpp'
src += 'src/rendering/vulkan/rendering/renderer.cpp'
src += 'src/rendering/vulkan/rendering/reserved_resource.cpp'
src += 'src/rendering/vulkan/rendering/scene.cpp'
src += 'src/rendering/vulkan/rendering/surface_renderer.cpp'

src += 'src/rendering/vulkan/resources/resource_ref.cpp'

src += 'src/rendering/vulkan/samplers/sampler.cpp'

src += 'src/rendering/vulkan/shaders/shader_module.cpp'
src += 'src/rendering/vulkan/shaders/shader_manager.cpp'
src += 'src/rendering/vulkan/shaders/shader_resource.cpp'

src += 'src/rendering/vulkan/state/condition_definition.cpp'
src += 'src/rendering/vulkan/state/node_insertion_context.cpp'
src += 'src/rendering/vulkan/state/condition_group.cpp'
src += 'src/rendering/vulkan/state/graphviz_dump.cpp'
src += 'src/rendering/vulkan/state/state_tracker.cpp'
src += 'src/rendering/vulkan/state/state_variable.cpp'
src += 'src/rendering/vulkan/state/state_variable_dag.cpp'
src += 'src/rendering/vulkan/state/state_variable_definition.cpp'
src += 'src/rendering/vulkan/state/state_variable_table.cpp'
src += 'src/rendering/vulkan/state/state_variable_iterator.cpp'

src += 'src/rendering/vulkan/spirv/disassemble.cpp'
src += 'src/rendering/vulkan/spirv/formatters.cpp'
src += 'src/rendering/vulkan/spirv/shader_metadata.cpp'
src += 'src/rendering/vulkan/spirv/type_info.cpp'
src += 'src/rendering/vulkan/spirv/variable_info.cpp'

src += 'src/rendering/vulkan/surfaces/surface.cpp'
src += 'src/rendering/vulkan/surfaces/swapchain.cpp'
src += 'src/rendering/vulkan/surfaces/vulkan_window.cpp'
src += 'src/rendering/vulkan/surfaces/window_factory.cpp'

src += 'src/rendering/vulkan/synchronization/fence.cpp'
src += 'src/rendering/vulkan/synchronization/semaphore.cpp'

src += 'src/rendering/vulkan/texture_atlases/texture_atlas.cpp'
src += 'src/rendering/vulkan/texture_atlases/texture_atlas_state.cpp'

src += 'src/rendering/vulkan/transfer/transfer_source_buffer.cpp'
src += 'src/rendering/vulkan/transfer/transfer_source_buffer_set.cpp'
src += 'src/rendering/vulkan/transfer/transfer_source_buffer_set_state.cpp'

src += 'src/rendering/vulkan/uniform_buffers/copy_to_uniform_buffer_operation.cpp'
src += 'src/rendering/vulkan/uniform_buffers/renderer_state_uniform_handler.cpp'
src += 'src/rendering/vulkan/uniform_buffers/uniform_buffer.cpp'
src += 'src/rendering/vulkan/uniform_buffers/uniform_buffer_state.cpp'
src += 'src/rendering/vulkan/uniform_buffers/uniform_data_source_manager.cpp'

src += 'src/rendering/vulkan/utils/spirv_introspect.cpp'

src += 'src/rendering/vulkan/x11/surface_factory.cpp'
src += 'src/rendering/vulkan/x11/window_factory.cpp'

src += 'src/signals/signal.cpp'

src += 'src/streams/streams.cpp'
src += 'src/streams/fd_source.cpp'
src += 'src/streams/fd_sink.cpp'
src += 'src/streams/string_sink.cpp'

src += 'src/time/hrtime.cpp'

src += 'src/tty/component.cpp'
src += 'src/tty/input_context.cpp'
src += 'src/tty/output_context.cpp'
src += 'src/tty/termpaint_to_keysym.cpp'
src += 'src/tty/termpaint_style_map.cpp'
src += 'src/tty/tty_restore.cpp'
src += 'src/tty/uv_termpaint_integration.cpp'

src += 'src/tty/components/divider.cpp'
src += 'src/tty/components/line_editor.cpp'
src += 'src/tty/components/line_table.cpp'
src += 'src/tty/components/linear_layout.cpp'
src += 'src/tty/components/static_string.cpp'
src += 'src/tty/components/scroll_view.cpp'

src += 'src/util/extern_templates.cpp'

src += 'src/ui/element.cpp'

src += 'src/ui/layout/linear_layout.cpp'

src += 'src/ui/elements/tabula_rasa.cpp'
src += 'src/ui/elements/text_view.cpp'

src += 'src/ui/formatters/layout_formatters.cpp'

src += 'src/ui/rendering/element_frame.cpp'
src += 'src/ui/rendering/renderer.cpp'

src += 'src/ui/text/line.cpp'
src += 'src/ui/text/line_table.cpp'
src += 'src/ui/text/paragraph_layout_engine.cpp'

src += 'src/ui/utils/test_rendering.cpp'
src += 'src/ui/utils/test_paragraph_layout.cpp'

src += 'src/windowing/window.cpp'

src += 'src/windowing/x11/context.cpp'
src += 'src/windowing/x11/exceptions.cpp'
src += 'src/windowing/x11/keyboard_context.cpp'
src += 'src/windowing/x11/keyboard_state.cpp'
src += 'src/windowing/x11/window.cpp'

#meson.current_build_dir(), meson.current_build_dir() / 'generated')
j_lib = library(
  'j',
  src,
  dependencies: dependencies,
  implicit_include_directories : false,
  cpp_pch: 'src/j_lib_pch.hpp')


j_lib_dep = declare_dependency(link_with: j_lib)
main_deps = [j_lib_dep]

j_exe = executable(
  'j',
  ['src/main.cpp'],
  dependencies: main_deps,
  implicit_include_directories : false,
  cpp_pch: 'src/j_lib_pch.hpp')

subdir('test')

run_target('clang-tidy',
  command: 'tools/run-clang-tidy.sh',
  depends: [j_lib, j_exe, j_test_exe])
