from collections import namedtuple
from bisect import bisect_left, bisect_right
from array import array
from math import log2
import itertools
import re

CodepointMax = 0x10FFFF
CodepointEnd = 0x110000

HistogramTypes={
  'by_codepoint': (1, 'codepoint', '#Code'),
  'by_range':     (0, 'range',     ' #Rng'),
}

Alias = namedtuple('Alias', ('canonical', 'aliases'))
UnicodeBlock = namedtuple('UnicodeBlock', ('begin', 'end', 'name', 'search_name'))

Interval = namedtuple('Interval', ('begin', 'end', 'value'))
Chunk = namedtuple('Chunk', ('begin', 'end', 'properties'))
LargeRegion = namedtuple('LargeRegion', ('min_begin', 'max_begin', 'min_end', 'max_end', 'value', 'special_cases'))
Region = namedtuple('Region', ('begin', 'end', 'value', 'special_cases'))
def large_region_min_size(r):
  return r.min_end - r.max_begin
def large_region_max_size(r):
  return r.max_end - r.min_begin

Piece = namedtuple('Piece', ('size', 'properties', 'index'))
PieceReference = namedtuple('PieceReference', ('begin', 'end', 'piece'))

def ranges_window(ranges, begin, end):
  return (r if r.begin >= begin and r.end <= end else r._replace(begin=max(r.begin, begin), end=min(r.end, end)) for r in ranges if r.end > begin and r.begin < end)

def chunks_intervals(chunks):
  return itertools.chain.from_iterable(c.properties for c in chunks)

def chunk_contains_any(chunk, values):
  for i in chunk.properties:
    if i.value in values:
      return True
  return False

def clip(interval, begin, end):
  return Interval(max(begin, interval.begin), min(end, interval.end), interval.value)

def overlaps(a, b):
  return a.begin < b.end and a.end > b.begin

def contains(a, b):
  return a.begin <= b.begin and a.end >= b.end

def parse_aliases(path, prop_alias, first_only=False):
  f = open(path, 'r')
  result = dict()
  for line in f:
    line = line.split('#', 1)[0].strip()
    if not line:
      continue
    prop, name, *aliases = line.split(';')
    if prop.rstrip() != prop_alias:
      continue
    name = name.strip()
    if first_only:
      result[name] = aliases[0].strip()
    else:
      alias = Alias(name, [s.strip() for s in aliases])
      result[name] = alias
      for a in alias.aliases:
        result[a] = alias
  return result

def to_search_regex(s):
  return re.sub(r'\\s| +|(?:(?<=[a-z])-(?=[a-z]))', '_?', s, flags=re.IGNORECASE)


def to_search_regexes(s):
  return to_search_regex(s[0]) if len(s) == 1 else '(?:' + (')|(?:'.join(to_search_regex(ss) for ss in s)) + ')'

def to_search_string(s):
  return re.sub(r'\s|(?:(?<=[a-z])-(?=[a-z]))', '_', s.casefold(), flags=re.IGNORECASE)

def parse_unicode_blocks(path):
  return [UnicodeBlock(i.begin, i.end, i.value, to_search_string(i.value)) for i in parse_unicode_table(path)]

def unicode_block_index(blocks, codepoint, prev=0):
  return bisect_right(blocks, (codepoint, codepoint, ''), lo=(prev or 0))

def parse_unicode_table(path, value_index=1, ignored = set(), begin = 0, end = CodepointEnd, is_unicodedata = False):
  f = open(path, 'r')
  result = []
  value_index = value_index - 1
  prev = None
  for line in f:
    if not is_unicodedata:
      line = line.split('#', 1)[0].strip()
      if not line:
        continue
    rng, *data = line.split(';')
    val = data[value_index].strip()
    if val in ignored:
      continue
    if is_unicodedata:
      name = data[0]
      if name[0] == '<':
        if name.endswith(', Last>'):
          r_begin = prev
          r_end = int(rng, 16) + 1
          prev = None
        elif name.endswith(', First>'):
          prev = int(rng, 16)
          r_begin = None
          r_end = None
          continue
        else:
          r_begin = int(rng, 16)
          r_end = r_begin + 1
      else:
        r_begin = int(rng, 16)
        r_end = r_begin + 1
    else:
      rng = rng.split('..')
      r_begin = int(rng[0], 16)
      r_end = (r_begin if len(rng) == 1 else int(rng[1], 16)) + 1
    if r_end > begin and r_begin < end:
      result.append(Interval(max(r_begin, begin), min(r_end, end), val))
  f.close()
  result.sort()
  return result

