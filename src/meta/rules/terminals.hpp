#pragma once

#include "strings/string_view.hpp"
#include "strings/string_map_fwd.hpp"
#include "bits/fixed_bitset.hpp"
#include "bits/fixed_bitset_hash.hpp"

namespace j::meta {
  struct module;
}
namespace j::meta::inline rules {
  struct terminal final {
    u8_t index = 0U;

    J_A(AI,NODISC) inline constexpr bool operator==(const terminal &) const noexcept = default;

    J_A(AI,NODISC) bool is_special() const noexcept { return index >= 253; }
    J_A(AI,NODISC) bool is_exit() const noexcept { return index == 254; }
    J_A(AI,NODISC) bool is_epsilon() const noexcept { return index == 255; }
  };

  constexpr inline terminal exit{254};
  constexpr inline terminal epsilon{255};

  struct terminal_set_iterator final {
    bits::fixed_bitset_iterator it;

    J_A(AI,ND,HIDDEN) inline terminal operator*() const noexcept {
      return terminal{(u8_t)*it};
    }
    J_A(AI,ND,HIDDEN) inline terminal_set_iterator & operator++() noexcept {
      ++it;
      return *this;
    }
    J_A(AI,ND,HIDDEN) inline terminal_set_iterator operator++(int) noexcept {
      return {it++};
    }
    J_A(AI,NODISC,HIDDEN) inline bool operator==(const terminal_set_iterator & rhs) const noexcept = default;
  };

  struct terminal_set final {
    bits::bitset256 terminals{};

    J_A(AI,ND) inline terminal_set() noexcept = default;
    J_A(AI) inline explicit terminal_set(const bits::bitset256 & bits) noexcept
      : terminals(bits)
    { }
    J_A(AI) inline explicit terminal_set(terminal t) noexcept {
      terminals.add(t.index);
    }

    J_A(ND,AI,NODISC,HIDDEN) inline bool operator!() const noexcept {
      return !terminals;
    }
    J_A(ND,AI,NODISC,HIDDEN) inline explicit operator bool() const noexcept {
      return (bool)terminals;
    }
    J_A(ND,AI,NODISC,HIDDEN) inline bool has(terminal t) const noexcept {
      return terminals.has(t.index);
    }
    J_A(ND,AI,NODISC,HIDDEN) inline bool has_epsilon() const noexcept {
      return terminals.has(255);
    }
    J_A(ND,AI,NODISC,HIDDEN) inline bool has_exit() const noexcept {
      return terminals.has(254);
    }
    J_A(ND,AI,NODISC,HIDDEN) inline u32_t size() const noexcept {
      return terminals.size();
    }

    J_A(ND,AI,NODISC,HIDDEN) inline bool is_subset_of(const terminal_set & rhs) const noexcept {
      return terminals.is_subset_of(rhs.terminals);
    }

    [[nodiscard]] inline bool operator==(const terminal_set & rhs) const noexcept = default;

    [[nodiscard]] inline terminal_set operator+(const terminal_set & rhs) const noexcept {
      return terminal_set{terminals | rhs.terminals};
    }
    [[nodiscard]] inline terminal_set operator&(const terminal_set & rhs) const noexcept {
      return terminal_set{terminals & rhs.terminals};
    }
    [[nodiscard]] inline terminal_set operator-(const terminal_set & rhs) const noexcept {
      return terminal_set{terminals - rhs.terminals};
    }

    inline terminal_set & operator+=(const terminal_set & rhs) noexcept {
      terminals |= rhs.terminals;
      return *this;
    }
    inline terminal_set & operator+=(terminal rhs) noexcept {
      terminals.add(rhs.index);
      return *this;
    }
    inline terminal_set & operator&=(const terminal_set & rhs) noexcept {
      terminals &= rhs.terminals;
      return *this;
    }
    inline terminal_set & operator-=(const terminal_set & rhs) noexcept {
      terminals -= rhs.terminals;
      return *this;
    }
    inline terminal_set & operator-=(terminal rhs) noexcept {
      terminals.del(rhs.index);
      return *this;
    }

    J_A(AI,NODISC) inline terminal_set_iterator begin() const noexcept {
      return { terminals.begin() };
    }
    J_A(AI,NODISC) inline terminal front() const noexcept {
      return { (u8_t)*terminals.begin() };
    }
    J_A(AI,NODISC) inline terminal_set_iterator end() const noexcept {
      return { terminals.end() };
    }
  };

  struct terminal_def final {
    strings::const_string_view name{};
    u8_t min_operands = 0;
    u8_t max_operands = 255;
  };

  struct terminal_collection final {
    u8_t size = 0U;
    terminal_def defs[128]{};
    terminal_set any_terminal{};
    terminal_set any_terminal_or_exit{exit};

    u8_t add(strings::const_string_view name, u8_t min_operands, u8_t max_operands) noexcept;
    J_A(AI,ND,NODISC,RNN) const terminal_def * begin() const noexcept { return defs; }
    J_A(AI,ND,NODISC,RNN) const terminal_def * end() const noexcept { return defs + size; }

    J_A(AI,NODISC) inline const terminal_def & def_of(terminal t) const noexcept {
      return defs[t.index];
    }

    [[nodiscard]] strings::const_string_view name_of(module * mod, terminal t) const noexcept;
  };

  struct terminal_hash final {
    J_A(AI,ND,NODISC,FLATTEN) inline u32_t operator()(terminal t) const noexcept {
      return crc32(17, t.index);
    }
  };

  struct terminal_set_hash final {
    [[no_unique_address]] bits::fixed_bitset_hash<4> hash;
    J_A(NI,ND,NODISC,FLATTEN) inline u32_t operator()(const terminal_set & ts) const noexcept {
      return hash(ts.terminals);
    }
  };
}
