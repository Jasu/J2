#pragma once

#include "lisp/values/imm_type.hpp"
#include "lisp/common/id.hpp"
#include "containers/span.hpp"

namespace j::lisp::detail {
  template<typename... Args>
  using lisp_fn_ptr_helper_t J_NO_DEBUG_TYPE = u64_t (*) (Args...);
}

namespace j::lisp::inline values {
  struct lisp_i64;
  struct lisp_f32;

  struct lisp_bool;
  struct lisp_nil;

  struct lisp_closure_ref;
  struct lisp_fn_ref;

  struct lisp_str_ref;
  struct lisp_sym_id;
  struct lisp_vec_ref;
  struct lisp_rec_ref;

  struct lisp_str;
  struct lisp_vec;

  struct lisp_imm {
    J_A(AI,ND,HIDDEN) inline lisp_imm() noexcept
      : raw(0)
    { }

    J_A(AI,ND,HIDDEN) inline explicit lisp_imm(u64_t raw) noexcept
      : raw(raw)
    { }

    J_A(AI,ND,HIDDEN) inline explicit lisp_imm(id i) noexcept
      : raw(i.raw)
    { }

    template<typename T = void>
    J_A(AI,ND) inline explicit lisp_imm(first_type_t<void, T> * J_NOT_NULL raw) noexcept
      : raw_ptr(raw)
    { }

    [[nodiscard]] constexpr imm_tag tag() const noexcept
    { return tag_of(raw); }
    [[nodiscard]] constexpr imm_type type() const noexcept
    { return imm_type_by_tag(tag_of(raw)); }

    /// Returns whether the imm contains an integer.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_i64()         const noexcept { return values::is_i64(raw); }
    /// Returns whether the imm contains a float.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_f32()         const noexcept { return values::is_f32(raw); }

    /// Returns whether the imm refers to a boolean.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_bool()        const noexcept { return values::is_bool(raw); }
    /// Returns whether the imm refers to boolean true.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_true()        const noexcept { return raw == true_v; }
    /// Returns whether the imm refers to boolean true.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_false()       const noexcept { return raw == false_v; }

    /// Returns whether the imm contains nil.
    J_A(AI,ND,NODISC,HIDDEN) inline bool is_nil()         const noexcept { return raw == nil_v; }

    /// Returns whether the imm refers to a non-lambda function or macro.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_fn_ref()    const noexcept { return values::is_fn_ref(raw); }
    /// Returns whether the imm refers to a lambda function.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_closure_ref()    const noexcept { return values::is_closure_ref(raw); }

    /// Returns whether the imm refers to a symbol.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_sym_id()  const noexcept { return values::is_sym_id(raw); }


    /// Returns whether the imm refers to a string.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_str_ref()  const noexcept { return values::is_str_ref(raw); }
    // Returns whether the imm refers to a vector.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_vec_ref()     const noexcept { return values::is_vec_ref(raw); }
    // Returns whether the imm refers to any other record.
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_rec_ref()     const noexcept { return values::is_rec_ref(raw); }
    J_A(AI,ND,NODISC,HIDDEN,FLATTEN) inline bool is_undefined() const noexcept { return values::is_undefined(raw); }

    [[nodiscard]] inline lisp_i64 & as_i64() noexcept {
      J_ASSERT(values::is_i64(raw));
      return *reinterpret_cast<lisp_i64*>(this);
    }
    [[nodiscard]] inline lisp_f32 & as_f32() noexcept {
      J_ASSERT(values::is_f32(raw));
      return *reinterpret_cast<lisp_f32*>(this);
    }

    [[nodiscard]] inline lisp_bool & as_bool() noexcept {
      J_ASSERT(values::is_bool(raw));
      return *reinterpret_cast<lisp_bool*>(this);
    }

    [[nodiscard]] inline lisp_nil & as_nil() noexcept {
      J_ASSERT(values::is_nil(raw));
      return *reinterpret_cast<lisp_nil*>(this);
    }

    [[nodiscard]] inline lisp_fn_ref & as_fn_ref() noexcept {
      J_ASSERT(values::is_fn_ref(raw));
      return *reinterpret_cast<lisp_fn_ref*>(this);
    }
    [[nodiscard]] inline lisp_closure_ref & as_closure_ref() noexcept {
      J_ASSERT(values::is_closure_ref(raw));
      return *reinterpret_cast<lisp_closure_ref*>(this);
    }

    [[nodiscard]] inline lisp_str_ref & as_str_ref() noexcept {
      J_ASSERT(values::is_str_ref(raw));
      return *reinterpret_cast<lisp_str_ref*>(this);
    }
    [[nodiscard]] inline lisp_sym_id & as_sym_id() noexcept {
      J_ASSERT(values::is_sym_id(raw));
      return *reinterpret_cast<lisp_sym_id*>(this);
    }
    [[nodiscard]] inline lisp_vec_ref & as_vec_ref() noexcept {
      J_ASSERT(values::is_vec_ref(raw));
      return *reinterpret_cast<lisp_vec_ref*>(this);
    }
    [[nodiscard]] inline lisp_rec_ref & as_rec_ref() noexcept {
      J_ASSERT(values::is_rec_ref(raw));
      return *reinterpret_cast<lisp_rec_ref*>(this);
    }