bar_template = '################################################################################                                                                                '

def print_indent(indent):
  if indent:
    print('  ' * indent, end='')

def print_chunk_header(c, indent=0):
  print_indent(indent)
  print(f'Chunk {c.begin:06X}-{c.end:06X}: ({len(c.properties)} intervals)')

def print_interval_raw(begin, end, value, indent=0, **kwargs):
  print_indent(indent)
  print(f'{format_unicode_range(begin, end, const_size=True)}: {format_key(value, **kwargs)}')

def print_interval(i, **kwargs):
  print_interval_raw(i.begin, i.end, i.value, **kwargs)

def print_intervals(intervals, indent=0, interval_filter=None, filter_after=0, collapse=False, unicode_blocks=None, **kwargs):
  prev_begin, prev_end, prev_value = None, None, None
  filter_was_ok = 0
  cur_unicode_block = None
  for i in intervals:
    filter_ok = not interval_filter or interval_filter(i)
    if filter_ok:
      filter_was_ok = filter_after
    else:
      filter_ok = filter_ok or filter_was_ok > 0
      filter_was_ok -= 1
    if collapse:
      if prev_value is not None and (not filter_ok or prev_value != i.value or prev_end != i.begin):
        if unicode_blocks:
          new_block = unicode_block_index(unicode_blocks, prev_begin, prev=cur_unicode_block)
          if new_block != cur_unicode_block:
            print_indent(indent - 1)
            block = unicode_blocks[new_block]
            print(f'Block {block.name} at {format_unicode_range(block.begin, block.end, const_size=True)}')
            cur_unicode_block = new_block
        print_interval_raw(prev_begin, prev_end, prev_value, indent=indent, **kwargs)
        prev_value = None
      if prev_value is not None:
        prev_end = i.end
      elif filter_ok:
        prev_begin, prev_end, prev_value = i
    elif filter_ok:
      if unicode_blocks:
        new_block = unicode_block_index(unicode_blocks, prev_begin, prev=cur_unicode_block)
        if new_block != cur_unicode_block:
          print_indent(indent - 1)
          block = unicode_blocks[new_block]
          print(f'Block {block.name} at {format_unicode_range(block.begin, block.end, const_size=True)}')
          cur_unicode_block = new_block
      print_interval(i, indent=indent, **kwargs)
  if prev_value is not None:
    if unicode_blocks:
      new_block = unicode_block_index(unicode_blocks, prev_begin, prev=cur_unicode_block)
      if new_block != cur_unicode_block:
        print_indent(indent - 1)
        block = unicode_blocks[new_block]
        print(f'Block {block.name} at {format_unicode_range(block.begin, block.end, const_size=True)}')
        cur_unicode_block = new_block
    print_interval_raw(prev_begin, prev_end, prev_value, indent=indent, **kwargs)

def print_chunk(c, indent=0, **kwargs):
  print_chunk_header(c, indent=indent)
  print_intervals(c.properties, indent=indent + 1, **kwargs)


def format_key(key, key_map=None, with_key=False):
  if not key_map or not key in key_map:
    return key
  return f'{key_map[key]} ({key})' if with_key and key != key_map[key] else key_map[key]

def print_histogram(h, key_index, label=None, width=50, bar_log=False, **kwargs):
  if label:
    print(label)
  h = [(entry[key_index], format_key(key, **kwargs), entry) for key, entry in h.items()]
  key_len = 0
  for t in h:
    key_len = max(key_len, len(t[1]))
  h.sort()
  max_sz = log2(h[-1][0]) if bar_log else h[-1][0]
  mul = width / max_sz
  for num, key, entry in h:
    if num != 0:
      begin = 80 - int(mul * (log2(num) if bar_log else num))
      bar = bar_template[begin:begin+width]
      range_above_256 = '' if entry[4] == entry[2] or entry[4] > entry[5] else f' (256+: {entry[4]:06X}..{entry[5]:06X})';
      key = key.rjust(key_len)
      print(f'  {key}: {num:6} |{bar}| Range: {entry[2]:06X}..{entry[3]:06X}{range_above_256}')

