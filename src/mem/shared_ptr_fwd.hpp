#pragma once

#include "mem/deleter.hpp"
#include "mem/shared_holder.hpp"

namespace j::mem {
  namespace detail {
    struct shared_ptr_control_block;
  }

  template<typename T> class weak_ptr;

  template<typename T>
  class shared_ptr final {
  public:
    using nicely_copyable_tag_t J_NO_DEBUG_TYPE = void;
    using zero_initializable_tag_t J_NO_DEBUG_TYPE = void;

    J_A(AI,ND) inline shared_ptr() noexcept = default;

    /// Aliasing move constructor.
    ///
    /// Constructs a shared_ptr that shares ownership of the object (previously)
    /// owned by owning_ptr, but contains a distinct pointer to T*.
    ///
    /// I.e. the object that would have been deleted by owning_ptr, had it been
    /// the last remaining pointer, will be deleted by this. aliasing_ptr is
    /// the object pointed to, and will not be deleted at all.
    ///
    /// This is used to implement the pointer casts, as they create a shared
    /// pointer that shares ownership of the original object, but point to some
    /// other facet of that object.
    template<typename U>
    J_A(AI) inline shared_ptr(shared_ptr<U> && owning_ptr, T * J_NOT_NULL aliasing_ptr) noexcept
      : m_target(aliasing_ptr),
        holder(static_cast<shared_holder &&>(owning_ptr.holder))
    {
    }

    /// Aliasing copy constructor.
    template<typename U>
    inline explicit shared_ptr(const shared_ptr<U> & owning_ptr, T * J_NOT_NULL aliasing_ptr) noexcept
      : m_target(aliasing_ptr),
        holder(owning_ptr.holder)
    {
    }

    template<OtherThan<T> U>
    J_A(AI,ND) inline shared_ptr(shared_ptr<U> && ptr) noexcept
      : m_target(ptr.m_target),
        holder(static_cast<shared_holder &&>(ptr.holder))
    {
    }

    template<OtherThan<T> U>
    J_A(ND) inline shared_ptr(const shared_ptr<U> & ptr) noexcept
      : m_target(ptr.m_target),
        holder(ptr.holder)
    {
    }

    template<OtherThan<T> U>
    J_A(AI,ND) inline shared_ptr & operator=(shared_ptr<U> && ptr) noexcept {
      m_target = ptr.m_target;
      holder = static_cast<shared_holder &&>(ptr.holder);
      return *this;
    }

    template<OtherThan<T> U>
    J_A(ND) inline shared_ptr & operator=(const shared_ptr<U> & ptr) noexcept {
      m_target = ptr.m_target;
      holder = ptr.holder;
      return *this;
    }

    void reset() noexcept {
      m_target = nullptr;
      holder.reset();
    }

    J_A(AI,ND,NODISC) inline T * get() const noexcept { return m_target; }

    J_A(AI,ND,NODISC) inline T * operator->() const noexcept {
      return m_target;
    }

    J_A(AI,ND,NODISC) inline bool operator!() const noexcept { return !(bool)holder; }

    J_A(AI,ND,NODISC) explicit inline operator bool() const noexcept { return (bool)holder; }

    J_A(AI,ND,NODISC) inline auto & operator*() const noexcept {
      return *m_target;
    }

    J_A(AI,ND) inline explicit shared_ptr(T *J_AA(NN,NOALIAS) target, detail::shared_ptr_control_block *J_AA(NN,NOALIAS) control_block) noexcept
      : m_target(target),
        holder(control_block)
    {
    }
  private:
    template<typename> friend class shared_ptr;
    template<typename> friend class weak_ptr;

      friend class shared_holder;

      T * m_target = nullptr;
  public:
      shared_holder holder{};
    };

    template<>
    auto & shared_ptr<void>::operator*() const noexcept;

    template<typename T, auto Deleter = destructor_v<T>, typename Result = T, typename... Args>
    shared_ptr<Result> make_shared_sized(u32_t size, Args && ... args) noexcept(is_nothrow_constructible_v<T, Args...>);

  template<typename T, auto Deleter = destructor_v<T>, typename... Args>
  [[nodiscard]] shared_ptr<void> make_void_shared(Args && ... args) {
    return make_shared_sized<T, Deleter, void, Args...>(sizeof(T), static_cast<Args &&>(args)...);
  }

  template<typename T, auto Deleter = destructor_v<T>, typename Result = T, typename... Args>
  J_A(AI,NODISC) inline shared_ptr<Result> make_shared(Args && ... args) noexcept(is_nothrow_constructible_v<T, Args...>) {
    return make_shared_sized<T, Deleter, Result, Args...>(sizeof(T), static_cast<Args &&>(args)...);
  }

  template<typename U, typename V>
  [[nodiscard]] static shared_ptr<U> reinterpret_pointer_cast(const mem::shared_ptr<V> & ptr) noexcept {
    return shared_ptr<U>{ptr, reinterpret_cast<U*>(ptr.get())};
  }

  template<typename U, typename V>
  [[nodiscard]] static shared_ptr<U> reinterpret_pointer_cast(mem::shared_ptr<V> && ptr) noexcept {
    return shared_ptr<U>{static_cast<mem::shared_ptr<V> &&>(ptr), reinterpret_cast<U*>(ptr.get())};
  }

  template<typename U, typename V>
  [[nodiscard]] static shared_ptr<U> static_pointer_cast(const mem::shared_ptr<V> & ptr) noexcept {
    return shared_ptr<U>{ptr, static_cast<U*>(ptr.get())};
  }

  template<typename U, typename V>
  [[nodiscard]] static shared_ptr<U> static_pointer_cast(mem::shared_ptr<V> && ptr) noexcept {
    return shared_ptr<U>{static_cast<mem::shared_ptr<V> &&>(ptr), static_cast<U*>(ptr.get())};
  }

  template<typename U, typename V>
  J_A(AI,ND,NODISC) inline bool operator==(const shared_ptr<U> & lhs, const shared_ptr<V> & rhs) noexcept {
    return lhs.get() == rhs.get();
  }
}

#define J_DECLARE_EXTERN_SHARED_PTR(T) extern template class j::mem::shared_ptr<T>
#define J_DEFINE_EXTERN_SHARED_PTR(T) template class j::mem::shared_ptr<T>

J_DECLARE_EXTERN_SHARED_PTR(void);
