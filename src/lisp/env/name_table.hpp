#pragma once

#include "containers/vector.hpp"
#include "containers/hash_map_fwd.hpp"
#include "lisp/values/lisp_str_hash.hpp"
#include "lisp/common/id.hpp"

J_DECLARE_EXTERN_NONCOPYABLE_VECTOR(const j::lisp::lisp_str*);

J_DECLARE_EXTERN_HASH_MAP(const j::lisp::lisp_str*, j::lisp::id,
                          HASH(j::lisp::lisp_str_ptr_hash),
                          KEYEQ(j::lisp::lisp_str_ptr_equal_to),
                          CONST_KEY(j::lisp::lisp_str_ptr_const_key));

namespace j::lisp::inline values {
  struct lisp_str_ref;
}

namespace j::lisp::mem {
  class heap;
}
namespace j::lisp::env {
  struct unresolved_ids final {
    id pkg_id = id::none;
    id sym_id = id::none;
  };

  struct unresolved_names final {
    const lisp_str * pkg_name = nullptr;
    const lisp_str * sym_name = nullptr;
  };

  struct name_table {
    explicit name_table(u8_t package_id) noexcept;

    id emplace(const lisp_str * J_NOT_NULL name) noexcept;
    id emplace(const lisp_str_ref & name) noexcept;

    id emplace(const strings::const_string_view & str, lisp::mem::heap * J_NOT_NULL heap) noexcept;

    id emplace_unresolved(
      const strings::const_string_view & package_id,
      const strings::const_string_view & symbol_id,
      lisp::mem::heap * J_NOT_NULL heap
    );

    [[nodiscard]] unresolved_ids get_unresolved_ids(id unresolved) const noexcept;
    [[nodiscard]] unresolved_names get_unresolved_names(id unresolved) const noexcept;

    [[nodiscard]] id id_of(const strings::const_string_view & str) const;
    [[nodiscard]] id try_get_id_of(const strings::const_string_view & str) const noexcept;

    [[nodiscard]] const lisp_str * name_of(id id) const;

    [[nodiscard]] u32_t size() const noexcept;

    u8_t package_id;
    vector<u32_t> unresolved;

    noncopyable_vector<const lisp_str*> names;

    hash_map<const lisp_str*, id, lisp_str_ptr_hash, lisp_str_ptr_equal_to, lisp_str_ptr_const_key> name_to_id;
  };
}
