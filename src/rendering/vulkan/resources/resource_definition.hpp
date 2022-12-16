#pragma once

#include "rendering/vulkan/resources/resource_ref.hpp"
#include "hzd/mem.hpp"

namespace j::rendering::vulkan::resources {
  namespace detail {
    template<typename T>
    constexpr inline u32_t maybe_sizeof_v = sizeof(T);
    template<>
    constexpr inline u32_t maybe_sizeof_v<void> = 0U;
  }

  template<typename Resource, typename Description, typename State>
  J_INLINE_GETTER_NONNULL J_HIDDEN resource_wrapper * default_create_resource_wrapper() {
    constexpr sz_t size = sizeof(Resource) > detail::maybe_sizeof_v<Description>
      ? sizeof(Resource)
      : detail::maybe_sizeof_v<Description>;
    constexpr sz_t state_size = (detail::maybe_sizeof_v<State> + sizeof(u64_t) - 1U) / sizeof(u64_t);
    u64_t * wrapper = ::new u64_t[state_size + (sizeof(resource_wrapper) + size - 1) / sizeof(u64_t)];
    if constexpr (!j::is_void_v<State>) {
      ::new (wrapper) State();
    }
    return ::new (wrapper + state_size) resource_wrapper();
  }

  template<typename Resource, typename Description, typename State>
  J_ALWAYS_INLINE J_HIDDEN void default_maybe_finalize_resource_wrapper(resource_wrapper * wrapper) noexcept {
    J_ASSERT_NOT_NULL(wrapper);
    if (wrapper->refcount) {
      return;
    }
    if (wrapper->status == wrapper_status::initialized) {
      reinterpret_cast<Resource*>(wrapper->data)->~Resource();
    } else if constexpr (!::j::is_void_v<Description>) {
      if (wrapper->status == wrapper_status::uninitialized) {
        reinterpret_cast<Description*>(wrapper->data)->~Description();
      }
    }

    u64_t * ptr = reinterpret_cast<u64_t *>(wrapper);
    if constexpr (!__is_same_as(void, State)) {
      ptr -= (sizeof(State) + sizeof(u64_t) - 1U) / sizeof(u64_t);
      if (wrapper->status != wrapper_status::empty) {
        reinterpret_cast<State*>(ptr)->~State();
      }
    }

    if (wrapper->weak_refcount) {
      wrapper->status = wrapper_status::empty;
    } else {
      wrapper->~resource_wrapper();
      ::delete[] ptr;
    }
  }

  template<typename Context, typename Resource, typename Description, typename Ref, auto CacheGetFn = nullptr, auto CacheSetFn = nullptr>
  J_ALWAYS_INLINE J_HIDDEN static void default_initialize_resource(
    const Context & context,
    Ref * ref
  ) {
    J_ASSERT_NOT_NULL(ref, ref->wrapper);
    J_ASSERT(ref->wrapper->status == wrapper_status::uninitialized, "Expected resource_wrapper to be uninitialized.");
    Description *desc_ptr;
    if constexpr (!::j::is_void_v<Description>) {
      desc_ptr = reinterpret_cast<Description*>(ref->wrapper->data);
      if constexpr (CacheGetFn != nullptr) {
        if (CacheGetFn(context, *desc_ptr, *ref)) {
          J_ASSERT(ref->wrapper->status == wrapper_status::initialized,
                   "CacheGetFn did not initialize resource wrapper.");
          return;
        }
      }
    }

    ref->wrapper->status = wrapper_status::initializing;

    try {
      if constexpr (!::j::is_void_v<Description>) {
        // Move the description to stack, since the wrapper stores it in the same
        // memory as the resource.
        Description desc{static_cast<Description &&>(*desc_ptr)};
        desc_ptr->~Description();
        if constexpr (CacheSetFn != nullptr) {
          ::new (ref->wrapper->data) Resource(context, desc);
          CacheSetFn(context, desc, *ref);
        } else {
          ::new (ref->wrapper->data) Resource(context, static_cast<Description &&>(desc));
        }
      } else {
        ::new (ref->wrapper->data) Resource(context);
      }
      ref->wrapper->status = wrapper_status::initialized;
    } catch (...) {
      ref->wrapper->status = wrapper_status::empty;
      throw;
    }
  }
}

#define DEFINE_WEAK_RESOURCE_TEMPLATES(RESOURCE, DESCRIPTION)                                                                         \
  template class ::j::rendering::vulkan::resources::weak_resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DEFINE_RESOURCE_TEMPLATES(RESOURCE, DESCRIPTION)                                                                              \
  template class ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DEFINE_RESOURCE_DEFINITION(RESOURCE, DESCRIPTION, STATE, ...)                                                                 \
  void RESOURCE##_definition::initialize_resource(                                                                                    \
    const ::j::rendering::vulkan::device_context_base & context,                                                                      \
    ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition> * ref                               \
  ) {                                                                                                                                 \
    ::j::rendering::vulkan::resources::default_initialize_resource<::j::rendering::vulkan::device_context_base, RESOURCE,             \
      DESCRIPTION, ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>,                     \
      ##__VA_ARGS__>(context, ref); \
  }                                                                                                                                   \
  ::j::rendering::vulkan::resources::resource_wrapper*                                                                                \
    RESOURCE##_definition::create_resource_wrapper()                                                                                  \
  {                                                                                                                                   \
    return ::j::rendering::vulkan::resources::default_create_resource_wrapper<RESOURCE, DESCRIPTION, STATE>();                        \
  }                                                                                                                                   \
  void RESOURCE##_definition::maybe_finalize_resource_wrapper(                                                                        \
    ::j::rendering::vulkan::resources::resource_wrapper* wrapper) noexcept                                                            \
  {                                                                                                                                   \
    ::j::rendering::vulkan::resources::default_maybe_finalize_resource_wrapper<RESOURCE, DESCRIPTION, STATE>(wrapper);                \
  }

#define DEFINE_SURFACE_RESOURCE_DEFINITION(RESOURCE, DESCRIPTION, STATE, ...)                                                         \
  void RESOURCE##_definition::initialize_resource(                                                                                    \
    const ::j::rendering::vulkan::context::surface_context & context,                                                                 \
    ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition> * ref                               \
  ) {                                                                                                                                 \
    ::j::rendering::vulkan::resources::default_initialize_resource<::j::rendering::vulkan::surface_context, RESOURCE, DESCRIPTION, \
      ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>, \
      ##__VA_ARGS__>(context, ref); \
  }                                                                                                                                   \
  ::j::rendering::vulkan::resources::resource_wrapper*                                                                                \
    RESOURCE##_definition::create_resource_wrapper()                                                                                  \
  {                                                                                                                                   \
    return ::j::rendering::vulkan::resources::default_create_resource_wrapper<RESOURCE, DESCRIPTION, STATE>();                        \
  }                                                                                                                                   \
  void RESOURCE##_definition::maybe_finalize_resource_wrapper(                                                                        \
    ::j::rendering::vulkan::resources::resource_wrapper* wrapper) noexcept                                                                     \
  {                                                                                                                                   \
    ::j::rendering::vulkan::resources::default_maybe_finalize_resource_wrapper<RESOURCE, DESCRIPTION, STATE>(wrapper);                \
  }
