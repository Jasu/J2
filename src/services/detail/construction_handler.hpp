#pragma once

#include "services/detail/dependencies_t.hpp"
#include "services/injected_calls/type_erased_injected_call.hpp"
#include "services/injected_calls/argument_list.hpp"
#include "services/finalizer_call.hpp"
#include "containers/trivial_array_fwd.hpp"
#include "mem/weak_ptr.hpp"
#include "mem/shared_ptr.hpp"

namespace j::services::detail {
  template<typename T>
  inline constexpr bool is_shared_ptr_v = false;

  template<typename T>
  inline constexpr bool is_shared_ptr_v<j::mem::shared_ptr<T>> = true;

  void null_deleter(void * J_NOT_NULL_NOESCAPE) noexcept;

  template<typename Service, sz_t NumFinalizers, bool NoDelete, bool HasDeps>
  struct deleter {
    const detail::finalizer_call_set & finalizer;
    dependencies_t deps;
    deleter(const detail::finalizer_call_set & finalizer, dependencies_t && deps) noexcept;
    void operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept;
  };

  template<typename Service, sz_t NumFinalizers, bool NoDelete, bool HasDeps>
  deleter<Service, NumFinalizers, NoDelete, HasDeps>::deleter(const detail::finalizer_call_set & finalizer, dependencies_t && deps) noexcept
    : finalizer(finalizer),
      deps(static_cast<dependencies_t &&>(deps))
  {
  }

  template<typename Service, sz_t NumFinalizers, bool NoDelete, bool HasDeps>
  void deleter<Service, NumFinalizers, NoDelete, HasDeps>::operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept {
    finalizer(service);
    if constexpr (!NoDelete) {
      ::delete service;
    }
    deps.clear();
  }

  template<typename Service, sz_t NumFinalizers, bool NoDelete>
  struct deleter<Service, NumFinalizers, NoDelete, false> {
    const detail::finalizer_call_set & finalizer;
    deleter(const detail::finalizer_call_set & finalizer) noexcept;
    void operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept;
  };

  template<typename Service, sz_t NumFinalizers, bool NoDelete>
  deleter<Service, NumFinalizers, NoDelete, false>::deleter(const detail::finalizer_call_set & finalizer) noexcept
    : finalizer(finalizer)
  {
  }

  template<typename Service, sz_t NumFinalizers, bool NoDelete>
  void deleter<Service, NumFinalizers, NoDelete, false>::operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept {
    finalizer(service);
    if constexpr (!NoDelete) {
      delete service;
    }
  }

  template<typename Service, bool NoDelete>
  struct deleter<Service, 0, NoDelete, true> {
    dependencies_t deps;
    deleter(const detail::finalizer_call_set &, dependencies_t && deps) noexcept;
    void operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept;
  };

  template<typename Service, bool NoDelete>
  deleter<Service, 0, NoDelete, true>::deleter(const detail::finalizer_call_set &, dependencies_t && deps) noexcept
    : deps(static_cast<dependencies_t &&>(deps))
  {
  }

  template<typename Service, bool NoDelete>
  void deleter<Service, 0, NoDelete, true>::operator()(Service * J_NOT_NULL_NOESCAPE service) noexcept {
    if constexpr (!NoDelete) {
      ::delete service;
    }
    deps.clear();
  }

  template<typename Service, bool NoDelete, sz_t NumFinalizers, bool HasDependencies>
  mem::shared_ptr<void> wrap_service_shared(Service * ptr, const detail::finalizer_call_set & finalizer, dependencies_t * deps = nullptr) noexcept {
    if constexpr (HasDependencies) {
      return mem::wrap_shared<Service>(
        ptr,
        deleter<Service, NumFinalizers, NoDelete, true>(finalizer, static_cast<dependencies_t &&>(*deps)));
    } else if constexpr (NumFinalizers == 0) {
      if constexpr (NoDelete) {
        return mem::wrap_shared<Service, &null_deleter>(ptr);
      } else {
        return mem::wrap_shared<Service>(ptr);
      }
    } else if constexpr (NoDelete) {
      return mem::wrap_shared<Service>(ptr, finalizer);
    } else {
      return mem::wrap_shared<Service>(ptr, deleter<Service, NumFinalizers, false, false>(finalizer));
    }
  }

