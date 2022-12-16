#pragma once

#include "functions/bound_function.hpp"

namespace j::signals {
  namespace detail {
    struct link {
      inline explicit link(bool should_delete = false) noexcept
        : m_next(reinterpret_cast<uptr_t>(this) + (should_delete ? 1ULL : 0ULL)) {
      }

      link(const link &) = delete;

      void move_from(link & rhs) noexcept {
        m_next = rhs.m_next & ~1UL;
        link * n = reinterpret_cast<link *>(m_next);
        if (n != &rhs) {
          rhs.find_predecessor()->set_next(this);
          rhs.set_next(&rhs);
        }
      }

      link(link && rhs) noexcept { move_from(rhs); }

      link & operator=(link && rhs) noexcept;

      J_RETURNS_NONNULL link * find_predecessor() const noexcept;

      void unbind() noexcept;

      J_A(AI) void set_next(link * J_NOT_NULL l) noexcept
      { m_next = reinterpret_cast<uptr_t>(l) | (m_next & 1UL); }

      J_A(AI,NODISC,RNN) link * next() const noexcept
      { return reinterpret_cast<link *>(m_next & ~1UL); }

      J_A(AI,NODISC) bool should_delete() const noexcept
      { return m_next & 1UL; }

      uptr_t m_next;
    };
  }

  template<typename Signature>
  class signal;

  template<typename Signature>
  class observer final : public functions::bound_function<Signature>, public detail::link {
  public:
    J_ALWAYS_INLINE observer(bool should_delete = false) noexcept : link(should_delete) { }

    template<auto Fn, typename Class>
    void bind(signal<Signature> & signal, Class * J_NOT_NULL o) noexcept;

    template<auto Fn>
    void bind(signal<Signature> & signal) noexcept;

    void bind(signal<Signature> & signal, Signature * J_NOT_NULL fn) noexcept;

    J_A(AI,ND) inline ~observer() {
      unbind();
    }
  };

  enum class aggregate_t : u8_t {
    all     = 0b00U,
    any     = 0b11U,
    none    = 0b01U,
    not_all = 0b10U,
  };
  enum class aggregate_reduce_t : u8_t {
    reduce,
  };

  inline constexpr aggregate_t aggregate_all{aggregate_t::all};
  inline constexpr aggregate_t aggregate_any{aggregate_t::any};
  inline constexpr aggregate_t aggregate_none{aggregate_t::none};
  inline constexpr aggregate_t aggregate_not_all{aggregate_t::not_all};

  inline constexpr aggregate_reduce_t aggregate_reduce{aggregate_reduce_t::reduce};

  template<typename Result, typename... Args>
  class signal<Result (Args...)> : public detail::link {
  public:
    signal() noexcept = default;

    ~signal();

    void connect(Result (* J_NOT_NULL fn) (Args...));

    template<auto Fn, typename Class>
    void connect(observer<Result (Args...)> & observer, Class * J_NOT_NULL o);

    template<auto Fn>
    void connect(observer<Result (Args...)> & observer);

    template<auto Fn, typename Class>
    void connect(Class * J_NOT_NULL o);

    template<auto Fn>
    void connect();

    template<typename... FnArgs>
    void operator()(FnArgs && ... args);

    template<typename Fn, typename Init, typename... FnArgs>
    J_A(ND) Init operator()(aggregate_reduce_t, Fn && fn, Init init, FnArgs && ... args) {
      detail::link * cur = next();
      while (cur != this) {
        init = static_cast<Fn &&>(fn)(
          init,
          (*static_cast<observer<Result (Args...)>*>(cur))(static_cast<FnArgs &&>(args)...));
        cur = cur->next();
      }
      return static_cast<Init &&>(init);
    }

    template<typename... FnArgs>
    J_A(ND) bool operator()(aggregate_t mode, FnArgs && ... args) {
      const bool break_on = (u8_t)mode & 0b01U;
      const bool break_result = (u8_t)mode & 0b10U;

      detail::link * cur = next();
      while (cur != this) {
        if (break_on == (bool)((*static_cast<observer<Result (Args...)>*>(cur))(static_cast<FnArgs &&>(args)...))) {
          return break_result;
        }
        cur = cur->next();
      }
      return !break_result;
    }
  };

  template<typename Signature>
  void observer<Signature>::bind(signal<Signature> & signal, Signature * J_NOT_NULL fn) noexcept {
    unbind();
    functions::bound_function<Signature>::operator=(functions::bound_function<Signature>(fn));
    set_next(signal.next());
    signal.set_next(this);
  }

  template<typename Signature>
  template<auto Fn, typename Class>
  void observer<Signature>::bind(signal<Signature> & signal, Class * J_NOT_NULL o) noexcept {
    unbind();
    functions::bound_function<Signature>::operator=(
      functions::bound_function<Signature>::template bind<Fn>(o)
    );
    set_next(signal.next());
    signal.set_next(this);
  }

  template<typename Signature>
  template<auto Fn>
  void observer<Signature>::bind(signal<Signature> & signal) noexcept {
    unbind();
    functions::bound_function<Signature>::operator=(
      functions::bound_function<Signature>::template bind<Fn>()
    );
    set_next(signal.next());
    signal.set_next(this);
  }

  template<typename Result, typename... Args>
  template<typename... FnArgs>
  void signal<Result (Args...)>::operator()(FnArgs && ... args) {
    detail::link * cur = next();
    while (cur != this) {
      (*static_cast<observer<Result (Args...)>*>(cur))(static_cast<FnArgs &&>(args)...);
      cur = cur->next();
    }
  }

  template<typename Result, typename... Args>
  template<auto Fn, typename Class>
  void signal<Result (Args...)>::connect(observer<Result (Args...)> & observer, Class * J_NOT_NULL o) {
    observer.template bind<Fn>(*this, o);
  }

  template<typename Result, typename... Args>
  template<auto Fn>
  void signal<Result (Args...)>::connect(observer<Result (Args...)> & observer) {
    observer.template bind<Fn>(*this);
  }

  template<typename Result, typename... Args>
  template<auto Fn, typename Class>
  void signal<Result (Args...)>::connect(Class * J_NOT_NULL o) {
    auto obs = ::new observer<Result (Args...)>(true);
    obs->template bind<Fn>(*this, o);
  }

  template<typename Result, typename... Args>
  template<auto Fn>
  void signal<Result (Args...)>::connect() {
    auto obs = ::new observer<Result (Args...)>(true);
    obs->template bind<Fn>(*this);
  }

  template<typename Result, typename... Args>
  void signal<Result (Args...)>::connect(Result (* J_NOT_NULL fn) (Args...)) {
    auto obs = ::new observer<Result (Args...)>(true);
    obs->bind(*this, fn);
  }

  template<typename Result, typename... Args>
  signal<Result (Args...)>::~signal() {
    while (next() != this) {
      auto cur = next();
      set_next(cur->next());
      cur->set_next(cur);
      if (cur->should_delete()) {
        ::delete static_cast<observer<Result (Args...)>*>(cur);
      }
    }
  }
}
