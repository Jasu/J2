#include "os/dwarf/die_model.hpp"
#include "hzd/crc32.hpp"
#include "containers/span.hpp"
#include "os/dwarf/die_fns.hpp"
#include "logging/global.hpp"

namespace j::os::dwarf {
  J_A(NODISC) bool fn_signature::operator==(const fn_signature & rhs) const noexcept = default;
  namespace {
    struct J_AT(HIDDEN) name_fmt final {
      const char *fmt;
      const char *sep;
      const char *anon;
    };

    J_A(ND) inline constexpr name_fmt name_formats[dt_max_named + 1]{
      [dt_ns]              = {"{#bright_cyan}{}{/}", "{#cyan}::{/}", "{#blue,italic}(anon ns){/}"},
      [dt_ns_inline]       = {"{#light_gray}{}{/}", "{#light_gray}::{/}", "{#light_gray,italic}(anon inline ns){/}"},

      [dt_class]           = {"{#bright_magenta,bold}{}{/}", "{#magenta}::{/}", "{#bright_magenta,italic}(anon class){/}"},
      [dt_struct]          = {"{#bright_magenta,bold}{}{/}", "{#magenta}::{/}", "{#bright_magenta,italic}(anon struct){/}"},
      [dt_union]           = {"{#bright_orange,bold}{}{/}", "{#orange}::{/}", "{#bright_orange,italic}(anon union){/}"},

      [dt_enum_type]       = {"{#bright_blue}{}{/}", "{#blue}::{/}", "{#bright_blue,italic}(anon enum){/}"},
      [dt_enum_class_type] = {"{#bright_blue,bold}{}{/}", "{#blue}::{/}", "{#bright_blue,italic}(anon enum class){/}"},

      [dt_base_type]       = {"{#bold}{}{/}", nullptr, "{#error}(anon base type (error)){/}"},
      [dt_typedef]         = {"{#bold,italic}{}{/}", "{#light_gray}::{/}", "{#bold,italic}(anon typedef){/}"},
    };

    J_A(ND) inline constexpr const char * wrapped_formats[dt_max_wrapped - dt_min_wrapped + 1]{
      "{#bright_orange}*{/}", // dt_ptr_type,
      "{#orange}&{/}", // dt_ref_type,
      "{#orange}&&{/}", // dt_rref_type,
      "{#light_gray} const{/}", // dt_const_type,
      "{#red} restrict{/}", // dt_restrict_type,
    };

    J_A(NODISC) bool is_in_scope(const def_child_base * scope, const def_child_base * J_AA(NN) cur) noexcept {
      for (; scope; scope = scope->parent) {
        if (scope == cur) {
          return true;
        }
      }
      return false;
    }

    void maybe_dump_parent(const def_child_base *scope, const def_child_base * J_AA(NN) self) noexcept {
      J_ASSERT(self->type <= dt_max_named);
      if (self->parent && !is_in_scope(scope, self->parent)) {
        self->parent->dump_name(scope);
        J_DEBUG_INL(name_formats[self->type].sep);
      }
    }

    void dump_simple_name(def_type type, const char *name) noexcept {
      J_ASSERT(type <= dt_max_named);
      const name_fmt & fmt = name_formats[type];
      if (name) {
        J_ASSERT(*name);
        J_DEBUG_INL(fmt.fmt, name);
      } else {
        J_DEBUG_INL(fmt.anon);
      }
    }

    void dump_ref(dwarf_ref target, const def_child_base * scope, bool is_type) noexcept {
      if (target.ptr) {
        target.ptr->dump_name(scope);
      } else if (!target.ref) {
        J_DEBUG_INL(is_type ? "{#bold}void{/}" : "{#error}(empty (error)){/}");
      } else {
        J_DEBUG_INL("{#error}(unresolved){/}");
      }
    }

    void dump_param_list(span<const fn_param> params, const def_child_base * scope) noexcept {
      J_DEBUG_INL("{#bold}({/}");
      bool is_first = true;
      for (auto & p : params) {
        if (!is_first) {
          J_DEBUG_INL("{#bold}, {/}");
        }
        is_first = false;

        if (p.is_synthetic) {
          J_DEBUG_INL("{#light_gray}[{/}");
        }
        dump_ref(p.type, scope, true);
        if (p.is_synthetic) {
          J_DEBUG_INL("{#light_gray}]{/}");
        }
      }
      J_DEBUG_INL("{#bold}){/}");
    }