  template<bool NoDelete, sz_t NumFinalizers, typename Service>
  void apply_initializers(
    Service * J_NOT_NULL service,
    const trivial_array<injected_calls::type_erased_injected_call> & init,
    const detail::finalizer_call_set & fini,
    services::container * c,
    const injection_context * ic,
    dependencies_t * deps = nullptr
  ) {
    if constexpr (NumFinalizers != 0 || !NoDelete) {
      try {
        for (auto & i : init) {
          i(c, ic, deps, service);
        }
      } catch (...) {
        if constexpr (NumFinalizers != 0) {
          fini(service);
        }
        if constexpr (!NoDelete) {
          ::delete service;
        }
        throw;
      }
    } else {
      for (auto & i : init) {
        i(c, ic, deps, service);
      }
    }
  }

  struct dummy_deps {
    J_ALWAYS_INLINE void clear() const noexcept { }
    J_ALWAYS_INLINE dependencies_t * operator&() const noexcept {
      return nullptr;
    }
  };

  template<bool HasConstructorDeps, bool HasInitializerDeps>
  struct maybe_dependencies {
    using type J_NO_DEBUG_TYPE = dependencies_t;
  };

  template<>
  struct maybe_dependencies<false, false> {
    using type J_NO_DEBUG_TYPE = dummy_deps;
  };

  template<typename ConstructionHandler, bool HasInitializerDeps>
  using maybe_dependencies_t J_NO_DEBUG_TYPE = typename maybe_dependencies<ConstructionHandler::has_dependencies_v, HasInitializerDeps>::type;

  /// Normal constructor
  template<
    typename Service,
    typename WiredArgs,
    bool NoDelete,
    sz_t NumInitializers,
    bool HasInitializerDeps,
    sz_t NumFinalizers
  >
  struct ctor_construction_handler;

  template<
    typename Service,
    bool NoDelete,
    sz_t NumInitializers,
    bool HasInitializerDeps,
    sz_t NumFinalizers,
    typename... WiredArgs
  >
  struct ctor_construction_handler<Service, injected_calls::wired_argument_list<WiredArgs...>, NoDelete, NumInitializers, HasInitializerDeps, NumFinalizers> {
    using ctor_t = injected_calls::wired_argument_list<WiredArgs...>;
    static inline ctor_t m_constructor;
    static inline const trivial_array<injected_calls::type_erased_injected_call> * m_initializers;
    static inline const detail::finalizer_call_set * m_finalizer;
  public:
    J_ALWAYS_INLINE static void initialize(
      ctor_t && ctor,
      const trivial_array<injected_calls::type_erased_injected_call> & initializer,
      const detail::finalizer_call_set & finalizer) noexcept
    {
      m_constructor = static_cast<injected_calls::wired_argument_list<WiredArgs...> &&>(ctor);
      if constexpr (NumInitializers != 0) {
        m_initializers = &initializer;
      }
      if constexpr (NumFinalizers != 0) {
        m_finalizer = &finalizer;
      }
    }
    static mem::shared_ptr<void> construct(services::container * c, const injection_context * ic) {
      if constexpr (!HasInitializerDeps && !ctor_t::has_dependencies_v) {
        if constexpr (NumFinalizers == 0 && !NoDelete) {
          // Make shared can be used, if nothing extra has to be done in the deleter.
          mem::shared_ptr<void> result = m_constructor.invoke(
            mem::make_void_shared<Service, j::mem::destructor_v<Service>, typename WiredArgs::result_type...>, c, ic, nullptr);
          if constexpr (NumInitializers != 0) {
            void * ptr = result.get();
            for (auto & i : *m_initializers) {
              i(c, ic, nullptr, ptr);
            }
          }
          return result;
        } else {
          Service * ptr = m_constructor.invoke([](auto && ... args) {
            return ::new Service(static_cast<decltype(args)>(args)...);
          }, c, ic, nullptr);
          if constexpr (NumInitializers != 0) {
            apply_initializers<NoDelete, NumFinalizers>(ptr, m_initializers, *m_finalizer, c, ic);
          }
          return wrap_service_shared<Service, NoDelete, NumFinalizers, false>(ptr, *m_finalizer);
        }
      } else {
        dependencies_t deps;
        Service * ptr = m_constructor.invoke([](auto && ... args) {
          return ::new Service(static_cast<decltype(args)>(args)...);
        }, c, ic, &deps);
        if constexpr (NumInitializers != 0) {
          apply_initializers<NoDelete, NumFinalizers>(ptr, m_initializers, *m_finalizer, c, ic, &deps);
        }
        return wrap_service_shared<Service, NoDelete, NumFinalizers, true>(ptr, *m_finalizer, &deps);
      }
    }
  };