def print_mini_histograms(h, types=('by_codepoint', 'by_range'), max_width=1000, indent=0):
  max_width -= indent * 2 + 5 + 4
  sums = [sum(entry[HistogramTypes[t][0]] for entry in h.values()) for t in types]
  h = sorted([([entry[HistogramTypes[t][0]] for t in types], ' ' + key) for key, entry in h.items()], reverse=True)
  h = [(key, *(f' {num}' for num in nums)) for nums, key in h]
  h = [(max(len(k) for k in t), *(k for k in t)) for t in h]

  bound = 0
  for t in h:
    bound += 1
    max_width -= t[0]
    if max_width <= 0:
      break

  print(' ' * (indent * 2 + 5) + ''.join(t[1].ljust(t[0]) for t in h[0:bound]) + ' Sum')
  i = 2
  for t in types:
    print(' ' * (indent * 2) + HistogramTypes[t][2] + ''.join(t[i].ljust(t[0]) for t in h[0:bound]) + f' {sums[i-2]}')
    i += 1

  # for nums, k in h:
  #   k = k.ljust(max(len(n) for n in nums))
  #   max_width -= len(k)
  #   if max_width < 0:
  #     break
  #   print(k, end='')
  #   num_taken += 1
  # print('')
  # for n, k in h[0:num_taken]:
  #   print(n.ljust(len(k)), end='')
  # print('')


def print_histograms(h, types=('by_codepoint', 'by_range'), label=None, **kwargs):
  for t in types:
    idx, name, _ = HistogramTypes[t]
    print_histogram(h, idx, **kwargs, label=(f'{label} by {name}:' if label else f'By {name}:'))

def accumulate_histogram_interval(h, i, begin=0, end=CodepointEnd, no_full_chunk=False):
  if i.end <= begin or i.begin >= end:
    return
  count_as = 0 if no_full_chunk and i.begin + 256 == i.end else 1
  rngsz = min(end, i.end) - max(begin, i.begin)
  high_begin = CodepointEnd if i.end < 256 else max(i.begin, 256)
  high_end = 0 if i.end < 256 else i.end - 1
  if i.value not in h:
    h[i.value] = array('l', [count_as, rngsz, i.begin, i.end - 1, high_begin, high_end])
  else:
    entry = h[i.value]
    entry[0] += count_as
    entry[1] += rngsz
    entry[2] = min(entry[2], i.begin)
    entry[3] = max(entry[3], i.end - 1)
    entry[4] = min(entry[4], high_begin)
    entry[5] = max(entry[5], high_end)

def accumulate_histogram(h, ranges, **kwargs):
  for i in ranges:
    accumulate_histogram_interval(h, i, **kwargs)

def print_range_histogram(ranges, no_full_chunk=False, **kwargs):
  h = dict()
  accumulate_histogram(h, ranges, no_full_chunk=no_full_chunk)
  print_histograms(h, **kwargs)

def print_chunk_histogram(chunks, no_full_chunk=False, **kwargs):
  h = dict()
  for c in chunks:
    accumulate_histogram(h, c.properties, no_full_chunk=no_full_chunk)
  print_histograms(h, **kwargs)

def delete_and_remap_codepoints(ranges, begin, end):
  delta = begin - end
  return [r if r.end <= begin else
          r._replace(
            begin=min(r.begin, begin) if r.begin <= end else r.begin + delta,
            end=min(r.end, begin) if r.end <= end else r.end + delta
          ) for r in ranges if r.begin < begin or r.end > end]

def split_ranges(ranges, at):
  return (
    [(r if r.end <= at else Interval(r.begin, at, r.value)) for r in ranges if r.begin < at],
    [(r if r.begin >= at else Interval(at, r.end, r.value)) for r in ranges if r.end > at])

