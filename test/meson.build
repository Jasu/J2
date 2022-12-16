# Generated Unicode tests
# =======================

ucd_grapheme_tests_tgt = custom_target(
  'GraphemeBreakTest.txt',
  input: '../ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'auxiliary/GraphemeBreakTest.txt', '-d', '@OUTDIR@'],
  output: 'GraphemeBreakTest.txt')
ucd_line_break_test_tgt = custom_target(
  'LineBreakTest.txt',
  input: '../ext/UCD.zip',
  command: ['unzip', '-o', '-j', '@INPUT@', 'auxiliary/LineBreakTest.txt', '-d', '@OUTDIR@'],
  output: 'LineBreakTest.txt')
ucd_line_break_cpp_tgt = custom_target(
  'line_break.cpp',
  input: ucd_line_break_test_tgt,
  depend_files: files('../tools/unicode_line_break_test.py'),
  command: [
    'python',
    join_paths(meson.current_source_dir(), '../tools/unicode_line_break_test.py'),
    '@INPUT@',
    '@OUTPUT@',
  ],
  output: 'line_break.cpp')
ucd_east_asian_width_cpp_tgt = custom_target(
  'east_asian_width.cpp',
  input: ucd_east_asian_width_txt_tgt,
  depend_files: files('../tools/unicode_east_asian_width_test.py', '../tools/ucd_utils.py'),
  command: [
    'python',
    join_paths(meson.current_source_dir(), '../tools/unicode_east_asian_width_test.py'),
    '@INPUT@',
    '@OUTPUT@',
  ],
  output: 'east_asian_width.cpp')
ucd_grapheme_break_cpp_tgt = custom_target(
  'grapheme_break.cpp',
  input: ucd_grapheme_tests_tgt,
  depend_files: files('../tools/unicode_grapheme_break_test.py'),
  command: [
    'python',
    join_paths(meson.current_source_dir(), '../tools/unicode_grapheme_break_test.py'),
    '@INPUT@',
    '@OUTPUT@',
  ],
  output: 'grapheme_break.cpp')
ucd_line_break_prop_test_cpp_tgt = custom_target(
  'lbr_prop_generated.cpp',
  input: ucd_line_break_tgt,
  depends: [ucd_derived_general_category_tgt],
  depend_files: files('../tools/unicode_line_break_prop_test.py'),
  command: [
    'python',
    join_paths(meson.current_source_dir(), '../tools/unicode_line_break_prop_test.py'),
    '@INPUT@',
    ucd_derived_general_category_tgt,
    '@OUTPUT@',
  ],
  output: 'lbr_prop_generated.cpp')

call_ret_asm_o = nasm_gen.process('lisp/assembly/call_ret.asm')
fp_convert_asm_o = nasm_gen.process('lisp/assembly/fp_convert.asm')
fp_ops_asm_o = nasm_gen.process('lisp/assembly/fp_ops.asm')
setcc_asm_o = nasm_gen.process('lisp/assembly/setcc.asm')
pop_asm_o = nasm_gen.process('lisp/assembly/pop.asm')
push_asm_o = nasm_gen.process('lisp/assembly/push.asm')
jmp_asm_o = nasm_gen.process('lisp/assembly/jmp.asm')
jcc_asm_o = nasm_gen.process('lisp/assembly/jcc.asm')

arith_bin_asm_o = nasm_gen.process('lisp/assembly/arith_bin.asm')
not_neg_asm_o = nasm_gen.process('lisp/assembly/not_neg.asm')

bt_asm_o = nasm_gen.process('lisp/assembly/bt.asm')
shifts_asm_o = nasm_gen.process('lisp/assembly/shifts.asm')
shiftd_asm_o = nasm_gen.process('lisp/assembly/shiftd.asm')
lea_asm_o = nasm_gen.process('lisp/assembly/lea.asm')

mov_index_asm_o = nasm_gen.process('lisp/assembly/mov_index.asm')
mov_imm_asm_o = nasm_gen.process('lisp/assembly/mov_imm.asm')
mov_rip_asm_o = nasm_gen.process('lisp/assembly/mov_rip.asm')
mov_asm_o = nasm_gen.process('lisp/assembly/mov.asm')

movd_movq_asm_o = nasm_gen.process('lisp/assembly/movd_movq.asm')
movsxzx_asm_o = nasm_gen.process('lisp/assembly/movsxzx.asm')

test_src = []

test_src += fp_ops_asm_o
test_src += fp_convert_asm_o
test_src += call_ret_asm_o
test_src += pop_asm_o
test_src += push_asm_o
test_src += setcc_asm_o
test_src += jcc_asm_o
test_src += jmp_asm_o

test_src += not_neg_asm_o
test_src += arith_bin_asm_o
test_src += bt_asm_o
test_src += shiftd_asm_o
test_src += shifts_asm_o

test_src += movsxzx_asm_o
test_src += lea_asm_o

test_src += mov_asm_o
test_src += mov_rip_asm_o
test_src += mov_imm_asm_o
test_src += mov_index_asm_o

test_src += movd_movq_asm_o

test_src += ucd_grapheme_properties_hpp_tgt
test_src += ucd_emoji_data_hpp_tgt
test_src += ucd_line_break_property_hpp_tgt
test_src += ucd_east_asian_width_hpp_tgt

