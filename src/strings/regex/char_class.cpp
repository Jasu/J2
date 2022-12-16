#include "strings/regex/char_class.hpp"
#include "strings/string.hpp"
#include "strings/format.hpp"
#include "strings/unicode/utf8.hpp"
#include "hzd/string.hpp"

namespace j::strings::regex {
  namespace  {
    inline const u8_t empty[] = {0,0,0,0};
    inline const u8_t max[] = {63,63,63,63};
    J_A(AI,NODISC) inline bool is_min(const u8_t * J_AA(NN) it, i32_t len) noexcept {
      return len == 0 || (!(it[0] & 63) && (len == 1 || (!(it[1] & 63) && (len == 2 || (!(it[2] & 63) && (len == 3 || !(it[3] & 63)))))));
    }
    J_A(AI,NODISC) inline bool is_max(const u8_t * J_AA(NN) it, i32_t len) noexcept {
      return len == 0 || ((it[0] & 63) == 63 && (len == 1 || ((it[1] & 63) == 63 && (len == 2 || ((it[2] & 63) == 63 && (len == 3 || (it[3] & 63) == 63))))));
    }

    inline const const_string_view min_utf8_seqs[5]{
      "",
      {"\x00", 1},
      "\u0080",
      "\u0800",
      "\u10000",
    };

    inline const char *const max_utf8_seqs[5]{
      "",
      "\x7F",
      "\u07FF",
      "\uFFFF",
      "\u10FFF",
    };
  }

  J_A(NODISC) i32_t char_tree::some_index(u8_t idx) const noexcept {
    u64_t mask = idx == 64 ? 0 : 1UL << idx;
    return bits::popcount(some & (mask - 1));
  }
// ƿ C6 BF CP 447 0x1BF
// Ɛ C6 90 CP 400 0x190
  char_tree::~char_tree() {
    i32_t num = num_some();
    for (i32_t i = 0; i != num; ++i) {
      children[i].~char_tree();
    }
    j::free(children);
  }

  void char_tree::add(const u8_t * J_AA(NN) ch, i32_t len) noexcept {
    u8_t idx = *ch & 63;
    J_ASSERT(len > 0 && len <= 4);
    u64_t mask = 1UL << idx;
    if (all & mask) {
      return;
    }
    if (len == 1) {
      all |= mask;
      return;
    }
    char_tree * ct = get_or_add_some(idx);
    ct->add(ch + 1, len - 1);
    if (ct->is_full()) {
      remove_some(idx, idx + 1);
      all |= mask;
    }
  }

  void char_tree::add(const u8_t * J_AA(NN) begin, const u8_t * J_AA(NN) end, i32_t len) {
    u8_t begin_idx = *begin & 63;
    u8_t end_idx = *end & 63;
    --len;
    J_ASSERT(begin != end && len >= 0 && len < 4 && begin_idx <= end_idx);
    if (!is_min(begin + 1, len)) {
      if (begin_idx == end_idx) {
        add_some(begin_idx, begin + 1, end + 1, len);
        return;
      }
      add_some(begin_idx, begin + 1, max, len);
      ++begin_idx;
    }
    if (!is_max(end + 1, len)) {
      add_some(end_idx, empty, end + 1, len);
      --end_idx;
    }
    add_all(begin_idx, end_idx);
  }

  void char_tree::add_some(u8_t at, const u8_t * J_AA(NN) begin, const u8_t * J_AA(NN) end, i32_t len) noexcept {
    J_ASSERT(len > 0 && len < 4 && at < 63);
    u64_t mask = (1UL << at);
    if (all & mask) {
      return;
    }
    char_tree * ch = get_or_add_some(at);
    ch->add(begin, end, len);
    if (ch->is_full()) {
      remove_some(at, at + 1);
      all |= mask;
    }
  }

  void char_tree::add_all(u32_t begin, u32_t end) noexcept {
    if (begin > end) {
      return;
    }
    J_ASSERT(begin <= end && end < 64);
    u64_t m = -(1UL << begin);
    if (end != 63) {
      m += 1UL << (end + 1);
    }
    all |= m;
    remove_some(begin, end);
  }

  void char_tree::remove_some(u8_t begin, u8_t end) noexcept {
    if (begin > end) {
      return;
    }
    J_ASSERT(end < 64 && begin <= end);
    u64_t mask = -(1UL << begin);
    if (end != 63) {
      mask += 1UL << (end + 1);
    }
    i32_t sz = bits::popcount(mask & some);
    if (!sz) {
      return;
    }
    i32_t begin_idx = some_index(begin);
    for (i32_t i = begin_idx; i < begin_idx + sz; ++i) {
      children[i].~char_tree();
    }
    some &= ~mask;
    if (!some) {
      j::free(children);
      children = nullptr;
    } else{
      j::memmove(children + begin_idx, children + begin_idx + sz, sz  * sizeof(char_tree));
      // children = (char_tree*)j::reallocate(children, sizeof(char_tree) * sz);
    }
  }

