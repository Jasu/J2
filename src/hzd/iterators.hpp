#pragma once

#include "hzd/concepts.hpp"

namespace j {
  template<typename It>
  [[nodiscard]] inline constexpr u32_t distance(It first, It last) noexcept {
    u32_t result = 0U;
    while (first != last) {
      ++first, ++result;
    }
    return result;
  }

  template<Subtractable It>
  J_A(AI,ND,NODISC) inline constexpr u32_t distance(It first, It last) noexcept {
    return last - first;
  }

  template<typename It>
  struct reverse_iterator final {
    It m_it;

    J_INLINE_GETTER auto * operator->() const noexcept {
      return &*m_it;
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return *m_it;
    }

    J_ALWAYS_INLINE reverse_iterator & operator--() noexcept {
      ++m_it;
      return *this;
    }

    J_ALWAYS_INLINE reverse_iterator operator--(int) noexcept {
      return reverse_iterator{m_it++};
    }

    J_ALWAYS_INLINE reverse_iterator & operator++() noexcept {
      --m_it;
      return *this;
    }

    J_ALWAYS_INLINE reverse_iterator operator++(int) noexcept {
      return reverse_iterator{m_it--};
    }

    J_INLINE_GETTER constexpr bool operator==(const reverse_iterator &) const noexcept = default;
  };

  template<typename It> reverse_iterator(It) -> reverse_iterator<It>;

  template<typename T>
  struct reverse_view final {
    T * original = nullptr;
    J_INLINE_GETTER decltype(auto) begin() noexcept {
      return original->rbegin();
    }
    J_INLINE_GETTER decltype(auto) begin() const noexcept {
      return original->rbegin();
    }
    J_INLINE_GETTER decltype(auto) end() noexcept {
      return original->rend();
    }
    J_INLINE_GETTER decltype(auto) end() const noexcept {
      return original->rend();
    }
  };

  template<typename T>
  struct reverse_iterator_view final {
    T * collection = nullptr;
    J_INLINE_GETTER auto begin() noexcept {
      auto it = collection->end();
      --it;
      return reverse_iterator{it};
    }
    J_INLINE_GETTER auto begin() const noexcept {
      auto it = collection->end();
      --it;
      return reverse_iterator{it};
    }
    J_INLINE_GETTER auto end() noexcept {
      auto it = collection->begin();
      --it;
      return reverse_iterator{it};
    }
    J_INLINE_GETTER auto end() const noexcept {
      auto it = collection->begin();
      --it;
      return reverse_iterator{it};
    }
  };

  template<typename T>
  reverse_iterator_view(T*) -> reverse_iterator_view<T>;

  /// Iterator for iterating singly linked lists.
  template<typename T, auto Member = false>
  struct linked_list_iterator final {
    T * it;

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return *(it->*Member);
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return *(it->*Member);
    }

    J_ALWAYS_INLINE linked_list_iterator & operator++() noexcept {
      it = it->next;
      return *this;
    }

    J_ALWAYS_INLINE linked_list_iterator operator++(int) noexcept {
      T * old = it;
      it = it->next;
      return {old};
    }

    J_INLINE_GETTER bool operator==(const linked_list_iterator & rhs) const noexcept = default;
  };

  /// Iterator for iterating singly linked lists.
  template<typename T>
  struct linked_list_iterator<T, false> final {
    T * it;

    J_INLINE_GETTER_NONNULL auto * operator->() const noexcept {
      return it;
    }

    J_INLINE_GETTER auto & operator*() const noexcept {
      return *it;
    }

    J_ALWAYS_INLINE linked_list_iterator & operator++() noexcept {
      it = it->next;
      return *this;
    }

    J_ALWAYS_INLINE linked_list_iterator operator++(int) noexcept {
      T * old = it;
      it = it->next;
      return {old};
    }

    J_INLINE_GETTER bool operator==(const linked_list_iterator & rhs) const noexcept = default;
  };

  template<typename It>
  struct deref_iterator final {
    It m_it;

    J_A(AI,NODISC,ND,RNN) inline auto * operator->() const noexcept {
      return *m_it;
    }

    J_A(AI,NODISC,ND) inline auto & operator*() const noexcept {
      return **m_it;
    }

    J_A(AI,ND) inline deref_iterator & operator++() noexcept {
      ++m_it;
      return *this;
    }

    J_A(AI,ND) inline deref_iterator operator++(int) noexcept {
      return deref_iterator{m_it++};
    }

    J_A(AI,ND,NODISC) inline constexpr bool operator==(const deref_iterator &) const noexcept = default;
  };

  template<typename It> deref_iterator(It) -> deref_iterator<It>;

  template<typename T>
  struct deref_iterator_view final {
    T * collection = nullptr;
    J_A(AI,NODISC,ND) inline auto begin() noexcept {
      return deref_iterator(collection->begin());
    }
    J_A(AI,NODISC,ND) inline auto begin() const noexcept {
      return defef_iterator(collection->begin());
    }
    J_A(AI,NODISC,ND) inline auto end() noexcept {
      return deref_iterator(collection->end());
    }
    J_A(AI,NODISC,ND) inline auto end() const noexcept {
      return deref_iterator(collection->end());
    }
  };

  template<typename T>
  deref_iterator_view(T*) -> deref_iterator_view<T>;
}