  template<typename Service,
           sz_t NumInitializers,
           bool HasInitializerDeps,
           sz_t NumFinalizers,
           typename Factory,
           bool NoDelete>
  class factory_construction_handler {
    static inline const trivial_array<injected_calls::type_erased_injected_call> * m_initializers;
    static inline const detail::finalizer_call_set * m_finalizer;
    static inline Factory m_factory;

  public:
    J_ALWAYS_INLINE static void initialize(
      Factory && factory,
      const trivial_array<injected_calls::type_erased_injected_call> & initializer,
      const detail::finalizer_call_set & finalizer) noexcept
    {
      m_factory = static_cast<Factory &&>(factory);
      if constexpr (NumInitializers != 0) {
        m_initializers = &initializer;
      }
      if constexpr (NumFinalizers != 0) {
        m_finalizer = &finalizer;
      }
    }

    static mem::shared_ptr<void> construct(services::container * c, const injection_context * ic) {
      if constexpr (HasInitializerDeps || Factory::has_dependencies_v) {
        dependencies_t deps;
        auto ptr = m_factory(c, ic, &deps);
        constexpr bool is_shared = is_shared_ptr_v<decltype(ptr)>;
        Service * raw;
        if constexpr (is_shared) {
          raw = ptr.get();
          deps.push(mem::shared_holder(static_cast<mem::shared_ptr<Service> &&>(ptr)));
        } else {
          raw = ptr;
        }
        if constexpr (NumInitializers != 0) {
          apply_initializers<NoDelete || is_shared, NumFinalizers>(raw, m_initializers, *m_finalizer, c, ic, &deps);
        }
        return wrap_service_shared<Service, NoDelete || is_shared, NumFinalizers, true>(ptr, *m_finalizer, &deps);
      } else {
        auto ptr = m_factory(c, ic, nullptr);
        if constexpr (is_shared_ptr_v<decltype(ptr)>) {
          if constexpr (NumInitializers != 0) {
            apply_initializers<true, NumFinalizers>(ptr.get(), m_initializers, *m_finalizer, c, ic);
          }
          return static_cast<decltype(ptr) &&>(ptr);
        } else {
          if constexpr (NumInitializers != 0) {
            apply_initializers<NoDelete, NumFinalizers>(ptr, m_initializers, *m_finalizer, c, ic);
          }
          return wrap_service_shared<Service, NoDelete, NumFinalizers, false>(ptr, *m_finalizer, nullptr);
        }
      }
    }
  };


