#!/bin/env python

import sys
import collections
from ucd_utils import *

if len(sys.argv) != 5:
  print(f'Usage: {sys.argv[0]} hpp|cpp LINEBREAK GENERAL_CATEGORY OUTPUT')
  sys.exit(1)

is_cpp = sys.argv[1] == 'cpp'
general_categories = parse_unicode_table(sys.argv[3])

def general_category(c):
  for cat in general_categories:
    if cat.begin <= c:
      return cat.value
  raise("Not found")

distinct_lbrs = set(['H2', 'H3'])
special_lbrs = set(['ZW', 'ZWJ', 'SY', 'SP', 'RI', 'BK', 'LF', 'HY', 'EM', 'CR',
                    'CB', 'JL', 'JT', 'JV', 'WJ', 'B2', 'IN', 'IS'])
lbr_map = {
  'CJ': 'NS',
  'AI': 'AL',
  'SG': 'AL',
  'NL': 'BK',
}

special_properties = dict([(v, []) for v in special_lbrs])
special_properties['CM'] = []
normal_properties = list()
table_chunks = [
  Chunk(0, 256, [])
]

def handle_table_chunk(rng):
  index = 0
  for c in table_chunks:
    if overlaps(c, rng):
      table_chunks[index].properties.append(clip(rng, c.begin, c.end))
      if contains(c, rng):
        return None
      return rng._replace(begin = min(rng.begin, c.end), end = max(rng.end, c.begin))
    index += 1
  return rng

for val in parse_unicode_table(sys.argv[2], end=0xF0000):
  if val.value == 'XX':
    continue
  if val.value == 'H2' or val.value == 'H3':
    continue
  elif val.value in lbr_map:
    val = val._replace(value = lbr_map[val.value])
  elif val.value == 'SA':
    val = val._replace(value = 'CM' if general_category(val.begin) in ['Mn', 'Mc'] else 'AL')

  distinct_lbrs.add(val.value)

  val = handle_table_chunk(val)
  if not val:
    continue

  if val.value == 'CM' and val.begin >= 0xE0000:
    special_properties['CM'].append(val)
  elif val.value in special_lbrs:
    special_properties[val.value].append(val)
  else:
    normal_properties.append(val)


normal_properties = default_ranges(normal_properties, [
  # Interval(0x000000, 0x0020A0, 'ID'), # XX
  Interval(0x0020A0, 0x0020D0, 'PR'),
  # Interval(0x0020D0, 0x003400, 'ID'), # XX
  Interval(0x003400, 0x004DC0, 'ID'),
  # Interval(0x004DC0, 0x004E00, 'ID'), # XX
  Interval(0x004E00, 0x00A000, 'ID'),
  # Interval(0x00A000, 0x00F900, 'ID'), # XX
  Interval(0x00F900, 0x00FB00, 'ID'),
  # Interval(0x00FB00, 0x01F000, 'ID'), # XX
  Interval(0x01F000, 0x01FB00, 'ID'),
  # Interval(0x01FB00, 0x01FC00, 'ID'), # XX
  Interval(0x01FC00, 0x01FFFE, 'ID'),
  Interval(0x020000, 0x02FFFE, 'ID'),
  Interval(0x030000, 0x03FFFE, 'ID'),
])

special_cases = {
  0x01400: 'BA',
  0x01FFD: 'BB',
  0x0A015: 'NS',
  0x0FDFC: 'PO',
  0x102E0: 'CM',
  0x10857: 'BA',
  0x1091F: 'BA',
}

normal_properties = assign_ranges(normal_properties, [
  Interval(0x000000, 0x000100, 'ID'),
  Interval(0x001400, 0x001401, 'AL'),
  Interval(0x001FFD, 0x001FFE, 'AL'),
  Interval(0x00A015, 0x00A016, 'ID'),
  Interval(0x00FDFC, 0x00FDFD, 'AL'),
  Interval(0x0102E0, 0x0102E1, 'AL'),
  Interval(0x010857, 0x010858, 'AL'),
  Interval(0x01091F, 0x010920, 'AL'),
])

chunk_size_shift = 8
chunk_size = 1 << chunk_size_shift
chunk_index_shift = chunk_size_shift - 4

chunks = [extend_ranges(c) for c in chunk_ranges(normal_properties, chunk_size)]
piece_references, pieces = merge_duplicate_chunks(chunks)
piece_references, pieces = sort_pieces_by_usage(piece_references, pieces)
piece_references, pieces = match_pieces(piece_references, pieces)

# def print_histogram(h, label):
#   h = [(num, key) for key, num in h.items()]
#   h.sort()
#   print(f'  {label}:')
#   for num, key in h:
#     if num != 0:
#       print(f'    {key}: {num:3}')

lbr_histogram = dict([(p, 0) for p in distinct_lbrs])
rng_histogram = dict([(p, 0) for p in distinct_lbrs])
num_dups = 0
for c in chunks:
  if not c.properties:
    continue
  for p in c.properties:
    lbr_histogram[p.value] += p.end - p.begin
    rng_histogram[p.value] += 1

# print(f'{normal_properties[-1].end:06X}')

# print(f'Size: {len(pieces) * chunk_size // 2 + len(piece_references)}')

outfile = open(sys.argv[4], "w")
if is_cpp:
  outfile.write("// GENERATED FILE - Do not edit.\n\n")
  outfile.write("#include <strings/unicode/line_break_property.hpp>\n\n")
  outfile.write("namespace j::strings::inline unicode {\n")
else:
  outfile.write("#pragma once\n\n")
  outfile.write("// GENERATED HEADER - Do not edit.\n\n")
  outfile.write("#include <hzd/types.hpp>\n\n")
  outfile.write("namespace j::strings::inline unicode {\n")

