#include "lisp/env/static_ids.hpp"
#include "logging/global.hpp"
#include "lisp/values/static_objects.hpp"
#include "hzd/crc32.hpp"
#include "containers/hash_map.hpp"

namespace j::lisp::env {
  J_SPD_DEFINE_STATIC_PKG(keyword_static_ids, "keywords", KEYWORD)
  J_SPD_DEFINE_STATIC_PKG(global_static_ids, "global", GLOBAL)
  J_SPD_DEFINE_STATIC_PKG(packages_static_ids, "packages", PACKAGES)
  J_SPD_DEFINE_STATIC_PKG(const_fold_static_ids, "keywords", KEYWORD)

  namespace {
    namespace s = strings;

    constinit const packages::static_pkg_def * const static_pkg_ptrs[]{
      &keyword_static_ids::def,
      &global_static_ids::def,
      &packages_static_ids::def,
      &air::static_ids::def,
      &const_fold_static_ids::def,
    };
  }

  constinit const packages::static_pkg_defs static_pkgs(static_pkg_ptrs);

  J_A(NODISC) bool is_static_id(id id) noexcept {
    return static_pkgs.is_static_id(id);
  }

  J_A(NODISC) packages::static_id_name get_static_id_name(id id) noexcept {
    return static_pkgs.get_static_id_name(id);
  }

  J_A(NODISC) packages::static_id_name maybe_get_static_id_name(id id) noexcept {
    return static_pkgs.maybe_get_static_id_name(id);
  }

  namespace {
    struct J_TYPE_HIDDEN static_sym_key final {
      u8_t pkg;
      s::const_string_view name;

      J_A(AI,ND,NODISC) inline bool operator==(const static_sym_key &) const noexcept = default;
    };

    struct J_TYPE_HIDDEN static_sym_key_hash final {
      J_A(ND,FLATTEN) inline u32_t operator()(const static_sym_key & k) const noexcept {
        return crc32(1 + k.pkg, k.name.begin(), k.name.size());
      }
    };

    J_A(NODISC) inline i32_t get_static_pkg_index(s::const_string_view pkg) noexcept {
      switch (pkg.size()) {
      case 7: return pkg == "keyword" ? 0 : -1;
      case 6: return pkg == "global" ? 1 : -1;
      case 8: return pkg == "packages" ? 2 : -1;
      case 4: return pkg == "%air" ? 3 : -1;
      case 11: return pkg == ":const-fold" ? 4 : -1;
      default: return -1;
      }
    }

    hash_map<static_sym_key, id, static_sym_key_hash> * static_id_map = nullptr;
    J_A(NODISC) hash_map<static_sym_key, id, static_sym_key_hash> & get_static_id_map() noexcept {
      if (J_UNLIKELY(!static_id_map)) {
        static_id_map = ::new hash_map<static_sym_key, id, static_sym_key_hash>();
        for (i32_t i = 0; i < num_static_pkgs; ++i) {
          const packages::static_pkg_def * const pkg = static_pkg_ptrs[i];
          for (i32_t j = 0, max = pkg->num_static_names; j < max; ++j) {
            J_DEBUG("Reg static #{}:#{} name={}", id(i, j).raw, i, j, pkg->static_names_begin[j]->value());
            static_id_map->insert(static_sym_key{(u8_t)i, pkg->static_names_begin[j]->value()}, id(i, j));
          }
        }
      }
      return *static_id_map;
    }
  }

  J_A(NODISC) bool is_static_id(s::const_string_view pkg, s::const_string_view sym) noexcept {
    i32_t pkg_idx = get_static_pkg_index(pkg);
    if (pkg_idx < 0) {
      return false;
    }
    return get_static_id_map().contains(static_sym_key{(u8_t)pkg_idx, sym});
  }

  J_A(NODISC) id get_static_id(s::const_string_view pkg, s::const_string_view sym) noexcept {
    id *result = get_static_id_map().maybe_at(static_sym_key{(u8_t)get_static_pkg_index(pkg), sym});
    if (!result) {
      J_THROW("Static ID {}:{} was not resolved.", pkg, sym);
    }
    return *result;
  }

  J_A(NODISC) id maybe_get_static_id(s::const_string_view pkg, s::const_string_view sym) noexcept {
    i32_t pkg_idx = get_static_pkg_index(pkg);
    if (pkg_idx < 0) {
      return id(nullptr);
    }
    id * result = get_static_id_map().maybe_at(static_sym_key{(u8_t)pkg_idx, sym});
    return result ? *result : id(nullptr);
  }
}