def validate_ranges(ranges):
  prev = None
  for r in ranges:
    if r.end <= r.begin:
      raise ValueError(f'Range {r.begin:06X}-{r.end:06X} was empty')
    if prev:
      if r.begin < prev.end:
        raise ValueError(f'Ranges {prev.begin:06X}-{prev.end:06X} and {r.begin:06X}-{r.end:06X} overlap')
    prev = r


def extract_ranges(ranges, begin, end):
  assert begin < end
  return (
    [(r if r.end <= end and r.begin >= begin else Interval(max(r.begin, begin), min(r.end, end), r.value)) for r in ranges if r.begin < end and r.end > begin],
    [(r if r.end <= begin or r.begin >= end else Interval(max(r.begin, end) if r.begin >= begin else r.begin, min(r.end, begin) if r.end <= end else r.end, r.value)) for r in ranges if r.end > end or r.begin < begin])


def erase_ranges(ranges, begin, end):
  assert begin < end
  result = []
  for i in ranges:
    if i.end <= begin or i.begin >= end:
      result.append(i)
      continue
    if i.begin < begin:
      result.append(i._replace(end=begin))
    if i.end > end:
      result.append(i._replace(begin=end))
  return result

def find_large_ranges(ranges, begin=0, end=0x110000, special_case_ratio=64, min_range_size=128, start_threshold=32):
  before_interval = None
  begin_interval = ranges[0]
  prev_interval = begin_interval
  result = []
  for r in ranges[1:]:
    if begin_interval.value != r.value:
      min_begin = before_interval.end if before_interval else begin
      if r.begin - min_begin >= min_range_size:
        result.append(LargeRegion(min_begin, begin_interval.begin, prev_interval.end, r.begin, begin_interval.value, []))
      before_interval = prev_interval
      begin_interval = r
    prev_interval = r

  min_begin = before_interval.end if before_interval else begin
  if end - min_begin >= min_range_size:
    result.append(LargeRegion(min_begin, begin_interval.begin, prev_interval.end, end, begin_interval.value, []))
  return result


def filter_ranges(ranges, ignored_vals=set()):
  return [r for r in ranges if r.value not in ignored_vals]

def chunk_ranges(ranges, sz, begin=0):
  result = list()
  end = begin + sz
  subranges = list()
  for r in ranges:
    while r.begin >= end:
      result.append(Chunk(begin, end, subranges))
      subranges = list()
      begin = end
      end += sz
    if r.end <= end:
      subranges.append(r)
      continue
    rng_begin = r.begin
    while r.end > end:
      subranges.append(Interval(rng_begin, end, r.value))
      result.append(Chunk(begin, end, subranges))
      subranges = list()
      begin = end
      rng_begin = end
      end += sz
    if r.end > begin:
      subranges.append(Interval(rng_begin, r.end, r.value))
  if subranges:
    result.append(Chunk(begin, end, subranges))
  return result



  #   if r.begin < end:
  #     if r.end <= end:
  #       subranges.append(r)
  #     else:
  #       rng_begin = r.begin
  #       while r.end >
  #       subranges.append(Interval(r.begin, end, r.value))
  #       begin += sz
  #       end += sz


  # for begin in range(0, ranges[-1].end, sz):
  #   end = begin + sz
  #   subranges = [clip(r, begin, end) for r in ranges if r.end > begin and r.begin < end]
  #   result.append(Chunk(begin, end, subranges))
  # return result

def extend_ranges(chunk):
  if not chunk.properties:
    return chunk
  prev_begin = chunk.begin
  prev = chunk.properties[0].value
  result = list()
  for begin, end, value in chunk.properties:
    if value == prev:
      continue
    result.append(Interval(prev_begin, begin, prev))
    prev = value
    prev_begin = begin
  result.append(Interval(prev_begin, chunk.end, prev))
  return chunk._replace(properties = result)

def append_to_range(range, interval):
  if interval.end <= interval.begin:
    pass
  elif range and range[-1].value == interval.value and range[-1].end == interval.begin:
    range[-1] = range[-1]._replace(end = interval.end)
  else:
    assert(not range or range[-1].end <= interval.begin)
    range.append(interval)