    J_A(AI,NODISC) inline const lisp_i64 & as_i64() const noexcept
    { return const_cast<lisp_imm*>(this)->as_i64(); }
    J_A(AI,NODISC) inline const lisp_f32 & as_f32() const noexcept
    { return const_cast<lisp_imm*>(this)->as_f32(); }

    J_A(AI,NODISC) inline const lisp_nil & as_nil() const noexcept
    { return const_cast<lisp_imm*>(this)->as_nil(); }
    J_A(AI,NODISC) inline const lisp_bool & as_bool() const noexcept
    { return const_cast<lisp_imm*>(this)->as_bool(); }

    J_A(AI,NODISC) inline const lisp_fn_ref & as_fn_ref() const noexcept
    { return const_cast<lisp_imm*>(this)->as_fn_ref(); }
    J_A(AI,NODISC) inline const lisp_closure_ref & as_closure_ref() const noexcept
    { return const_cast<lisp_imm*>(this)->as_closure_ref(); }

    J_A(AI,NODISC) inline const lisp_str_ref & as_str_ref() const noexcept
    { return const_cast<lisp_imm*>(this)->as_str_ref(); }
    J_A(AI,NODISC) inline const lisp_sym_id & as_sym_id() const noexcept
    { return const_cast<lisp_imm*>(this)->as_sym_id(); }
    J_A(AI,NODISC) inline const lisp_vec_ref & as_vec_ref() const noexcept
    { return const_cast<lisp_imm*>(this)->as_vec_ref(); }
    J_A(AI,NODISC) inline const lisp_rec_ref & as_rec_ref() const noexcept
    { return const_cast<lisp_imm*>(this)->as_rec_ref(); }

    J_A(AI,NODISC) inline bool eq(const lisp_imm & rhs) const noexcept
    { return raw == rhs.raw; }

    template<typename Fn, typename... Args>
    J_A(AI,ND) inline decltype(auto) visit(Fn && fn, Args && ... args)
    { return visit_imm(*this, static_cast<Fn &&>(fn), static_cast<Args &&>(args)...); }

    template<typename Fn, typename... Args>
    J_A(AI,ND) inline decltype(auto) visit(Fn && fn, Args && ... args) const
    { return visit_imm(*this, static_cast<Fn &&>(fn), static_cast<Args &&>(args)...); }

    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return raw != nil_v && raw != false_v;
    }

    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return raw == nil_v || raw == false_v;
    }

    union {
      u64_t raw = nil_v;
      void * raw_ptr;
    };

  private:
    /// Dummy, only for constructor homing.
    J_A(HIDDEN) explicit lisp_imm(null_t&) noexcept;
  };

  template<typename Imm, typename Fn, typename... Args>
  J_A(ND) decltype(auto) visit_imm(Imm & imm, Fn && fn, Args && ... args) {
    switch (imm.type()) {
    case imm_i64:
      return static_cast<Fn &&>(fn)(imm.as_i64(), static_cast<Args &&>(args)...);
    case imm_f32:
      return static_cast<Fn &&>(fn)(imm.as_f32(), static_cast<Args &&>(args)...);

    case imm_bool:
      return static_cast<Fn &&>(fn)(imm.as_bool(), static_cast<Args &&>(args)...);

    case imm_nil:
      return static_cast<Fn &&>(fn)(imm.as_nil(), static_cast<Args &&>(args)...);

    case imm_fn_ref:
      return static_cast<Fn &&>(fn)(imm.as_fn_ref(), static_cast<Args &&>(args)...);
    case imm_closure_ref:
      return static_cast<Fn &&>(fn)(imm.as_closure_ref(), static_cast<Args &&>(args)...);

    case imm_str_ref:
      return static_cast<Fn &&>(fn)(imm.as_str_ref(), static_cast<Args &&>(args)...);
    case imm_sym_id:
      return static_cast<Fn &&>(fn)(imm.as_sym_id(), static_cast<Args &&>(args)...);
    case imm_vec_ref:
      return static_cast<Fn &&>(fn)(imm.as_vec_ref(), static_cast<Args &&>(args)...);
    case imm_rec_ref:
      return static_cast<Fn &&>(fn)(imm.as_rec_ref(), static_cast<Args &&>(args)...);
    }
  }

  using lisp_fn_ptr_t = u64_t (*) (u64_t *, u32_t);


  template<u32_t N>
  using lisp_abi_call_ptr_t = repeat_t<detail::lisp_fn_ptr_helper_t, u64_t, N>;
  using lisp_abi_call_0_ptr_t = lisp_abi_call_ptr_t<0>;
  using lisp_abi_call_1_ptr_t = lisp_abi_call_ptr_t<1>;
  using lisp_abi_call_2_ptr_t = lisp_abi_call_ptr_t<2>;


  using lisp_closure_func_t = u64_t (u64_t *, u32_t, void *);
  using lisp_closure_func_ptr_t = lisp_closure_func_t *;

  using imms_t = span<lisp_imm>;
  using const_imms_t = span<const lisp_imm>;

  J_A(ND) inline const lisp_imm lisp_f{false_v};
  J_A(ND) inline const lisp_imm lisp_t{true_v};
}
