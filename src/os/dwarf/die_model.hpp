#pragma once

#include "os/dwarf.hpp"
#include "os/dwarf_reader.hpp"

namespace j::os::dwarf {
  struct def_base;

  struct dwarf_ref final {
    const char *ref = nullptr;
    def_base *ptr = nullptr;

    J_A(AI,ND) inline dwarf_ref() noexcept = default;

    J_A(AI,ND) inline dwarf_ref(null_t) noexcept { }

    J_A(AI,ND) inline explicit dwarf_ref(const char * ref) noexcept
      : ref(ref)
    { }

    J_A(AI,ND) inline dwarf_ref(def_base * ptr) noexcept
      : ptr(ptr)
    { }

    J_A(AI,NODISC) inline operator bool() const noexcept { return ref || ptr; }

    J_A(AI,ND,NODISC,HIDDEN) inline bool operator==(const dwarf_ref & rhs) const noexcept = default;
  };


  enum def_type {
    dt_ns,
    dt_ns_inline,

    dt_class,
    dt_struct,
    dt_union,

    dt_enum_type,
    dt_enum_class_type,

    dt_base_type,
    dt_typedef,

    dt_fn_decl,
    dt_fn_type,

    dt_array_type,

    dt_mem_ptr_type,

    dt_ptr_type,
    dt_ref_type,
    dt_rref_type,
    dt_const_type,
    dt_restrict_type,

    dt_max_named = dt_typedef,
    dt_min_wrapped = dt_ptr_type,
    dt_max_wrapped = dt_restrict_type,
  };

  struct def_child_base;

  struct def_base {
    def_type type;

    J_A(AI,ND,HIDDEN) inline def_base(def_type type) noexcept : type(type) { }

    virtual ~def_base();

    virtual void resolve(dwarf_context * J_AA(NN) ctx) noexcept;

    virtual void dump_name(const def_child_base *) const noexcept = 0;
    virtual bool eq(const def_base & rhs) const noexcept;
    virtual u32_t hash() const noexcept;
  };

  J_A(AI,ND,NODISC,HIDDEN) inline bool operator==(const def_base & lhs, const def_base & rhs) noexcept {
    return lhs.type == rhs.type && lhs.eq(rhs);
  }

  struct def_named_base : def_base {
    const char * name;

    J_A(AI,ND,HIDDEN) inline def_named_base(def_type type, const char * name) noexcept
      : def_base(type),
        name(name)
    { }

    void dump_name(const def_child_base *) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    u32_t hash() const noexcept override;
  };

  struct def_child_base : def_base {
    const char * name;
    def_child_base * parent;


    J_A(AI,ND,HIDDEN) inline def_child_base(def_type type, const char *name, def_child_base *parent) noexcept
      : def_base(type),
      name(name),
      parent(parent)
    { }

    void dump_name(const def_child_base *) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    u32_t hash() const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
  };

  struct def_ns final : def_child_base {
    J_A(AI,ND,HIDDEN) inline def_ns(const char *name, def_child_base *parent, bool is_inline) noexcept
      : def_child_base(is_inline ? dt_ns_inline : dt_ns, name, parent){
    }
  };


  struct def_base_type final : def_named_base {
    J_A(AI,ND,HIDDEN) inline def_base_type(const char * J_AA(NN) name) noexcept
      : def_named_base(dt_base_type, name)
    { }
  };

  struct def_wrapped_type final : def_base {
    dwarf_ref target;

    J_A(AI,ND,HIDDEN) inline def_wrapped_type(dwarf_ref target, def_type wrapper) noexcept
      : def_base(wrapper),
        target(target)
    { }

    u32_t hash() const noexcept override;
    void dump_name(const def_child_base * scope) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
  };

  struct def_array_type final : def_base {
    dwarf_ref target;
    i32_t size;

    J_A(AI,ND,HIDDEN) inline def_array_type(dwarf_ref target, i32_t size) noexcept
      : def_base(dt_array_type),
        target(target),
        size(size)
    { }

    void dump_name(const def_child_base * scope) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
    u32_t hash() const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
  };

  struct def_mem_ptr_type final : def_base {
    dwarf_ref target;
    dwarf_ref class_type;

    J_A(AI,ND,HIDDEN) inline def_mem_ptr_type(dwarf_ref target, dwarf_ref class_type) noexcept
      : def_base(dt_mem_ptr_type),
        target(target),
        class_type(class_type)
    { }

    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
    u32_t hash() const noexcept override;
    void dump_name(const def_child_base * scope) const noexcept override;
    bool eq(const def_base & rhs) const noexcept override;
  };

  struct def_typedef final : def_child_base {
    dwarf_ref target;

    def_typedef(const char *name, def_child_base *parent, dwarf_ref target) noexcept
      : def_child_base(dt_typedef, name, parent),
        target(target)
    { }

    bool eq(const def_base & rhs) const noexcept override;
    void resolve(dwarf_context * J_AA(NN) ctx) noexcept override;
    u32_t hash() const noexcept override;
  };

  struct def_class_type final : def_child_base {
    J_A(AI,ND,HIDDEN) inline def_class_type(const char *name, def_child_base *parent, def_type type) noexcept
      : def_child_base(type, name, parent)
    { }
  };

  struct def_enum_type final : def_child_base {
    J_A(AI,ND,HIDDEN) inline def_enum_type(const char *name, def_child_base *parent, bool is_class) noexcept
      : def_child_base(is_class ? dt_enum_class_type : dt_enum_type, name, parent)
    { }
  };

  struct def_hash final {
    J_A(NODISC) bool operator()(def_base * J_AA(NN) lhs, def_base * J_AA(NN) rhs) const noexcept;
    J_A(NODISC) u32_t operator()(def_base * J_AA(NN) lhs) const noexcept;
    J_A(NODISC) bool operator()(const def_base & lhs, const def_base & rhs) const noexcept;
    J_A(NODISC) u32_t operator()(const def_base & lhs) const noexcept;
  };
}
