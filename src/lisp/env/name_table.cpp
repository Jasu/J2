#include "name_table.hpp"

#include "containers/vector.hpp"
#include "lisp/values/lisp_imms.hpp"
#include "containers/hash_map.hpp"
#include "lisp/mem/heap.hpp"

J_DEFINE_EXTERN_NONCOPYABLE_VECTOR(const j::lisp::lisp_str*);
J_DEFINE_EXTERN_HASH_MAP(const j::lisp::lisp_str*, j::lisp::id,
                         HASH(j::lisp::lisp_str_ptr_hash),
                         KEYEQ(j::lisp::lisp_str_ptr_equal_to),
                         CONST_KEY(j::lisp::lisp_str_ptr_const_key));

namespace j::lisp::env {
  name_table::name_table(u8_t package_id) noexcept
    : package_id(package_id)
  {
  }

  id name_table::emplace(const lisp_str * J_NOT_NULL name) noexcept {
    id result{package_id, names.size()};
    auto pair = name_to_id.emplace(name, result);
    if (!pair.second) {
      return pair.first->second;
    }
    names.emplace_back(name);
    return result;
  }

  id name_table::emplace(const lisp_str_ref & name) noexcept {
    return emplace(name.value());
  }

  id name_table::emplace(const strings::const_string_view & str,
                         lisp::mem::heap * J_NOT_NULL heap) noexcept {
    if (auto it = name_to_id.maybe_at(str)) {
      return *it;
    }
    const lisp_str * lstr = lisp_str::allocate(*heap, str, (lisp::mem::object_hightag_flag)0, 1U);
    id result{package_id, names.size()};
    name_to_id.emplace(lstr, result);
    names.emplace_back(lstr);
    return result;
  }

  [[nodiscard]] id name_table::emplace_unresolved(
    const strings::const_string_view & package_id,
    const strings::const_string_view & symbol_id,
    lisp::mem::heap * J_NOT_NULL heap
  ) {
    const id pkg_id = emplace(package_id, heap);
    const id sym_id = emplace(symbol_id, heap);
    const u32_t unresolved_value = pkg_id.index() << 16 | sym_id.index();
    u32_t i = 0U;
    for (; i < unresolved.size(); ++i) {
      if (unresolved[i] == unresolved_value) {
        return id(unresolved_v, this->package_id, i);
      }
    }
    unresolved.push_back(unresolved_value);
    return id(unresolved_v, this->package_id, i);
  }

  unresolved_ids name_table::get_unresolved_ids(id unresolved) const noexcept {
    J_ASSERT(unresolved.is_unresolved() && unresolved.index() < this->unresolved.size());
    u32_t raw = this->unresolved[unresolved.index()];
    return { id(this->package_id, raw >> 16), id(this->package_id, raw & 0xFFFFU) };
  }

  unresolved_names name_table::get_unresolved_names(id unresolved) const noexcept {
    J_ASSERT(unresolved.is_unresolved() && unresolved.index() < this->unresolved.size());
    u32_t raw = this->unresolved[unresolved.index()];
    return { names[raw >> 16], names[raw & 0xFFFFU] };
  }

  const lisp_str * name_table::name_of(id id) const {
    J_ASSERT(id.package_id() == package_id);
    return id.is_gensym() ? nullptr : names.at(id.index());
  }

  id name_table::id_of(const strings::const_string_view & str) const {
    return name_to_id.at(str);
  }

  id name_table::try_get_id_of(const strings::const_string_view & str) const noexcept {
    const id * result = name_to_id.maybe_at(str);
    return result ? *result : id::none;
  }

  u32_t name_table::size() const noexcept {
    return name_to_id.size();
  }
}