  char_tree_iterator::char_tree_iterator(const char_tree * ct) noexcept
    : depth(0)
  {
    while (ct) {
      J_ASSERT(depth < 4);
      its[depth] = {ct, 0};
      if (its[depth].is_some()) {
        ct = ct->children;
      } else {
        ct = nullptr;
      }
      ++depth;
    }
  }

  J_A(ND) bool char_tree_iterator::next(bool set) noexcept {
    J_ASSERT(depth);
    while (!its[depth - 1] || !its[depth - 1].next(set)) {
      if (!--depth) {
        return false;
      }
      ++its[depth - 1].idx;
    }
    while (its[depth - 1].is_some()) {
      its[depth] = { its[depth - 1].tree->get_some(its[depth - 1].idx), 0 };
      its[depth].next(set);
      ++depth;
    }
    return true;
  }

  u32_t char_tree_iterator::get_code_point() const noexcept {
    J_ASSERT(*this);
    u32_t codepoint = its[0].idx;
    i32_t len = !(codepoint & 0b00100000) ? 2 : !(codepoint & 0b00010000) ? 3 : 4;
    if (len == 3) {
      codepoint &= 0xF;
    } else if (len == 4) {
      codepoint &= 0x7;
    }
    for (i32_t i = 1; i != len; ++i) {
      codepoint <<= 6;
      codepoint |= i < depth ? its[i].idx : 0;
    }
    return codepoint;
  }

  void char_tree_iterator::get_char(char * J_AA(NN) to) const noexcept {
    utf8_encode(to, get_code_point());
  }

  void char_tree_iterator::get_prev_char(char * J_AA(NN) to) const noexcept {
    u32_t cp = *this ? get_code_point() - 1 : 0x1FFFFU;
    utf8_encode(to, cp);
  }

  J_A(RNN,NODISC) char_tree * char_tree::get_or_add_some(u8_t begin) noexcept {
      J_ASSERT(begin < 64);
      i32_t idx = some_index(begin);
      u64_t mask = 1UL << begin;
      if (!(some & mask)) {
        i32_t sz = num_some();
        some |= mask;
        children = (char_tree*)j::reallocate(children, sizeof(char_tree) * (sz + 1));
        j::memmove(children + idx + 1, children + idx, (sz - idx) * sizeof(char_tree));
        j::memzero(children + idx, sizeof(char_tree));
      }
      return children + idx;
    }

  J_A(NODISC) i32_t ascii_mask::size() const noexcept {
    return bits::popcount(mask[0]) + bits::popcount(mask[1]);
  }
  void ascii_mask::set(u8_t begin, u8_t end) noexcept {
    J_ASSERT(begin < end && end <= 128);
    if (begin < 64) {
      u64_t m = -(1UL << begin);
      if (end < 64) {
        m += 1UL << end;
      }
      mask[0] |= m;
    }
    if (end > 64) {
      u64_t m = -(1UL << (begin < 64 ? 0 : begin - 64));
      if (end != 128) {
        m += 1UL << (end - 64);
      }
      mask[1] |= m;
    }
  }
  void char_class::add_range(const_string_view min, const_string_view max) noexcept {
    J_ASSERT(min.size() && min.size() <= max.size() && max.size() <= 4);
    if (min.size() == 1) {
      ascii_mask.set((u8_t)min[0], max.size() != 1 ? 128 : (u8_t)max[0] + 1);
      min = min_utf8_seqs[2];
    }
    if (max.size() != 1) {
      while (min.size() != max.size()) {
        utf8_tree.add((const u8_t*)min.begin(), (const u8_t*)max_utf8_seqs[min.size()], min.size());
        min = min_utf8_seqs[min.size() + 1];
      }
      utf8_tree.add((const u8_t*)min.begin(), (const u8_t*)max.begin(), min.size());
    }
  }

  string format_mask(const ascii_mask & mask) noexcept {
    string result;
    for (u8_t i = 0; i != 128; ++i) {
      if (mask.has(i)) {
        if (result) {
          result.push_back(',');
        }
        result += format("'{}'", (char)i);
        u8_t ch = i;
        for (; i != 128 && mask.has(i); ++i) { }
        if (i > ch - 1) {
          result += format("{}'{}'", i > ch - 2 ? '-' : ',', (char)(i - 1));
        }
      }
    }
    return result;
  }

  J_A(NODISC) string format_char_tree(const char_tree & tree) noexcept {
    string result;
    for (char_tree_iterator it(&tree); it && it.next(true);) {
      u32_t cp = it.get_code_point();
      it.next(false);
      u32_t cp2 = it.get_code_point() - 1;
      if (result) {
        result.push_back(',');
      }
      result += format("'{codepoint}'(U+{:04X})", cp, cp);
      if (cp != cp2) {
        result += format("{}'{codepoint}'(U+{:04X})", cp2 == cp + 1 ? ',' : '-', cp2, cp2);
      }
    }

    return result;
  }
}
