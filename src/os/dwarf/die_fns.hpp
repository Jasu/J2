#pragma once

#include "os/dwarf/die_model.hpp"

namespace j::os::dwarf {
  struct fn_param final {
    dwarf_ref type;
    bool is_synthetic;

    J_A(AI,ND,NODISC) inline bool operator==(const fn_param & rhs) const noexcept = default;
  };

  using fn_params = vector<fn_param>;

  struct fn_signature final {
    dwarf_ref result;
    fn_params params;
    J_A(NODISC) bool operator==(const fn_signature & rhs) const noexcept;
  };

  struct def_fn_type final : def_base {
    fn_signature sig;

    J_A(AI,ND,HIDDEN) inline explicit def_fn_type(fn_signature && sig) noexcept
      : def_base(dt_fn_type),
        sig(static_cast<fn_signature &&>(sig))
    { }

    void dump_name(const def_child_base * scope) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    u32_t hash() const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
  };

  struct def_fn_decl final : def_child_base {
    fn_signature sig;

    J_A(AI,ND,HIDDEN) inline def_fn_decl(const char *name, def_child_base *parent, fn_signature && sig) noexcept
      : def_child_base(dt_fn_decl, name, parent),
        sig(static_cast<fn_signature &&>(sig))
    { }

    void dump_name(const def_child_base * scope) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    u32_t hash() const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
  };
}