    J_A(NODISC,AI) inline u32_t hash_ref(const dwarf_ref & ref, u32_t base) noexcept {
      return crc32(base, (iptr_t)ref.ptr - (iptr_t)ref.ref);
    }

    J_A(NODISC) u32_t hash_sig(const fn_signature & sig) noexcept {
      u32_t hash = hash_ref(sig.result, 11);
      for (auto & p : sig.params) {
        hash = hash_ref(p.type, hash + p.is_synthetic);
      }
      return hash;
    }

    void resolve_sig(dwarf_context * J_AA(NN) ctx, fn_signature & sig) noexcept {
      resolve_ref(ctx, sig.result);
      for (auto & p : sig.params) {
        resolve_ref(ctx, p.type);
      }
    }

    J_A(NODISC,AI) inline u32_t hash_name(const char *name, u32_t base) noexcept {
      if (!name) {
        name = "abcd0123";
      }
      return crc32(base, name, j::strlen(name));
    }

    J_A(NODISC) inline def_child_base * get_class_type(const fn_signature & sig, bool & is_const) noexcept {
      if (!sig.params || !sig.params[0].is_synthetic) {
        return nullptr;
      }
      def_base * base = sig.params[0].type.ptr;
      bool c = false;
      while (base && base->type >= dt_min_wrapped && base->type <= dt_max_wrapped) {
        if (base->type == dt_const_type) {
          c = true;
        }
        base = static_cast<def_wrapped_type*>(base)->target.ptr;
      }
      if (!base || base->type < dt_class || base->type > dt_union) {
        return nullptr;
      }
      is_const = c;
      return static_cast<def_child_base*>(base);
    }
  }

  def_base::~def_base() { }
  void def_base::resolve(dwarf_context * J_AA(NN)) noexcept { }
  bool def_base::eq(const def_base &) const noexcept { return true; }
  u32_t def_base::hash() const noexcept { return type; }


  void def_named_base::dump_name(const def_child_base *) const noexcept {
    dump_simple_name(type, name);
  }
  bool def_named_base::eq(const def_base & rhs) const noexcept {
    const def_named_base & rhs_ = (const def_named_base &)rhs;
    return (name == rhs_.name || (name && rhs_.name && !j::strcmp(name, rhs_.name)));
  }
  u32_t def_named_base::hash() const noexcept {
    return hash_name(name, type);
  }