  template<typename Service, sz_t NumInitializers, bool HasInitializerDeps, sz_t NumFinalizers>
  class static_singleton_construction_handler {
    static inline void * m_ptr;
    static inline mem::weak_ptr<void> m_instance;
    static inline typename maybe_dependencies<false, HasInitializerDeps>::type m_dependencies;
    static inline const trivial_array<injected_calls::type_erased_injected_call> * m_initializers;
    static inline const detail::finalizer_call_set * m_finalizer;
  public:
    J_ALWAYS_INLINE static void initialize(
      void * ptr,
      const trivial_array<injected_calls::type_erased_injected_call> & initializer,
      const detail::finalizer_call_set & finalizer) noexcept
    {
      m_ptr = ptr;
      if constexpr (NumInitializers != 0) {
        m_initializers = &initializer;
      }
      if constexpr (NumFinalizers != 0) {
        m_finalizer = &finalizer;
      }
    }

    static void deleter(void * J_NOT_NULL_NOESCAPE service) noexcept {
      if constexpr (NumFinalizers != 0) {
        (*m_finalizer)(service);
      }
      if constexpr (HasInitializerDeps) {
        m_dependencies.clear();
      }
    }

    static mem::shared_ptr<void> construct(services::container * c, const injection_context * ic) {
      mem::shared_ptr<void> result{m_instance.lock()};
      if (J_UNLIKELY(!result)) {
        result = mem::wrap_shared<void>(static_cast<Service*>(m_ptr), &deleter);
        if constexpr (NumInitializers != 0) {
          for (auto & i : *m_initializers) {
            i(c, ic, &m_dependencies, m_ptr);
          }
        }
        m_instance = result;
      }
      return result;
    }
  };

  /// Global singleton, placement
  template<
    typename Service,
    typename Constructor,
    bool NoDelete,
    sz_t NumInitializers,
    bool HasInitializerDeps,
    sz_t NumFinalizers
  >
  class placement_singleton_construction_handler final {
    static inline mem::weak_ptr<void> m_instance;
    static inline typename maybe_dependencies<Constructor::has_dependencies_v, HasInitializerDeps>::type m_dependencies;
    static inline u64_t m_instance_data[(sizeof(Service) + sizeof(u64_t) - 1) / sizeof(u64_t)];
    static inline u64_t m_constructor[(sizeof(Constructor) + sizeof(u64_t) - 1) / sizeof(u64_t)];
    static inline const trivial_array<injected_calls::type_erased_injected_call> * m_initializers;
    static inline const detail::finalizer_call_set * m_finalizer;
  public:
    J_ALWAYS_INLINE static void initialize(
      Constructor && ctor,
      const trivial_array<injected_calls::type_erased_injected_call> & initializer,
      const detail::finalizer_call_set & finalizer) noexcept
    {
      ::new (&m_constructor[0]) Constructor(static_cast<Constructor &&>(ctor));
      if constexpr (NumInitializers != 0) {
        m_initializers = &initializer;
      }
      if constexpr (NumFinalizers != 0) {
        m_finalizer = &finalizer;
      }
    }

    static void deleter(void * J_NOT_NULL_NOESCAPE service) noexcept {
      if constexpr (NumFinalizers != 0) {
        (*m_finalizer)(service);
      }
      if constexpr (!NoDelete) {
        static_cast<Service*>(service)->~Service();
      }
      if constexpr (Constructor::has_dependencies_v || HasInitializerDeps) {
        m_dependencies.clear();
      }
    }

    static mem::shared_ptr<void> construct(services::container * c, const injection_context * ic) {
      mem::shared_ptr<void> result{m_instance.lock()};
      if (J_UNLIKELY(!result)) {
        result = mem::wrap_shared<void>(
          reinterpret_cast<Constructor*>(&m_constructor[0])->invoke(
             [](void * target, auto && ... args) J_NO_DEBUG J_HIDDEN -> Service*{
              return ::new (target) Service(static_cast<decltype(args)>(args)...);
            }, c, ic, &m_dependencies, &m_instance_data[0]), &deleter);
        if constexpr (NumInitializers != 0) {
          void * ptr = result.get();
          for (auto & i : *m_initializers) {
            i(c, ic, &m_dependencies, ptr);
          }
        }
        m_instance = result;
      }
      return result;
    }
  };
}
