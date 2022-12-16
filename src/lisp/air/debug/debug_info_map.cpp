#include "lisp/air/debug/debug_info_map.hpp"
#include "containers/hash_map.hpp"

J_DEFINE_EXTERN_HASH_MAP(const j::lisp::air::exprs::expr *, j::lisp::air::debug_info *);

namespace j::lisp::air::inline debug {

  void debug_info_map::add(const exprs::expr * J_NOT_NULL expr, debug_info * J_NOT_NULL info) {
    auto p = map.emplace(expr, info);
    if (!p.second) {
      info->next = p.first->second;
      p.first->second = info;
    }
  }

  [[nodiscard]] debug_info_view debug_info_map::get(const exprs::expr * J_NOT_NULL expr) const noexcept {
    const debug_info * const * ptr = map.maybe_at(expr);
    return {ptr ? *ptr : nullptr};
  }

  void debug_info_map::clear() noexcept {
    map.clear();
  }

  debug_info_map::~debug_info_map() {
    for (auto & p : map) {
      for (auto cur = p.second; cur;) {
        auto next = cur->next;
        ::delete cur;
        cur = next;
      }
    }
  }

  void debug_info_map::add(const exprs::expr * J_NOT_NULL expr, strings::string name, strings::string value, const char * bg_color) {
    add(expr, ::new debug_info{
        static_cast<strings::string &&>(name),
        static_cast<strings::string &&>(value),
        bg_color});
  }

  void debug_info_map::add_oneline(const exprs::expr * J_NOT_NULL expr, strings::string value, const char * bg_color) {
    add(expr, ::new debug_info{{}, static_cast<strings::string &&>(value), bg_color});
  }
}
