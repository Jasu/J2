#pragma once

#include "rendering/vulkan/resources/resource_wrapper.hpp"
#include "hzd/mem.hpp"
#include "hzd/type_traits.hpp"

namespace j::rendering::vulkan::inline context {
  class device_context_base;
  class surface_context;
}

namespace j::rendering::vulkan::resources {
  namespace detail {
    enum class create_tag_t { v };
  }

  namespace detail {
    struct resource_ref_weak_base {
      /// Pointer to the actual resource, wrapped with book-keeping information.
      mutable resource_wrapper * wrapper = nullptr;

      J_ALWAYS_INLINE constexpr resource_ref_weak_base() noexcept = default;

      [[nodiscard]] J_RETURNS_NONNULL static resource_wrapper * containing_record(const void * J_NOT_NULL resource) noexcept;

      J_ALWAYS_INLINE constexpr explicit resource_ref_weak_base(resource_wrapper * rec) noexcept
        : wrapper(rec)
      {
      }

      J_ALWAYS_INLINE resource_ref_weak_base(const resource_ref_weak_base & rhs) noexcept = default;

      J_ALWAYS_INLINE resource_ref_weak_base(resource_ref_weak_base && rhs) noexcept
        : wrapper(rhs.wrapper)
      { rhs.wrapper = nullptr; }

      J_INLINE_GETTER bool operator!() const noexcept
      { return !wrapper || wrapper->status == wrapper_status::empty; }

      J_INLINE_GETTER explicit operator bool() const noexcept
      { return !operator!(); }

      J_INLINE_GETTER bool operator==(const resource_ref_weak_base &) const noexcept = default;
    };


    struct resource_ref_base : public resource_ref_weak_base {
    public:
      J_ALWAYS_INLINE explicit resource_ref_base(resource_wrapper * rec) : resource_ref_weak_base(rec) {
        if (rec) {
          ++rec->refcount;
        }
      }

      J_ALWAYS_INLINE constexpr resource_ref_base() noexcept = default;

      resource_ref_base(const resource_ref_base & rhs) noexcept;

      [[nodiscard]] bool is_initialized() const noexcept {
        return wrapper && wrapper->status == wrapper_status::initialized;
      }

      void assert_not_empty() const noexcept {
        J_ASSUME_NOT_NULL(wrapper);
        J_ASSUME(wrapper->status != wrapper_status::empty);
      }
    };
  }

  template<
    typename Resource,
    typename Description,
    typename ResourceDefinition
  >
  class resource_ref final : public detail::resource_ref_base
  {
    template<typename... DescArgs>
    J_RETURNS_NONNULL J_HIDDEN static resource_wrapper * create_wrapper(DescArgs && ... desc_args);

    template<typename T>
    struct J_HIDDEN enable_if_not_same {
      using type J_NO_DEBUG_TYPE = void;
    };

    template<>
    struct J_HIDDEN enable_if_not_same<resource_ref &> { };

    template<>
    struct J_HIDDEN enable_if_not_same<const resource_ref &> { };

    template<>
    struct J_HIDDEN enable_if_not_same<resource_wrapper * &> { };
  public:
    J_INLINE_GETTER static resource_ref from_this(const Resource * J_NOT_NULL resource) noexcept {
      return resource_ref{containing_record(resource)};
    }

    using resource_ref_base::resource_ref_base;

    using resource_t J_NO_DEBUG_TYPE = Resource;
    using resource_definition_t J_NO_DEBUG_TYPE = ResourceDefinition;

    constexpr resource_ref() noexcept = default;

    template<typename... DescArgs>
    J_HIDDEN J_ALWAYS_INLINE static resource_ref create(DescArgs && ... desc_args) {
      return resource_ref{create_wrapper(static_cast<DescArgs&&>(desc_args)...)};
    }

    void reset() noexcept;

    ~resource_ref() {
      reset();
    }

    template<typename DescArg,
             typename... DescArgs,
             u8_t = -j::is_void_v<Description>,
             typename = typename enable_if_not_same<DescArg &>::type
    >
    J_ALWAYS_INLINE resource_ref(DescArg && head, DescArgs && ... tail)
      : resource_ref_base(create_wrapper(static_cast<DescArg &&>(head), static_cast<DescArgs &&>(tail)...))
    {
    }

    template<typename... DescArgs>
    J_ALWAYS_INLINE explicit resource_ref(const detail::create_tag_t &, DescArgs && ... args)
      : resource_ref_base(create_wrapper(static_cast<DescArgs &&>(args)...))
    {
    }

    resource_ref(const resource_ref & rhs) noexcept = default;
    resource_ref(resource_ref && rhs) noexcept = default;

    resource_ref & operator=(const resource_ref & rhs) noexcept;

    resource_ref & operator=(resource_ref && rhs) noexcept;

    J_INLINE_GETTER Resource & get() const noexcept {
      J_ASSERT(wrapper != nullptr && wrapper->status == wrapper_status::initialized);
      return *reinterpret_cast<Resource*>(wrapper->data);
    }
  };


  template<typename Res, typename Desc, typename Def>
  template<typename... DescArgs>
  J_RETURNS_NONNULL J_HIDDEN resource_wrapper * resource_ref<Res, Desc, Def>::create_wrapper(DescArgs && ... desc_args) {
    resource_wrapper * wrapper = Def::create_resource_wrapper();
    if constexpr (!::j::is_void_v<Desc>) {
      if constexpr (::j::is_nothrow_constructible_v<Desc, DescArgs && ...>) {
        ::new (wrapper->data) Desc(static_cast<DescArgs &&>(desc_args)...);
      } else {
        try {
          ::new (wrapper->data) Desc(static_cast<DescArgs &&>(desc_args)...);
        } catch (...) {
          Def::maybe_finalize_resource_wrapper(wrapper);
          throw;
        }
      }
    } else {
      static_assert(sizeof...(DescArgs) == 0);
    }
    return wrapper;
  }

  template<typename Res, typename Desc, typename Def>
  resource_ref<Res, Desc, Def> & resource_ref<Res, Desc, Def>::operator=(resource_ref && rhs) noexcept {
    if (J_LIKELY(rhs.wrapper != wrapper)) {
      reset();
      wrapper = rhs.wrapper;
      rhs.wrapper = nullptr;
    }
    return *this;
  }

  template<typename Resource, typename Description, typename ResourceDefinition>
  void resource_ref<Resource, Description, ResourceDefinition>::reset() noexcept {
    if (wrapper) {
      --wrapper->refcount;
      ResourceDefinition::maybe_finalize_resource_wrapper(wrapper);
      wrapper = nullptr;
    }
  }

  template<typename Res, typename Desc, typename Def>
  resource_ref<Res, Desc, Def> & resource_ref<Res, Desc, Def>
    ::operator=(const resource_ref & rhs) noexcept
  {
    if (J_LIKELY(rhs.wrapper != wrapper)) {
      reset();
      if (J_LIKELY(rhs.wrapper)) {
        wrapper = rhs.wrapper;
        ++wrapper->refcount;
      }
    }
    return *this;
  }
}
namespace j::rendering::vulkan {
  J_NO_DEBUG inline constexpr resources::detail::create_tag_t create_tag = resources::detail::create_tag_t::v;
}
