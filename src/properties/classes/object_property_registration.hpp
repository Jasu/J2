#pragma once

#include <typeinfo>

#include "exceptions/assert_lite.hpp"
#include "hzd/type_traits.hpp"
#include "properties/classes/object_property_definition.hpp"

namespace j::properties::classes {
  inline object_property_getter::object_property_getter(get_object_property_t getter)
    : getter(getter)
  {
    J_ASSERT_NOT_NULL(getter);
  }

  class property_base {
  protected:
    const char * m_name;
    attributes::attribute_map m_metadata;

  public:
    J_ALWAYS_INLINE property_base() noexcept = default;

    template<typename... Args>
    J_ALWAYS_INLINE explicit property_base(const char * J_NOT_NULL name) noexcept : m_name(name)
    {
    }

    template<typename... Args, sz_t = sizeof...(Args) - 1>
    J_ALWAYS_INLINE explicit property_base(const char * name, Args && ... args)
      : m_name(name),
        m_metadata(static_cast<Args &&>(args)...)
    {
      J_ASSERT_NOT_NULL(name);
    }

    J_INLINE_GETTER_NONNULL constexpr const char * name() const noexcept {
      return m_name;
    }
  };

  template<auto Pointer>
  class J_TYPE_HIDDEN member;

  template<typename PropClass, typename Member, Member (PropClass::*Pointer)>
  class J_TYPE_HIDDEN member<Pointer> final : public property_base {
    template<typename Class>
    J_INLINE_GETTER_NONNULL static void * get_property(void * object) {
      return &(static_cast<PropClass*>(reinterpret_cast<Class*>(object))->*Pointer);
    }

  public:
    using property_base::property_base;

    template<typename Class>
    J_HIDDEN J_ALWAYS_INLINE object_property_definition to_definition() {
      return object_property_definition(m_name, &get_property<Class>, typeid(Member), static_cast<attributes::attribute_map &&>(m_metadata));
    }
  };

  template<auto Pointer>
  class J_TYPE_HIDDEN accessor;

  template<typename PropClass, typename Member, Member & (PropClass::*Pointer)()>
  class J_TYPE_HIDDEN accessor<Pointer> final : public property_base {
    template<typename Class>
    J_INLINE_GETTER_NONNULL static void * get_property(void * object) {
      return &((static_cast<PropClass*>(reinterpret_cast<Class*>(object))->*Pointer)());
    }

  public:
    using property_base::property_base;

    template<typename Class>
    J_INLINE_GETTER object_property_definition to_definition() {
      return object_property_definition(m_name, &get_property<Class>, typeid(Member), static_cast<attributes::attribute_map &&>(m_metadata));
    }
  };

  namespace detail {
    template<typename Fn>
    struct lambda_traits;

    template<typename ReturnType, typename Lambda, typename FirstArg>
    struct lambda_traits<ReturnType (Lambda::*)(FirstArg) const> {
      using return_type J_NO_DEBUG_TYPE = j::remove_cref_t<ReturnType>;
      using ptr_t J_NO_DEBUG_TYPE = ReturnType (*)(FirstArg);
    };
  }

  template<typename PtrT, typename Property, typename UniqueT>
  class J_TYPE_HIDDEN by_lambda final : public property_base {
  public:
    inline static PtrT m_ptr;

    template<typename Class>
    J_ALWAYS_INLINE static void * call(void * v) {
      J_ASSERT_NOT_NULL(v);
      return &m_ptr(*reinterpret_cast<Class*>(v));
    }

    template<typename Class>
    J_INLINE_GETTER object_property_definition to_definition() {
      return {
        m_name,
        &call<Class>,
        typeid(Property),
        static_cast<attributes::attribute_map &&>(m_metadata)};
    }

    template<typename... Args>
    J_ALWAYS_INLINE explicit by_lambda(const char * name, PtrT ptr, Args && ... args)
      : property_base(name, static_cast<Args &&>(args)...)
    {
      J_ASSERT_NOT_NULL(name, ptr);
      m_ptr = ptr;
    }
  };

  template<typename Lambda,
           typename LambdaTraits = detail::lambda_traits<decltype(&Lambda::operator())>,
           typename... Args>
  by_lambda(const char *, Lambda lambda, Args && ...) -> by_lambda<
    typename LambdaTraits::ptr_t,
    typename LambdaTraits::return_type,
    decltype(&Lambda::operator())>;

  inline void object_property_definition::set_definition(const wrappers::wrapper_definition * definition) {
    J_ASSERT(!m_definition, "object_property_definition::set_definition was called twice.");
    J_ASSERT_NOT_NULL(definition);
    m_definition = definition;
  }
}
