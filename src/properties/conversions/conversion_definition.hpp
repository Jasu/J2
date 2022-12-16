#pragma once

#include "properties/conversions/conversion.hpp"
#include "type_id/type_id.hpp"
#include "hzd/type_traits.hpp"

#include <typeinfo>

namespace j::properties::conversions {
  template<typename Fn>
  struct J_TYPE_HIDDEN fn_helper;

  template<typename Result, typename Arg>
  struct J_TYPE_HIDDEN fn_helper<Result (*)(Arg)> {
    using result_t J_NO_DEBUG_TYPE = j::remove_cref_t<Result>;
    using arg_t J_NO_DEBUG_TYPE = j::remove_ref_t<Arg>;
  };

  template<typename Result, typename Arg>
  struct J_TYPE_HIDDEN fn_helper<Result (*)(Arg) noexcept> {
    using result_t J_NO_DEBUG_TYPE = j::remove_cref_t<Result>;
    using arg_t J_NO_DEBUG_TYPE = j::remove_ref_t<Arg>;
  };

  template<typename Fn>
  struct J_TYPE_HIDDEN lambda_helper;

  template<typename Result, typename Class, typename Arg>
  struct J_TYPE_HIDDEN lambda_helper<Result (Class::*)(Arg) const> {
    using result_t J_NO_DEBUG_TYPE = j::remove_cref_t<Result>;
    using arg_t J_NO_DEBUG_TYPE = j::remove_ref_t<Arg>;
    using fn_t J_NO_DEBUG_TYPE = Result (*)(Arg);
  };

  template<typename Result, typename Class, typename Arg>
  struct J_TYPE_HIDDEN lambda_helper<Result (Class::*)(Arg) const noexcept> {
    using result_t J_NO_DEBUG_TYPE = j::remove_cref_t<Result>;
    using arg_t J_NO_DEBUG_TYPE = j::remove_ref_t<Arg>;
    using fn_t J_NO_DEBUG_TYPE = Result (*)(Arg);
  };

  template<typename From, typename To, typename FnPtr>
  J_HIDDEN void * create_from_impl(const void * from, void * fn) {
    decltype(auto) val{reinterpret_cast<FnPtr>(fn)(*reinterpret_cast<const From*>(from))};
    return ::new To(static_cast<decltype(val) &&>(val));
  }

  template<typename From, typename To, typename FnPtr>
  J_HIDDEN void create_from_in_place_impl(void * to, const void * from, void * fn) {
    ::new (to) To(reinterpret_cast<FnPtr>(fn)(*reinterpret_cast<const From*>(from)));
  }

  template<typename From, typename To, typename FnPtr>
  J_HIDDEN void assign_from_impl(void * to, const void * from, void * fn) {
    *reinterpret_cast<To*>(to) = reinterpret_cast<FnPtr>(fn)(*reinterpret_cast<const From*>(from));
  }

  class conversion_definition final {
  public:
    conversion conversion;
    type_id::type_id from;
    type_id::type_id to;

    template<
      typename ArgFn,
      typename FnHelper = lambda_helper<decltype(&ArgFn::operator())>,
      typename Fn = typename FnHelper::fn_t
    >
    J_HIDDEN J_ALWAYS_INLINE conversion_definition(ArgFn fn) noexcept
      : conversion{
          &create_from_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn>,
          &create_from_in_place_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn>,
          &assign_from_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn>,
          reinterpret_cast<void*>(static_cast<Fn>(fn))
        },
        from(typeid(typename FnHelper::arg_t)),
        to(typeid(typename FnHelper::result_t))
    {
    }

    template<
      typename Fn,
      typename FnHelper = fn_helper<Fn*>
    >
    J_HIDDEN J_ALWAYS_INLINE conversion_definition(Fn * fn) noexcept
      : conversion{
          &create_from_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn*>,
          &create_from_in_place_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn*>,
          &assign_from_impl<typename FnHelper::arg_t, typename FnHelper::result_t, Fn*>,
          reinterpret_cast<void*>(fn)
        },
        from(typeid(typename FnHelper::arg_t)),
        to(typeid(typename FnHelper::result_t))
    {
    }
  };
}
