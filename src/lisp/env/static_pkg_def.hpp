#pragma once

#include "lisp/common/id.hpp"
#include "containers/span.hpp"

namespace j::lisp::inline values {
  struct lisp_str;
}

namespace j::lisp::packages {
  struct static_id_name final {
    const lisp_str * const pkg;
    const lisp_str * const sym;

    J_A(AI,NODISC,ND,HIDDEN) inline explicit operator bool() const noexcept {
      return pkg;
    }

    J_A(AI,NODISC,ND,HIDDEN) inline bool operator!() const noexcept {
      return !pkg;
    }
  };

  struct static_pkg_def final {
    const lisp_str * pkg_name;
    const lisp_str * const * static_names_begin;
    i32_t num_static_names;

    template<i32_t Sz>
    J_A(AI,ND,HIDDEN) inline constexpr static_pkg_def(const lisp_str * J_NOT_NULL pkg_name,
                                                      const lisp_str * const (& static_names)[Sz]) noexcept
      : pkg_name(pkg_name),
        static_names_begin(static_names),
        num_static_names(Sz)
    { }

    J_A(RNN,NODISC,AI,ND) inline const lisp_str * const * begin() const noexcept {
      return static_names_begin;
    }

    J_A(RNN,NODISC,AI,ND) inline const lisp_str * const * end() const noexcept {
      return static_names_begin + num_static_names;
    }

    J_A(NODISC,AI,ND) inline i32_t size() const noexcept {
      return num_static_names;
    }

    J_A(NODISC,AI,ND,RNN) inline const lisp_str * operator[](i32_t idx) const noexcept {
      return static_names_begin[idx];
    }
  };

  struct static_pkg_defs final {
    const static_pkg_def * const * static_pkgs_begin;
    i32_t num_static_pkgs;

    template<i32_t Sz>
    J_A(AI,ND,HIDDEN) inline constexpr static_pkg_defs(const static_pkg_def * const (& pkgs)[Sz]) noexcept
      : static_pkgs_begin(pkgs),
        num_static_pkgs(Sz)
    { }

    J_A(RNN,NODISC,AI,ND) inline const static_pkg_def * const * begin() const noexcept {
      return static_pkgs_begin;
    }

    J_A(RNN,NODISC,AI,ND) inline const static_pkg_def * const * end() const noexcept {
      return static_pkgs_begin + num_static_pkgs;
    }

    J_A(NODISC,AI,ND) inline i32_t size() const noexcept {
      return num_static_pkgs;
    }

    J_A(NODISC,AI,ND) inline const static_pkg_def & operator[](i32_t idx) const noexcept {
      return *static_pkgs_begin[idx];
    }

    J_A(NODISC,ND) inline bool is_static_id(id id) const noexcept {
      return !id.is_unresolved() && !id.is_gensym()
        && id.package_id() < num_static_pkgs
        && (i32_t)id.index() < static_pkgs_begin[id.package_id()]->num_static_names;
    }

    J_A(NODISC,ND) inline static_id_name maybe_get_static_id_name(id id) const noexcept {
      if (!is_static_id(id)) {
        return {nullptr, nullptr};
      }
      const static_pkg_def * pkg = static_pkgs_begin[id.package_id()];
      return { pkg->pkg_name, pkg->static_names_begin[id.index()] };
    }

    J_A(NODISC,ND) inline static_id_name get_static_id_name(id id) const noexcept {
      J_ASSERT(is_static_id(id));
      const static_pkg_def * pkg = static_pkgs_begin[id.package_id()];
      return { pkg->pkg_name, pkg->static_names_begin[id.index()] };
    }
  };
}

#define J_SPD_WITH_STATIC_IDS(SUFFIX, MACRO) J_STATIC_IDS_##SUFFIX(MACRO)
#define J_SPD_DECLARE_STATIC_ID(NAME, ...) constexpr inline id id_##NAME{package_idx, idx_##NAME};
#define J_SPD_DECLARE_STATIC_IDX(NAME, ...) idx_##NAME,
#define J_SPD_DECLARE_STATIC_PKG(NS, IDX, SUFFIX)      \
  namespace NS { \
    constexpr inline u8_t package_idx = IDX; \
    enum id_index : u8_t {                                    \
      J_SPD_WITH_STATIC_IDS(SUFFIX, J_SPD_DECLARE_STATIC_IDX)  \
      num_static_ids, \
    }; \
    J_SPD_WITH_STATIC_IDS(SUFFIX, J_SPD_DECLARE_STATIC_ID) \
    extern constinit const packages::static_pkg_def def; \
  }

#define J_SPD_DO_DEFINE_STATIC_NAME(ID, NAME, ...) constinit const static_lisp_str name_##ID(NAME);
#define J_SPD_DEFINE_STATIC_NAME(NAME, ...) J_SPD_DO_DEFINE_STATIC_NAME(NAME, __VA_OPT__(__VA_ARGS__,) #NAME)
#define J_SPD_DEFINE_STATIC_NAME_REF(NAME, ...) &name_##NAME.value,
#define J_SPD_DEFINE_STATIC_PKG(NS, NAME, SUFFIX)                                \
  namespace NS {                                                                 \
    namespace {                                                                  \
      constinit const static_lisp_str pkg_name(NAME);                            \
      J_SPD_WITH_STATIC_IDS(SUFFIX, J_SPD_DEFINE_STATIC_NAME)                    \
      constinit const lisp_str * const static_names[] {                          \
        J_SPD_WITH_STATIC_IDS(SUFFIX, J_SPD_DEFINE_STATIC_NAME_REF)              \
      };                                                                         \
    }                                                                            \
    constinit const packages::static_pkg_def def(&pkg_name.value, static_names); \
  }
