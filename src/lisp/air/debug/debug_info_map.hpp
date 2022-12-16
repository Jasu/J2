#pragma once

#include "strings/concepts.hpp"
#include "containers/hash_map_fwd.hpp"
#include "strings/string.hpp"
#include "strings/format.hpp"
#include "hzd/iterators.hpp"

namespace j::lisp::air::exprs {
  struct expr;
}

namespace j::lisp::air::inline debug {
  struct debug_info final {
    strings::string name;
    strings::string value;
    const char * bg_color = nullptr;
    debug_info * next = nullptr;
  };
}

J_DECLARE_EXTERN_HASH_MAP(const j::lisp::air::exprs::expr *, j::lisp::air::debug_info *);

namespace j::lisp::air::inline debug {
  struct debug_info_view final {
    const debug_info * first = nullptr;

    J_INLINE_GETTER bool empty() const noexcept {
      return !first;
    }

    J_INLINE_GETTER bool operator!() const noexcept {
      return !first;
    }

    J_INLINE_GETTER explicit operator bool() const noexcept {
      return first;
    }

    J_INLINE_GETTER linked_list_iterator<const debug_info> begin() const noexcept {
      return {first};
    }

    J_INLINE_GETTER linked_list_iterator<const debug_info> end() const noexcept {
      return {nullptr};
    }
  };

  struct debug_info_map final {
    J_BOILERPLATE(debug_info_map, CTOR_CE, COPY_DEL)

    hash_map<const exprs::expr*, debug_info*> map;

    ~debug_info_map();

    void add(const exprs::expr * J_NOT_NULL expr, debug_info * J_NOT_NULL info);

    [[nodiscard]] debug_info_view get(const exprs::expr * J_NOT_NULL expr) const noexcept;

    void clear() noexcept;

    template<strings::IsArgNotAnyString Name, strings::IsArgNotAnyString Value>
    J_A(AI,HIDDEN) inline void add(const exprs::expr * J_NOT_NULL expr, const Name & name, const Value & value, const char * bg_color = nullptr) {
      add(expr, strings::format("{}", name), strings::format("{}", value), bg_color);
    }

    template<strings::IsArgNotAnyString Value>
    J_A(AI,HIDDEN) inline void add(const exprs::expr * J_NOT_NULL expr, strings::string name, const Value & value, const char * bg_color = nullptr) {
      add(expr, static_cast<strings::string &&>(name), strings::format("{}", value), bg_color);
    }

    template<strings::IsArgNotAnyString Name>
    J_A(AI,HIDDEN) inline void add(const exprs::expr * J_NOT_NULL expr, const Name & name, strings::string value, const char * bg_color = nullptr) {
      add(expr, strings::format("{}", name), static_cast<strings::string &&>(value), bg_color);
    }

    void add(const exprs::expr * J_NOT_NULL expr, strings::string name, strings::string value, const char * bg_color = nullptr);

    template<strings::IsArgNotAnyString Value>
    J_A(AI,HIDDEN) inline void add_oneline(const exprs::expr * J_NOT_NULL expr, const Value & value, const char * bg_color = nullptr) {
      add_oneline(expr, strings::format("{}", value), bg_color);
    }

    void add_oneline(const exprs::expr * J_NOT_NULL expr, strings::string value, const char * bg_color = nullptr);
  };
}
