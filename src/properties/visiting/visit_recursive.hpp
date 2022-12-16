#pragma once

#include "properties/access.hpp"
#include "properties/visiting/visit_path.hpp"
#include "properties/visiting/detail.hpp"

namespace j::properties::visiting {
  namespace detail {
    template<typename Visitor, typename... TailArgs>
    static void visit_recursive_impl(typed_access access, Visitor && visitor, const visit_path & path, TailArgs && ... tail_args) {
      switch (access.type()) {
      case PROPERTY_TYPE::EMPTY:
        detail::throw_empty_visit();
      case PROPERTY_TYPE::NIL:
        static_cast<Visitor &&>(visitor)(static_cast<nil_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::STRING:
        static_cast<Visitor &&>(visitor)(
          static_cast<string_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::INT:
        static_cast<Visitor &&>(visitor)(
          static_cast<int_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::BOOL:
        static_cast<Visitor &&>(visitor)(
          static_cast<bool_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::FLOAT:
        static_cast<Visitor &&>(visitor)(
          static_cast<float_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::LIST: {
        auto & list = static_cast<list_access &>(access);
        static_cast<Visitor &&>(visitor)(list, path, static_cast<TailArgs &&>(tail_args)...);
        visit_path child_path(COMPONENT_TYPE::ARRAY_INDEX, path);
        const sz_t sz = list.size();
        for (sz_t i = 0; i < sz; ++i) {
          child_path.set_array_index(i);
          visit_recursive_impl(list[i].get_value(), static_cast<Visitor &&>(visitor), child_path, static_cast<TailArgs &&>(tail_args)...);
        }
        break;
      }
      case PROPERTY_TYPE::MAP: {
        auto & map = static_cast<map_access &>(access);
        static_cast<Visitor &&>(visitor)(map, path, static_cast<TailArgs &&>(tail_args)...);
        visit_path child_path(COMPONENT_TYPE::MAP_KEY, path);
        for (auto & p : map) {
          child_path.set_string(p.key().as_string().as_j_string_view());
          visit_recursive_impl(p.value().get_value(), static_cast<Visitor &&>(visitor), child_path, static_cast<TailArgs &&>(tail_args)...);
        }
        break;
      }
      case PROPERTY_TYPE::SET:
        static_cast<Visitor &&>(visitor)(
          static_cast<set_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      case PROPERTY_TYPE::OBJECT: {
        auto & object = static_cast<object_access &>(access);
        static_cast<Visitor &&>(visitor)(object, path, static_cast<TailArgs &&>(tail_args)...);
        visit_path child_path(COMPONENT_TYPE::PROPERTY_NAME, path);
        for (auto & p : object) {
          child_path.set_string(strings::const_string_view(p.name()));
          visit_recursive_impl(p.value().get_value(), static_cast<Visitor &&>(visitor), child_path, static_cast<TailArgs &&>(tail_args)...);
        }
        break;
      }
      case PROPERTY_TYPE::ENUM:
        static_cast<Visitor &&>(visitor)(
          static_cast<enum_access &>(access), path, static_cast<TailArgs &&>(tail_args)...);
        break;
      }
    }
  }

  template<typename Visitor, typename... TailArgs>
  J_ALWAYS_INLINE_NO_DEBUG void visit_recursive(typed_access & access, Visitor && visitor, TailArgs && ... tail_args) {
    detail::visit_recursive_impl(access, static_cast<Visitor &&>(visitor), visit_path::root, static_cast<TailArgs &&>(tail_args)...);
  }

  template<typename Visitor, typename... TailArgs>
  J_ALWAYS_INLINE_NO_DEBUG void visit_recursive(wrappers::wrapper & wrapper, Visitor && visitor, TailArgs && ... tail_args) {
    detail::visit_recursive_impl(wrapper.get_value(), static_cast<Visitor &&>(visitor), visit_path::root, static_cast<TailArgs &&>(tail_args)...);
  }
}
