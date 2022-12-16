#pragma once

#include "rendering/vulkan/resources/resource_ref.hpp"

#define DECLARE_WEAK_RESOURCE_REF(RESOURCE, DESCRIPTION)                                                                 \
  using weak_##RESOURCE##_ref = ::j::rendering::vulkan::resources::weak_resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DECLARE_WEAK_RESOURCE_TEMPLATES(RESOURCE, DESCRIPTION)                                                           \
  extern template class ::j::rendering::vulkan::resources::weak_resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DECLARE_RESOURCE_TEMPLATES(RESOURCE, DESCRIPTION)                                                                \
  extern template class ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DECLARE_RESOURCE(RESOURCE, DESCRIPTION, STATE)                                                                   \
  struct RESOURCE##_definition {                                                                                         \
    using state_t J_NO_DEBUG_TYPE = STATE;                                                                               \
    static ::j::rendering::vulkan::resources::resource_wrapper * create_resource_wrapper();                              \
    static void initialize_resource(                                                                                     \
      const ::j::rendering::vulkan::context::device_context_base &,                                                      \
      ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition> * ref                \
    );                                                                                                                   \
    static void maybe_finalize_resource_wrapper(::j::rendering::vulkan::resources::resource_wrapper * wrapper) noexcept; \
  };                                                                                                                     \
  using RESOURCE##_ref = ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>

#define DECLARE_SURFACE_RESOURCE(RESOURCE, DESCRIPTION, STATE)                                                           \
  struct RESOURCE##_definition {                                                                                         \
    using state_t J_NO_DEBUG_TYPE = STATE;                                                                               \
    static ::j::rendering::vulkan::resources::resource_wrapper * create_resource_wrapper();                              \
    static void initialize_resource(                                                                                     \
      const ::j::rendering::vulkan::context::surface_context &,                                                          \
      ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition> * ref                \
    );                                                                                                                   \
    static void maybe_finalize_resource_wrapper(::j::rendering::vulkan::resources::resource_wrapper * wrapper) noexcept; \
  };                                                                                                                     \
  using RESOURCE##_ref = ::j::rendering::vulkan::resources::resource_ref<RESOURCE, DESCRIPTION, RESOURCE##_definition>
