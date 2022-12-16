#pragma once

#include "os/dwarf/unit_ctx.hpp"
#include "os/dwarf_attr.hpp"
#include "os/dwarf_reader_base.hpp"

namespace j::os::dwarf {
  struct dwarf_ref;
    enum attr_spec_type {
      attr_type_str,
      attr_type_ref,
      attr_type_bool,
      attr_type_i32,
      attr_type_u32,
    };

  struct attr_spec final {
    unsigned int attr;
    attr_spec_type type;
    union {
      const char ** str = nullptr;
      dwarf_ref * ref;
      bool * boolean;
      i32_t * i32;
      u32_t * u32;
    } ptr;

    J_A(AI) inline attr_spec(dw_at at, const char ** J_AA(NN) p) noexcept
    : attr(at),
      type(attr_type_str),
      ptr{.str = p}
      { }

    J_A(AI) inline attr_spec(dw_at at, dwarf_ref * J_AA(NN) p) noexcept
    : attr(at),
      type(attr_type_ref),
      ptr{.ref = p}
      { }

    J_A(AI) inline attr_spec(dw_at at, bool * J_AA(NN) p) noexcept
    : attr(at),
      type(attr_type_bool),
      ptr{.boolean = p}
      { }

    J_A(AI) inline attr_spec(dw_at at, i32_t * J_AA(NN) p) noexcept
    : attr(at),
      type(attr_type_i32),
      ptr{.i32 = p}
      { }
    J_A(AI) inline attr_spec(dw_at at, u32_t * J_AA(NN) p) noexcept
    : attr(at),
      type(attr_type_u32),
      ptr{.u32 = p}
      { }
  };

  struct die_ctx final {
    const abbrev * abbrev;
    const char * die_begin;
    unit_ctx * unit;
    die_ctx * parent;

    void skip_attrs(base_reader & ir);
    void read_attrs(base_reader & ir, const attr_spec * J_AA(NN) spec, u32_t count);
  };

  struct die_reader final {
    die_ctx * die = nullptr;
    base_reader ir;
    unit_ctx * unit = nullptr;

    J_A(AI,NODISC) inline explicit operator bool() const noexcept { return (bool)ir; }
    J_A(AI,NODISC) inline bool operator!() const noexcept { return !ir; }

    J_A(AI,NODISC) inline bool has_children() const noexcept { return die->abbrev->has_children; }
    J_A(AI,NODISC) inline const char * die_begin() const noexcept { return die->die_begin; }

    J_A(AI,ND) inline void skip_attrs() {
      die->skip_attrs(ir);
    }

    template<i32_t N>
    J_A(AI,ND) inline void read_attrs(const attr_spec (&& spec)[N]) {
      die->read_attrs(ir, spec, N);
    }

    template<i32_t N>
    J_A(AI,ND) inline void read_attrs(const attr_spec (& spec)[N]) {
      die->read_attrs(ir, spec, N);
    }

    J_A(AI,ND) inline void read_attrs(const attr_spec * J_AA(NN) spec, u32_t count) {
      die->read_attrs(ir, spec, count);
    }

    void open(die_ctx * J_AA(NN) target, unit_ctx * J_AA(NN) u);
    void open(die_ctx * J_AA(NN) target, unit_ctx * J_AA(NN) u, const char * J_AA(NN) at);
    J_A(NODISC) u32_t next();
    void skip_children();
    void enter_children(die_ctx * J_AA(NN) target);
  };
}
