#pragma once

#include "hzd/mem.hpp"
#include "bits/bitops.hpp"
#include "exceptions/assert_lite.hpp"
#include "hzd/utility.hpp"
#include "strings/string.hpp"

namespace j::strings::regex {

  struct char_tree final {
    J_BOILERPLATE(char_tree, CTOR_NE_ND, COPY_DEL)

    u64_t all = 0;
    u64_t some = 0;
    char_tree * children = nullptr;

    J_A(AI,NODISC) inline bool is_full() const noexcept { return all == U64_MAX; }
    J_A(NODISC,HIDDEN) i32_t some_index(u8_t idx) const noexcept;
    J_A(AI,NODISC) inline i32_t num_some() const noexcept {
      return bits::popcount(some);
    }

    inline char_tree(char_tree && rhs) noexcept
      : all(rhs.all), some(rhs.some), children(rhs.children)
    {
      rhs.all = rhs.some = 0;
      rhs.children = nullptr;
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept { return !all && !some; }
    J_A(AI,NODISC) explicit inline operator bool() const noexcept { return all || some; }
    ~char_tree();

    void add(const u8_t * J_AA(NN) ch, i32_t len) noexcept;
    void add(const u8_t * J_AA(NN) begin, const u8_t * J_AA(NN) end, i32_t len);
    void add_some(u8_t at, const u8_t * J_AA(NN) begin, const u8_t * J_AA(NN) end, i32_t len) noexcept;
    void add_all(u32_t begin, u32_t end) noexcept;

    J_A(AI,RNN,NODISC) inline const char_tree * get_some(u8_t begin) const noexcept {
      J_ASSERT(children && begin < 64 && some & (1UL << begin));
      return children + some_index(begin);
    }

    J_A(RNN,NODISC) char_tree * get_or_add_some(u8_t begin) noexcept;

    void remove_some(u8_t begin, u8_t end) noexcept;
  };

  struct single_char_tree_iterator final {
    const char_tree * tree = nullptr;
    u8_t idx = 64;
    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept {return idx != 64; }
    J_A(AI,ND,NODISC) inline bool operator!() const noexcept {return idx == 64; }
    J_A(AI,ND) inline bool next(bool set) noexcept {
      J_ASSERT(tree && idx < 64);
      for (u64_t mask = 1UL << idx; mask && !(mask & tree->some) && (set == !(mask & tree->all)); mask <<= 1, ++idx) { }
      return idx < 64;
    }
    J_A(NODISC,AI,ND) inline bool is_all() noexcept {
      J_ASSERT(tree && idx < 64);
      return tree->all & (1UL << idx);
    }
    J_A(NODISC,AI,ND) inline bool is_some() noexcept {
      J_ASSERT(tree && idx < 64);
      return tree->some & (1UL << idx);
    }
  };

  struct char_tree_iterator final {
    single_char_tree_iterator its[4] = {};
    i32_t depth = 0;

    char_tree_iterator(const char_tree * ct) noexcept;
    J_A(ND) bool next(bool set) noexcept;
    u32_t get_code_point() const noexcept;
    void get_char(char * J_AA(NN) to) const noexcept;
    void get_prev_char(char * J_AA(NN) to) const noexcept;

    J_A(AI,ND,NODISC) inline explicit operator bool() const noexcept { return depth; }
    J_A(AI,ND,NODISC) inline bool operator!() const noexcept {return !depth; }
  };

  struct ascii_mask final {
    u64_t mask[2] = {0, 0};

    J_A(AI,NODISC) inline ascii_mask operator|(const ascii_mask & rhs) const noexcept {
      return {mask[0] | rhs.mask[0], mask[1] | rhs.mask[1]};
    }
    J_A(AI,NODISC) inline ascii_mask operator-(const ascii_mask & rhs) const noexcept {
      return {mask[0] & ~rhs.mask[0], mask[1] & ~rhs.mask[1]};
    }
    J_A(AI,NODISC) inline ascii_mask operator~() const noexcept {
      return {~mask[0], ~mask[1]};
    }

    J_A(AI,NODISC) inline bool operator==(const ascii_mask & rhs) const noexcept {
      return mask[0] == rhs.mask[0] && mask[1] == rhs.mask[1];
    }
    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return !mask[0] && !mask[1];
    }
    J_A(AI,NODISC) explicit inline operator bool() const noexcept {
      return mask[0] || mask[1];
    }
    J_A(NODISC) i32_t size() const noexcept;

    J_A(AI) inline ascii_mask & operator|=(const ascii_mask & rhs) noexcept {
      mask[0] |= rhs.mask[0];
      mask[1] |= rhs.mask[1];
      return * this;
    }
    J_A(AI) inline ascii_mask & operator-=(const ascii_mask & rhs) noexcept {
      mask[0] &= ~rhs.mask[0];
      mask[1] &= ~rhs.mask[1];
      return * this;
    }

    J_A(AI,NODISC) inline bool has(u8_t ch) const noexcept {
      J_ASSERT(ch < 128);
      return mask[ch >= 64 ? 1 : 0] & (1UL << (ch & 63));
    }

    J_A(AI) inline void set(u8_t ch) noexcept {
      J_ASSERT(ch < 128);
      mask[ch >= 64 ? 1 : 0] |= 1UL << (ch & 63);
    }

    void set(u8_t begin, u8_t end) noexcept;
  };

  struct char_class {
    ascii_mask ascii_mask;
    char_tree utf8_tree;

    J_A(AI,ND) inline void add_char(const_string_view ch) noexcept {
      i32_t sz = ch.size();
      if (sz == 1) {
        ascii_mask.set((u8_t)ch[0]);
      } else {
        utf8_tree.add((const u8_t*)ch.begin(), sz);
      }
    }

    void add_range(const_string_view min, const_string_view max) noexcept;
  };

  J_A(NODISC) string format_mask(const ascii_mask & mask) noexcept;
  J_A(NODISC) string format_char_tree(const char_tree & tree) noexcept;
}