  void def_child_base::dump_name(const def_child_base * scope) const noexcept {
    if (scope == this) {
      J_DEBUG_INL("{#light_gray,italic}self{/}");
      return;
    }
    maybe_dump_parent(scope, this);
    dump_simple_name(type, name);
  }
  bool def_child_base::eq(const def_base & rhs) const noexcept {
    const def_child_base & rhs_ = (const def_child_base &)rhs;
    return rhs_.parent == parent && (name == rhs_.name || (name && rhs_.name && !j::strcmp(name, rhs_.name)));
  }
  u32_t def_child_base::hash() const noexcept {
    return hash_name(name, (type << 24) ^ ((uptr_t)parent >> 3));
  }
  void def_child_base::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_parent(ctx, parent);
  }


  u32_t def_wrapped_type::hash() const noexcept {
    return hash_ref(target, type);
  }
  void def_wrapped_type::dump_name(const def_child_base * scope) const noexcept {
    dump_ref(target, scope, true);
    J_ASSERT(type >= dt_min_wrapped && type <= dt_max_wrapped);
    J_DEBUG_INL(wrapped_formats[type - dt_min_wrapped]);
  }
  bool def_wrapped_type::eq(const def_base & rhs) const noexcept {
    return target == ((const def_wrapped_type &)rhs).target;
  }
  void def_wrapped_type::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_ref(ctx, target);
  }


  void def_array_type::dump_name(const def_child_base * scope) const noexcept {
    dump_ref(target, scope, true);
    if (size == -1) {
      J_DEBUG_INL("{#bold}[]{/}");
    } else {
      J_DEBUG_INL("{#bold,white}[{}]{/}", size);
    }
  }
  bool def_array_type::eq(const def_base & rhs) const noexcept {
    const def_array_type & rhs_ = (const def_array_type &)rhs;
    return rhs_.target == target && rhs_.size == size;
  }
  u32_t def_array_type::hash() const noexcept {
    return hash_ref(target, size + 37);
  }
  void def_array_type::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_ref(ctx, target);
  }



  void def_mem_ptr_type::dump_name(const def_child_base * scope) const noexcept {
    dump_ref(target, scope ? scope : ((def_child_base*)class_type.ptr), true);
    J_DEBUG_INL(" ");
    dump_ref(class_type, scope, true);
    J_DEBUG_INL("{#orange}::{#bold}*{/}{/}");
  }
  bool def_mem_ptr_type::eq(const def_base & rhs) const noexcept {
    const def_mem_ptr_type & rhs_ = (const def_mem_ptr_type &)rhs;
    return rhs_.target == target && rhs_.class_type == class_type;
  }
  u32_t def_mem_ptr_type::hash() const noexcept {
    return hash_ref(class_type, hash_ref(target, 123));
  }
  void def_mem_ptr_type::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_ref(ctx, target);
    resolve_ref(ctx, class_type);
  }

  bool def_typedef::eq(const def_base & rhs) const noexcept {
    const def_typedef & rhs_ = (const def_typedef &)rhs;
    return rhs_.parent == parent && target == rhs_.target && (name == rhs_.name || (name && rhs_.name && !j::strcmp(name, rhs_.name)));
  }
  u32_t def_typedef::hash() const noexcept {
    return hash_name(name, hash_ref(target, 11 | ((uptr_t)parent >> 3)));
  }
  void def_typedef::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_parent(ctx, parent);
    resolve_ref(ctx, target);
  }


  void def_fn_type::dump_name(const def_child_base * scope) const noexcept {
    span<const fn_param> normal_params(sig.params);
    dump_ref(sig.result, scope, true);
    J_DEBUG_INL(" ");
    dump_param_list(normal_params, scope);
  }

  bool def_fn_type::eq(const def_base & rhs) const noexcept {
    const def_fn_type & rhs_ = (const def_fn_type &)rhs;
    return sig == rhs_.sig;
  }

  u32_t def_fn_type::hash() const noexcept {
    return hash_sig(sig);
  }

  void def_fn_type::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_sig(ctx, sig);
  }

  void def_fn_decl::dump_name(const def_child_base * scope) const noexcept {
    span<const fn_param> normal_params(sig.params);
    bool is_const = false;
    def_child_base *class_type = get_class_type(sig, is_const);
    const def_child_base * param_scope = scope ? scope : class_type ? class_type : parent;
    dump_ref(sig.result, param_scope, true);
    J_DEBUG_INL(" ");
    if (class_type || parent) {
      if (class_type) {
        normal_params.remove_prefix(1);
      }
      (class_type ? class_type : parent)->dump_name(scope);
      J_DEBUG_INL("{#bright_yellow}::{/}");
    }
    if (name) {
      J_DEBUG_INL("{#bold,bright_yellow}{}{/}", name);
    }
    dump_param_list(normal_params, param_scope);
    if (is_const) {
      J_DEBUG_INL("{#light_gray} const{/}");
    }
  }
  bool def_fn_decl::eq(const def_base & rhs) const noexcept {
    const def_fn_decl & rhs_ = (const def_fn_decl &)rhs;
    return rhs_.parent == parent && (name == rhs_.name || (name && rhs_.name && !j::strcmp(name, rhs_.name)))
      && sig == rhs_.sig;
  }

  u32_t def_fn_decl::hash() const noexcept {
    return hash_name(name, ((uptr_t)parent >> 3)) ^ hash_sig(sig);
  }

  void def_fn_decl::resolve(dwarf_context * J_AA(NN) ctx) noexcept {
    resolve_parent(ctx, parent);
    resolve_sig(ctx, sig);
  }

  J_A(NODISC) bool def_hash::operator()(def_base * J_AA(NN) lhs, def_base * J_AA(NN) rhs) const noexcept {
    // J_ASSERT(lhs != rhs || lhs->eq(*rhs));
    // J_ASSERT(lhs->hash() == rhs->hash() || !lhs->eq(*rhs));
    return lhs == rhs || (lhs->type == rhs->type && lhs->eq(*rhs));
  }
  J_A(NODISC) u32_t def_hash::operator()(def_base * J_AA(NN) lhs) const noexcept {
    return lhs->hash();
  }
  J_A(NODISC) bool def_hash::operator()(const def_base & lhs, const def_base & rhs) const noexcept {
    // J_ASSERT(&lhs != &rhs || lhs.eq(rhs));
    // J_ASSERT(lhs.hash() == rhs.hash() || !lhs.eq(rhs));
    return lhs.type == rhs.type && lhs.eq(rhs);
  }

  J_A(NODISC) u32_t def_hash::operator()(const def_base & lhs) const noexcept {
    return lhs.hash();
  }
}