# def default_ranges(ranges, defaults, end = 0):
#   defaults = defaults.copy()
#   defaults.reverse()
#   cur_default = defaults.pop()

#   cur = 0
#   result = list()
#   for r in ranges:
#     while r.begin > cur and (cur < cur_default.end or defaults):
#       while cur >= cur_default.end and defaults:
#         cur_default = defaults.pop()
#       if cur < cur_default.end:
#         next = min(r.begin, cur_default.end)
#         if cur < next:
#           append_to_range(result, Interval(cur, next, cur_default.value))
#         cur = next
#     append_to_range(result, r)
#     cur = r.end
#   while cur < end and (cur < cur_default.end or defaults):
#     if cur_default.end == cur:
#       cur_default = defaults.pop()
#     next = min(cur_default.end, end)
#     if cur < next:
#       append_to_range(result, cur_default)
#       cur = next
#   return result
def default_ranges(ranges, defaults):
  return assign_ranges(defaults, ranges)

def assign_ranges(ranges, assignments):
  ranges = ranges.copy()
  ranges.reverse()
  a = assignments.copy()
  a.reverse()
  result = []
  nx = a.pop()
  while nx or ranges:
    cur = result[-1].end if result else 0
    if nx and (not ranges or nx.begin <= max(cur, ranges[-1].begin)):
      add = nx
      while ranges and ranges[-1].end <= add.begin:
        ranges.pop()
      nx = a.pop() if a else None
    else:
      beg = max(ranges[-1].begin, cur)
      end = min(ranges[-1].end, nx.begin) if nx else ranges[-1].end
      add = ranges[-1]._replace(begin=beg, end=end)

    append_to_range(result, add)
    while ranges and ranges[-1].end <= result[-1].end:
      ranges.pop()

  return result

def merge_duplicate_chunks(chunks):
  piece_map = dict()
  pieces = list()
  piece_references = list()
  for c in chunks:
    k = chunk_key(c)
    if k in piece_map:
      result_piece = piece_map[k]
    else:
      result_piece = Piece(
        size = c.end - c.begin,
        properties = [Interval(p.begin - c.begin, p.end - c.begin, p.value) for p in c.properties],
        index = len(pieces))
      piece_map[k] = result_piece
      pieces.append(result_piece)
    piece_references.append(PieceReference(c.begin, c.end, result_piece))
  return (piece_references, pieces)

def reindex_pieces(piece_references, pieces, omit_empty):
  indexes = dict()
  for i in range(0, len(pieces)):
    if pieces[i].index not in indexes:
      indexes[pieces[i].index] = i
  result_pieces = [p._replace(index = indexes[p.index]) for p in pieces]
  return (
    [p._replace(piece = result_pieces[indexes[p.piece.index] if not omit_empty or p.piece.properties else 0]) for p in piece_references],
    result_pieces)

def sort_pieces_by_usage(piece_references, pieces, omit_empty = True):
  uses = [(sum([(1 if r.piece == p else 0) for r in piece_references]), p)
          for p in pieces if not omit_empty or p.properties]
  uses.sort(reverse=True)
  return reindex_pieces(piece_references, [p[1] for p in uses], omit_empty)

PieceHalf = namedtuple('PieceHalf', ['key', 'index', 'properties', 'is_second'])
Match = namedtuple('Match', ['score', 'id1', 'id2', 'min_idx', 'max_idx', 'halves'])

