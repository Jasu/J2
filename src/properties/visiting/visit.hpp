#pragma once

#include "properties/access.hpp"
#include "properties/visiting/detail.hpp"

namespace j::properties::visiting {
  /// Non-recursively visit [access] with [visitor].
  ///
  /// [tail_args] are appended to the argument list of visit.
  template<typename Access, typename Visitor, typename... TailArgs>
  decltype(auto) visit(Access && access, Visitor && visitor, TailArgs && ... tail_args) {
    switch (access.type()) {
    case PROPERTY_TYPE::EMPTY:
      detail::throw_empty_visit();
    case PROPERTY_TYPE::NIL:
      return static_cast<Visitor &&>(visitor)(
        static_cast<nil_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::STRING:
      return static_cast<Visitor &&>(visitor)(
        static_cast<string_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::INT:
      return static_cast<Visitor &&>(visitor)(
        static_cast<int_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::BOOL:
      return static_cast<Visitor &&>(visitor)(
        static_cast<bool_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::FLOAT:
      return static_cast<Visitor &&>(visitor)(
        static_cast<float_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::LIST:
      return static_cast<Visitor &&>(visitor)(
        static_cast<list_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::MAP:
      return static_cast<Visitor &&>(visitor)(
        static_cast<map_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::SET:
      return static_cast<Visitor &&>(visitor)(
        static_cast<set_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::OBJECT:
      return static_cast<Visitor &&>(visitor)(
        static_cast<object_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    case PROPERTY_TYPE::ENUM:
      return static_cast<Visitor &&>(visitor)(
        static_cast<enum_access &>(access), static_cast<TailArgs &&>(tail_args)...);
    }
  }
}