distinct_lbrs = [p for p in distinct_lbrs]
distinct_lbrs.sort(
  reverse = True,
  key = lambda prop: 100000 if prop == 'AL' else (lbr_histogram[prop] if prop in lbr_histogram else 0))


# Write lbr_t enum
if not is_cpp:
  outfile.write("  enum class lbr_t : u8_t {\n")
  outfile.write("    // Normal ranges present in maps:\n   ")
  has_printed_comment = False
  for prop in distinct_lbrs:
    if not has_printed_comment and prop in special_lbrs:
      outfile.write("\n    // Special ranges handled manually in functions:\n   ")
      has_printed_comment = True
    outfile.write(" " + prop.lower() + ",")
  outfile.write("\n  };\n\n")



# Write full tables
if not is_cpp:
  for c in table_chunks:
    outfile.write(f'  inline constexpr lbr_t lbr_table_{c.begin:04x}_{c.end:04x}[] = ')
    outfile.write("{")
    props = c.properties.copy()
    props.reverse()
    cur_cat = props.pop()
    for i in range(c.begin, c.end):
      if i == cur_cat.end:
        cur_cat = props.pop()
      if i % 8 == 0:
        outfile.write("\n")
        outfile.write(f'    // U+{i:06X} - U+{i + 7:06X}')
        outfile.write("\n   ")
      outfile.write(" lbr_t::" + cur_cat.value.lower() + ",")
    outfile.write("\n  };\n\n");

# Write actual table
piece_references, last_piece_value = trim_piece_references(piece_references)
if is_cpp:
  write_nibble_table(outfile, 'lbr_table', pieces, get_value = lambda v : distinct_lbrs.index(v), is_inline = False)
  write_index_table(outfile, 'lbr_index_table', piece_references, is_inline = False)
  outfile.write("}\n")
else:
  outfile.write("  extern constinit const u64_t lbr_table[];\n")
  outfile.write("  extern constinit const u8_t lbr_index_table[];\n")



  outfile.write("  inline lbr_t lbr_category(u32_t c) noexcept {\n")

  # Get from full tables
  for r in table_chunks:
    if r.begin != 0:
      outfile.write(f'    if (c >= 0x{r.begin:06X} && c < 0x{r.end:06X}) ')
    else:
      outfile.write(f'    if (c < 0x{r.end:06X}) ')
    outfile.write("{\n")
    outfile.write(f'      return lbr_table_{r.begin:04x}_{r.end:04x}[c];')
    outfile.write("\n    }\n\n")

  # Write hard-coded H2 / H3 handling
  outfile.write("""\
      if (c >= 0xAC00U) {
        if (c < 0xD7B0U) {
          // Precomposed Hangul Jamo
          return ((c - 0xAC00U) % 0x1C == 0) ? lbr_t::h2 : lbr_t::h3;
        } else if (c >= 0xF0000U) {
        // Private use
        return lbr_t::al;
        }
      }

  """)
  outfile.write("  switch (c) {\n")
  for c, t in special_cases.items():
    outfile.write(f'  case 0x{c:06X}U: return lbr_t::{t.lower()};' + "\n")
  outfile.write("  default: break;\n")
  outfile.write("  }\n\n")

  # Write special-cased properties
  for prop, rngs in special_properties.items():
    subranges = []
    additional_points = []
    end = 256
    for start, end2, _ in rngs:
      if start + 1 == end2:
        additional_points.append(f'c == 0x{start:06X}U')
        continue
      subranges.append(Interval(end, start, None))
      end = end2
    if not subranges and not additional_points:
      continue
    subranges.append(Interval(end, 0x110000, None))
    subranges.sort(reverse = True,
                  key = lambda r: (r.end - r.begin) if (r.end != 0x110000 and r.begin != 256) else 2 * 2 * (r.end - r.begin))
    conditions = []
    for r in subranges:
      if r.end == r.begin or (r.begin == 256 and r.end == 0x110000):
        continue
      if r.begin + 1 == r.end:
        conditions.append(f'c != 0x{r.begin:06X}U')
      else:
        if r.begin == 256:
          conditions.append(f'c >= 0x{r.end:06X}U')
        elif r.end == 0x110000:
          conditions.append(f'c < 0x{r.begin:06X}U')
        else:
          conditions.append(f'(c < 0x{r.begin:06X}U || c >= 0x{r.end:06X}U)')

    if additional_points and conditions:
      outfile.write(f'    if (({" && ".join(conditions)}) || {" || ".join(additional_points)}) ')
    elif conditions:
      outfile.write(f'    if ({" && ".join(conditions)}) ')
    else:
      outfile.write(f'    if ({" || ".join(additional_points)}) ')
    outfile.write("{\n")
    outfile.write("      return lbr_t::" + prop.lower() + ";\n")
    outfile.write("    }\n")

  # Write array lookup
  outfile.write(f'    const u32_t idx = (c >= 0x{len(piece_references) << (chunk_size_shift):X}U)' + "\n")
  outfile.write(f'      ? {last_piece_value << (chunk_index_shift - 1)}U' + "\n")
  outfile.write(f'      : (lbr_index_table[c >> {chunk_size_shift}U] << {chunk_index_shift - 1}U);' + "\n");
  outfile.write(f'    const u32_t offset = (c >> 4U) & {(1 << (chunk_index_shift)) - 1}U;' + "\n");
  outfile.write("""\
      const u64_t val = lbr_table[idx + offset];
      return static_cast<lbr_t>((val >> ((c & 15U) << 2U)) & 15U);
    }
  }
  """)