def split_pieces(pieces):
  result = list()
  for piece in pieces:
    begin = clip_piece(piece, 0, piece.size // 2)
    end = clip_piece(piece, piece.size // 2, piece.size)
    result.append(Match(
      piece.index * piece.index,
      piece.index,
      piece.index,
      piece.index,
      piece.index,
      [
        PieceHalf(props_to_string(begin), piece.index, begin, False),
        PieceHalf(props_to_string(end), piece.index, end, True)
      ]))
  return result

def combine_match(first, second):
  # diff = min(abs(first.max_idx - second.min_idx - 1), abs(second.max_idx - first.min_idx - 1))
  diff = (first.min_idx - second.max_idx)
  return Match(
    first.min_idx * second.max_idx + diff * diff * diff,
    first.id1,
    second.id1,
    min(first.min_idx, second.min_idx),
    max(first.max_idx, second.max_idx),
    first.halves[:-1] + second.halves)

def find_matches(matches):
  begins = dict()
  for m in matches:
    k = m.halves[0].key
    if k in begins:
      begins[k].append(m)
    else:
      begins[k] = [m]

  result = list()
  for m in matches:
    k = m.halves[-1].key
    if k in begins:
      for b in begins[k]:
        if b != m:
          result.append(combine_match(m, b))
  result.sort()
  return result

def match_pieces(piece_references, pieces):
  sz = pieces[0].size
  pieces = split_pieces(pieces)
  matches = find_matches(pieces)
  while matches:
    pieces = [p for p in pieces if p.id1 != matches[0].id1 and p.id1 != matches[0].id2]
    pieces.append(matches[0])
    matches = find_matches(pieces)
  pieces.sort()

  result = []
  for piece in pieces:
    result += [Piece(sz // 2, h.properties, h.index) for h in piece.halves]
    # for h in piece.halves:
      # print(f'{h.index}: {h.key}')
  return reindex_pieces(piece_references, result, False)





def write_nibble_table(f, name, pieces, get_value = lambda x: x, is_inline = True):
  if is_inline:
    f.write(f'  inline constexpr u64_t {name}[] = ' + '{')
  else:
    f.write(f'  constinit const u64_t {name}[] = ' + '{')
  for p in pieces:
    props = p.properties.copy()
    props.reverse()
    prop = props.pop()
    v = 0
    f.write("\n    // Piece #0x" + f'{p.index:02X}{piece_to_string(p)}' + "\n   ")
    for i in range(0,p.size):
      if i == prop.end and props:
        prop = props.pop()
      v >>= 4
      v |= get_value(prop.value) << 60
      if i % 16 == 15:
        f.write(f' 0x{v:016X}ULL,')
        if i % 64 == 63:
          f.write("\n   ")
        v = 0
  f.write("\n  };\n\n")

def trim_piece_references(piece_references):
  cur = None
  num = 0
  for p in piece_references:
    if p.piece.index == cur:
      num += 1
    else:
      num = 1
      cur = p.piece.index
  return (piece_references[:-num], cur)

def write_index_table(f, name, piece_references, is_inline = True):
  if is_inline:
    f.write(f'  inline constexpr u8_t {name}[] = ' + "{");
  else:
    f.write(f'  constinit const u8_t {name}[] = ' + "{");
  i = 0
  for r in piece_references:
    if i % 8 == 0:
      f.write("\n   ")
      if i % 64 == 0:
        f.write(f' // U+{r.begin:06X}')
        f.write("\n   ")

    f.write(f' 0x{r.piece.index:02X}U,')
    i += 1
  f.write("\n  };\n\n")

def chunk_key(chunk):
  return ''.join([f'.{p.value}:{p.begin - chunk.begin:x}' if p.end == p.begin + 1 else
                  f'.{p.value}:{p.begin - chunk.begin:x}:{p.end - p.begin:x}' for p in chunk.properties])
def format_unicode_range(begin, end, const_size=False):
  end -= 1
  if const_size:
    return f'        {begin:06X}' if begin == end else f'{begin:06X}..{end:06X}'
  return f'{begin:04X}' if begin == end else f'{begin:04X}..{end:04X}'

def piece_to_string(chunk):
  return ''.join([f' 0x{p.begin:02X}={p.value}' if p.end == p.begin + 1 else f' 0x{p.begin:02X}-{p.end:02X}={p.value}'
    for p in chunk.properties])
def clip_piece(chunk, begin, end):
  return [Interval(max(begin, p.begin) - begin, min(end, p.end) - begin, p.value)  for p in chunk.properties if p.end > begin and p.begin < end]
def props_to_string(props):
  return ''.join([f'.{p.value}:{p.begin:x}' if p.end == p.begin + 1 else f'.{p.value}:{p.begin:x}:{p.end - p.begin:x}' for p in props])