# test_src += ucd_grapheme_break_cpp_tgt
test_src += ucd_line_break_prop_test_cpp_tgt
# test_src += ucd_line_break_cpp_tgt
test_src += ucd_east_asian_width_cpp_tgt
test_src += 'attributes/keyed.cpp'

test_src += 'bits/bitops.cpp'
test_src += 'bits/bitmask.cpp'
test_src += 'bits/bitset.cpp'
test_src += 'bits/fixed_bitset.cpp'

test_src += 'containers/avl_tree.cpp'
test_src += 'containers/deque.cpp'
test_src += 'containers/hash_map.cpp'
test_src += 'containers/rope_tree.cpp'

test_src += 'color/serialization.cpp'
test_src += 'color/transforms.cpp'

test_src += 'detail/exceptions.cpp'
test_src += 'detail/temp_file.cpp'
test_src += 'detail/nontrivial.cpp'
test_src += 'detail/stringify.cpp'

# test_src += 'fonts/specification_serialization.cpp'

test_src += 'hzd/crc32.cpp'

# test_src += 'lisp/assembly/emit_arith_bin.cpp'
# test_src += 'lisp/assembly/emit_bt.cpp'
# test_src += 'lisp/assembly/emit_fp_ops.cpp'
# test_src += 'lisp/assembly/emit_fp_convert.cpp'
# test_src += 'lisp/assembly/emit_jcc.cpp'
# test_src += 'lisp/assembly/emit_jmp.cpp'
# test_src += 'lisp/assembly/emit_lea.cpp'
# test_src += 'lisp/assembly/emit_mov.cpp'
# test_src += 'lisp/assembly/emit_movd_movq.cpp'
# test_src += 'lisp/assembly/emit_movsxzx.cpp'
# test_src += 'lisp/assembly/emit_not_neg.cpp'
# test_src += 'lisp/assembly/emit_pop.cpp'
# test_src += 'lisp/assembly/emit_push.cpp'
# test_src += 'lisp/assembly/emit_ret.cpp'
# test_src += 'lisp/assembly/emit_setcc.cpp'
# test_src += 'lisp/assembly/emit_shifts.cpp'
# test_src += 'lisp/assembly/emit_shiftd.cpp'
# test_src += 'lisp/reader.cpp'

test_src += 'lisp/compilation/arithmetic.cpp'
test_src += 'lisp/compilation/closures.cpp'
test_src += 'lisp/compilation/compilation.cpp'
test_src += 'lisp/compilation/utils.cpp'
test_src += 'lisp/compilation/vecs.cpp'

test_src += 'lisp/air/trs.cpp'

test_src += 'math/fixed.cpp'
test_src += 'math/interval.cpp'

test_src += 'mem/ring_buffer.cpp'

test_src += 'properties/conversions.cpp'
test_src += 'properties/enum_access.cpp'
test_src += 'properties/bitmask_set.cpp'
test_src += 'properties/list_access.cpp'
test_src += 'properties/map_access.cpp'
test_src += 'properties/set_access.cpp'
test_src += 'properties/metadata.cpp'
test_src += 'properties/string_access.cpp'
test_src += 'properties/variant.cpp'
test_src += 'properties/variant_access.cpp'
test_src += 'properties/string_access.cpp'
test_src += 'properties/int_access.cpp'
test_src += 'properties/bool_access.cpp'
test_src += 'properties/float_access.cpp'
test_src += 'properties/object_access.cpp'
test_src += 'properties/path_to_string.cpp'
test_src += 'properties/visit.cpp'
test_src += 'properties/visit_definition.cpp'
test_src += 'properties/visit_recursive.cpp'
test_src += 'properties/visit_path_to_string.cpp'

test_src += 'rendering/get_vertex_attribute_format.cpp'

test_src += 'serialization/yaml.cpp'

test_src += 'strings/equality.cpp'
test_src += 'strings/format_integer.cpp'
test_src += 'strings/format_string_iterator.cpp'
test_src += 'strings/landmark.cpp'
test_src += 'strings/markers.cpp'
test_src += 'strings/placeholder.cpp'
test_src += 'strings/styled_string.cpp'
test_src += 'strings/string.cpp'
test_src += 'strings/utf8.cpp'
# test_src += 'strings/utf8_rope.cpp'
test_src += 'strings/utf8_rope2.cpp'
# test_src += 'strings/utf8_rope_egc.cpp'
# test_src += 'strings/utf8_rope_lbr.cpp'
# test_src += 'strings/utf8_rope_lbr_edit.cpp'

test_src += 'ui/linear_layout.cpp'

test_src += 'util/hex.cpp'

test_cpp_args = [
  '-DDOCTEST_CONFIG_SUPER_FAST_ASSERTS',
  # Broken, https://github.com/doctest/doctest/issues/674
  # '-DDOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING',
]

j_test_main_lib = static_library(
  'j-test-main',
  ['../ext/doctest/doctest/parts/doctest.cpp'],
  cpp_args: test_cpp_args + ['-DDOCTEST_CONFIG_IMPLEMENT_WITH_MAIN']
)

j_test_main = declare_dependency(
  link_with: j_test_main_lib,
  include_directories: include_directories('../ext/doctest/doctest/parts')
)

j_test_exe = executable(
  'j-test',
  test_src,
  cpp_args: test_cpp_args,
  cpp_pch: '../pch/j_test_pch.hpp',
  dependencies: [j_lib_dep, j_test_main])
