#pragma once

#include "lisp/values/imm_type.hpp"

namespace j::lisp::air::inline values {
  enum J_AT(HIDDEN) rep : u8_t {
    rep_none = 0U,
    rep_tagged,
    rep_untagged,
    /// Applies to vectors and rest arguments. The array is represented by two registers,
    /// one for begin, and one for size.
    rep_range,
  };

  struct J_AT(HIDDEN) rep_spec final {
    u8_t value = 0U;

    J_A(AI,NODISC) inline u8_t mask() const noexcept {
      return (value & rep_requirement_mask) << 1U;
    }
    J_A(AI,NODISC) inline u8_t hint() const noexcept {
      return (value & rep_hint_mask) >> 2U;
    }
    J_A(AI,NODISC) inline bool has_rep() const noexcept {
      return value >> 6;
    }
    J_A(AI,NODISC) inline enum rep rep() const noexcept {
      return (enum rep)(value >> 6);
    }
    inline void set_rep(enum rep rep) noexcept {
      value = (value & 0b00111111U) | ((u8_t)rep << 6);
    }
    inline void set_hint(u8_t hint) noexcept {
      value = (value & ~rep_hint_mask) | (hint << 2U);
    }
    inline void set_hint(enum rep rep) noexcept {
      value = (value & ~rep_hint_mask) | (0b100U << rep);
    }
    inline void add(enum rep rep, bool prefer = false) noexcept {
      value |= (prefer ? 0b001001U : 0b000001U) << ((u8_t)rep - 1U);
    }

    inline void set_mask(enum rep rep) noexcept {
      value = (value & ~rep_requirement_mask & ~rep_hint_mask) | (1U << (rep - 1U));
    }

    inline void set_mask(u8_t hint) noexcept {
      value = (value & ~rep_requirement_mask & ~rep_hint_mask) | (hint << 2U | hint >> 1U);
    }

    J_A(AI,NODISC) constexpr inline bool has(enum rep rep) const noexcept {
      return value & (1U << ((u8_t)rep - 1U));
    }

    J_A(NODISC,AI,HIDDEN) inline bool has_hint(enum rep rep) const noexcept {
      return value & (0b100U << (u8_t)rep);
    }

    J_A(NODISC,PURE) enum rep get_best() const noexcept {
      if (!mask()) {
        return rep_none;
      }
      return (enum rep)(bits::ctz_safe(hint() ? hint() : mask()));
    }

    J_A(AI) constexpr rep_spec() noexcept = default;

    J_A(AI) inline explicit constexpr rep_spec(u8_t value) noexcept
      : value(value)
    { }

    J_A(AI) inline explicit constexpr rep_spec(enum rep value) noexcept
      : value(1U << (value - 1U))
    { }


    J_INLINE_GETTER constexpr explicit operator bool() const noexcept {
      return value;
    }

    J_INLINE_GETTER constexpr bool operator!() const noexcept {
      return !value;
    }

    [[nodiscard]] rep_spec operator&(rep_spec rhs) const noexcept {
      u8_t new_value = value & rhs.value;
      if (!(new_value & rep_hint_mask)) {
        new_value |= (value | rhs.value) & (new_value << 3);
      }
      return rep_spec(new_value);
    }

    rep_spec & operator&=(rep_spec rhs) noexcept {
      u8_t new_value = value & rhs.value;
      if (!(new_value & rep_hint_mask)) {
        value = new_value | ((value | rhs.value) & (new_value << 3));
      } else {
        value = new_value;
      }
      return *this;
    }

    [[nodiscard]] rep_spec operator|(rep_spec rhs) const noexcept {
      u8_t new_value = value | rhs.value;
      u8_t new_hint = value & rhs.value & rep_hint_mask;
      if (new_hint) {
        new_value = (new_value & ~rep_hint_mask) | new_hint;
      } else if (!(new_value & rep_hint_mask)) {
        new_value |= (value & rhs.value) << 3;
      }
      return rep_spec(new_value);
    }

    rep_spec & operator|=(rep_spec rhs) noexcept {
      u8_t new_value = value | rhs.value;
      u8_t new_hint = value & rhs.value & rep_hint_mask;
      if (new_hint) {
        new_value = (new_value & ~rep_hint_mask) | new_hint;
      } else if (!(new_value & rep_hint_mask)) {
        new_value |= (value & rhs.value) << 3;
      }
      value = new_value;
      return *this;
    }

    J_A(AI,ND,NODISC,HIDDEN) bool operator==(rep_spec rhs) const noexcept {
      return value == rhs.value;
    }

    J_A(HIDDEN,ND) const static rep_spec none;
    J_A(HIDDEN,ND) const static rep_spec any;
    J_A(HIDDEN,ND) const static rep_spec tagged_or_untagged;
    J_A(HIDDEN,ND) const static rep_spec tagged;
    J_A(HIDDEN,ND) const static rep_spec untagged;
    J_A(HIDDEN,ND) const static rep_spec range;
    J_A(HIDDEN,ND) const static rep_spec range_or_untagged;
    J_A(HIDDEN,ND) const static rep_spec range_or_tagged;

  private:
    J_A(HIDDEN,ND) static constexpr inline u8_t rep_requirement_mask   = 0b00000111U;
    J_A(HIDDEN,ND) static constexpr inline u8_t rep_hint_mask          = 0b00111000U;
  };

  //
  //                                                       HINTS                   REQUIREMENTS
  //                                                       Tagged ╾┈┈┈┈┈┈╮  ╭────╼ Range
  //                                                       Untagged ╾┈┈╮ ┊  │ ╭──╼ Untagged
  //                                                       Range ╾┈┈┈╮ ┊ ┊  │ │ ╭╼ Tagged
  //                                                                 ╽ ╽ ╽  ╽ ╽ ╽
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::none              {0b000000U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::any               {0b000111U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::tagged            {0b000001U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::untagged          {0b000010U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::range             {0b000100U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::range_or_untagged {0b000110U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::range_or_tagged   {0b000101U};
  J_A(ND,HIDDEN) constexpr inline rep_spec rep_spec::tagged_or_untagged{0b000011U};

  J_AV(HIDDEN) extern constinit const rep_spec possible_reps[num_imm_types];

  J_A(NODISC,CONST) rep_spec get_possible_reps(imm_type_mask mask) noexcept;
}
