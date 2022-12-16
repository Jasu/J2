#pragma once

#include "mem/shared_ptr_fwd.hpp"

namespace j::meta::inline rules {
  struct pat;
  struct pat_elem;
  struct nonterminal;
  struct grammar;
  struct reduction_group;
  struct reduction;
  struct reductions;
  struct nt_data_type;
  struct terminal;

  using pat_p = mem::shared_ptr<pat>;
  using red_p = mem::shared_ptr<reductions>;
  using nt_p = nonterminal*;
  using nt_data_type_p = mem::shared_ptr<nt_data_type>;

  enum class pat_layer : i8_t {
    inner,
    ambient,
    main,
    standard,
    recovery,
  };

  enum class pat_prec_type : i8_t {
    special,
    shift,
    shift_any,
    shift_anys,
    reduce,
    rewrite,
    reduce_any,
    reduce_anys,
  };

  struct pat_precedence final {
    i32_t precedence = 0;
    J_BOILERPLATE(pat_precedence, CTOR_NE_ND, EQ)

    J_A(AI,ND) inline pat_precedence(pat_layer layer, i8_t precedence, pat_prec_type type, i32_t index = 0) noexcept
      : precedence(((i32_t)layer << 28) + (-((i32_t)precedence - I8_MAX) << 20) + ((i32_t)type << 16) + index)
    { }
    J_A(AI,NODISC) inline i8_t prec() const noexcept {
      return I8_MAX - (u8_t)(precedence >> 20);
    }
    J_A(AI,NODISC) inline pat_layer layer() const noexcept {
      return (pat_layer)(precedence >> 28);
    }
    J_A(AI,NODISC) inline bool is_same_layer(const pat_precedence & rhs) const noexcept {
      return (precedence >> 28) == (rhs.precedence >> 28);
    }
    J_A(AI,NODISC) inline pat_precedence with_layer(pat_layer layer) const noexcept {
      return pat_precedence((precedence & ((1 << 28) - 1)) | ((i32_t)layer << 28));
    }
    J_A(AI) inline void set_layer(pat_layer layer) noexcept {
      precedence &= (1 << 28) - 1;
      precedence |= (i32_t)layer << 28;
    }
    J_A(AI,NODISC) inline bool operator<(const pat_precedence & rhs) const noexcept {
      return precedence < rhs.precedence;
    }
    J_A(AI,NODISC) inline bool operator<=(const pat_precedence & rhs) const noexcept {
      return precedence <= rhs.precedence;
    }
    J_A(AI,NODISC) inline bool operator>(const pat_precedence & rhs) const noexcept {
      return precedence > rhs.precedence;
    }
    J_A(AI,NODISC) inline bool operator>=(const pat_precedence & rhs) const noexcept {
      return precedence >= rhs.precedence;
    }

  private:
    J_A(AI) inline explicit pat_precedence(i32_t precedence) noexcept
      : precedence(precedence)
    { }
  };

  template<typename T>
  J_A(AI,NODISC) inline i32_t precedence_key(const T & t) noexcept {
    return t.precedence.precedence;
  }
}
