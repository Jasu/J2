#pragma once

#include "os/dwarf.hpp"
#include "strings/string_view.hpp"
#include "strings/string.hpp"
#include "containers/vector.hpp"

namespace j::os::dwarf {
  struct dwarf_context;
  struct dwarf_ref;
  struct def_child_base;

  struct dwarf_unit_info final {
    const char * name = nullptr;
    const char * compdir = nullptr;

    J_A(AI,ND,HIDDEN) inline operator bool() { return (bool)name; }
    J_A(AI,ND,HIDDEN) inline bool operator!() { return !name; }
  };

  struct dwarf_info final {
    vector<dwarf_unit_info> units;
  };

  void resolve_ref(dwarf_context * J_AA(NN) ctx, dwarf_ref & ref) noexcept;
  void resolve_parent(dwarf_context * J_AA(NN) ctx, def_child_base * & ref) noexcept;

  struct dwarf_reader final {
    J_A(AI,ND,HIDDEN) inline dwarf_reader(strings::const_string_view path) noexcept : path(path) { }
    J_A(NODISC) dwarf_info read();
    ~dwarf_reader();
  private:
    dwarf_context * ctx = nullptr;
    strings::const_string_view path;
  };
}
