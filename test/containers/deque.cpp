#include <detail/preamble.hpp>

#include "detail/nontrivial.hpp"
#include "containers/deque.hpp"
#include "hzd/mem.hpp"

TEST_SUITE_BEGIN("Containers - Deque");

using namespace j;

namespace {
  template<typename T, sz_t I>
  void assert_contents(const deque<T> & d, const T (& values)[I]) {
    REQUIRE(d.size() == I);
    REQUIRE(!d.empty());
    auto it = d.begin();
    REQUIRE(it != d.end());
    for (sz_t i = 0; i < I; ++i, ++it) {
      REQUIRE(it != d.end());
      CHECK(*it == values[i]);
    }
    REQUIRE(it == d.end());
    REQUIRE(d.front() == values[0]);
    REQUIRE(d.back() == values[I - 1ULL]);
  }

  template<typename T>
  void assert_empty(const T & d) {
    REQUIRE(d.empty());
    REQUIRE(d.size() == 0);
    REQUIRE(d.begin() == d.end());
  }
}

TEST_CASE("Empty") {
  deque<int> d;
  assert_empty(d);
  d.clear();
  assert_empty(d);
}

TEST_CASE("Emplace back single") {
  deque<unsigned int> d;
  d.emplace_back(0x81713181U);
  assert_contents(d, {0x81713181U});
}

TEST_CASE("Push back single") {
  deque<unsigned int> d;
  d.push_back(0x33221919U);
  assert_contents(d, {0x33221919U});
}

TEST_CASE("Push back single const") {
  deque<unsigned int> d;
  const unsigned int i = 0x98991211;
  d.push_back(i);
  assert_contents(d, {i});
}

TEST_CASE("Emplace front single") {
  deque<unsigned int> d;
  d.emplace_front(0xF0A080F9U);
  assert_contents(d, {0xF0A080F9U});
}

TEST_CASE("Push front single") {
  deque<unsigned int> d;
  d.push_front(0x76182911U);
  assert_contents(d, {0x76182911U});
}

TEST_CASE("Push front single const") {
  deque<unsigned int> d;
  const unsigned int i = 0x99887766;
  d.push_front(i);
  assert_contents(d, {i});
}

TEST_CASE("Push back two") {
  deque<unsigned int> d;
  d.push_back(0x76182911U);
  d.push_back(0x89114401U);
  assert_contents(d, {0x76182911U, 0x89114401U});
}

TEST_CASE("Push front two") {
  deque<unsigned int> d;
  d.push_front(0x76182911U);
  d.push_front(0x89114401U);
  assert_contents(d, {0x89114401U, 0x76182911U});
}

TEST_CASE("Push back, then front") {
  deque<unsigned int> d;
  d.push_back(0x76182911U);
  d.push_front(0x89114401U);
  assert_contents(d, {0x89114401U, 0x76182911U});
}

TEST_CASE("Push front, then back") {
  deque<unsigned int> d;
  d.push_front(0xFEFEAA01U);
  d.push_back(0x77319401U);
  assert_contents(d, {0xFEFEAA01U, 0x77319401U});
}

TEST_CASE("Push back, pop back") {
  deque<unsigned int> d;
  d.push_back(0x81713181U);
  d.pop_back();
  assert_empty(d);
}

TEST_CASE("Push front, pop front") {
  deque<unsigned int> d;
  d.push_front(0x81713181U);
  d.pop_front();
  assert_empty(d);
}

TEST_CASE("Push back, pop front") {
  deque<unsigned int> d;
  d.push_back(0x81713181U);
  d.pop_front();
  assert_empty(d);
}

TEST_CASE("Push front, pop back") {
  deque<unsigned int> d;
  d.push_front(0x81713181U);
  d.pop_back();
  assert_empty(d);
}

TEST_CASE("Pop at chunk crossing") {
  deque<unsigned int> d;
  for (int first_push = 0; first_push < 2; ++first_push) {
    for (int second_push = 0; second_push < 2; ++second_push) {
      for (int first_pop = 0; first_pop < 2; ++first_pop) {
        for (int second_pop = 0; second_pop < 2; ++second_pop) {
          INFO("Push " << (first_push ? "front" : "back")
               << ", push " <<(second_push ? "front" : "back")
               << ", pop " <<(first_pop ? "front" : "back")
               << ", pop " <<(second_pop ? "front" : "back"));
          unsigned int expected_front = 0x11223344U, expected_back = 0x11223344U;
          if (first_push) {
            d.push_front(0x11223344U);
          } else {
            d.push_back(0x11223344U);
          }
          if (second_push) {
            d.push_front(0x24342311U);
            expected_front = 0x24342311U;
          } else {
            d.push_back(0x24342311U);
            expected_back = 0x24342311U;
          }

          assert_contents(d, {expected_front, expected_back});

          if (first_pop) {
            d.pop_front();
            assert_contents(d, {expected_back});
          } else {
            d.pop_back();
            assert_contents(d, {expected_front});
          }
          if (second_pop) {
            d.pop_front();
          } else {
            d.pop_back();
          }
          assert_empty(d);
        }
      }
    }
  }
}

TEST_CASE("Pop elements in the middle") {
  for (int first_pop = 0; first_pop < 2; ++first_pop) {
    for (int second_pop = 0; second_pop < 2; ++second_pop) {
      deque<unsigned int> d;
      d.push_back(0x77777777U);
      d.push_back(0x87777778U);
      d.push_back(0x97777779U);
      d.pop_front();
      assert_contents(d, {0x87777778U, 0x97777779U});

      INFO("Pop " <<(first_pop ? "front" : "back")
           << ", pop " <<(second_pop ? "front" : "back"));

      if (first_pop) {
        d.pop_front();
        assert_contents(d, {0x97777779U});
      } else {
        d.pop_back();
        assert_contents(d, {0x87777778U});
      }
      if (second_pop) {
        d.pop_front();
      } else {
        d.pop_back();
      }
      assert_empty(d);
    }
  }
}

TEST_CASE("Push elements around middle") {
  deque<unsigned int> d;
  d.push_back(0x77777777U);
  d.push_back(0x87777778U);
  d.push_back(0x97777779U);
  d.pop_front();
  d.pop_front();
  assert_contents(d, {0x97777779U});

  d.push_front(0xA777777AU);
  assert_contents(d, {0xA777777AU, 0x97777779U});
  d.push_back(0xB777777BU);
  assert_contents(d, {0xA777777AU, 0x97777779U, 0xB777777B});
}

TEST_CASE("Push elements around middle (edges)") {
  deque<unsigned int> d;
  d.push_back(0x77777777U);
  d.push_back(0x87777778U);
  d.pop_front();
  assert_contents(d, {0x87777778U});
  d.push_front(0x97777779U);
  assert_contents(d, {0x97777779U, 0x87777778U});

  deque<unsigned int> d2;
  d2.push_front(0xAABBCCDDU);
  d2.push_front(0xEEFF0011U);
  d2.pop_back();
  assert_contents(d2, {0xEEFF0011U});
  d2.push_back(0x11133311U);
  assert_contents(d2, {0xEEFF0011U, 0x11133311U});
}

TEST_CASE("Full chunks") {
  for (int is_back = 0; is_back < 2; ++is_back) {
    deque<unsigned int> d;
    for (unsigned int i = 0; i < 8U; ++i) {
      if (is_back) {
        d.push_back(0x11111111U * i);
      } else {
        d.push_front(0x11111111U * i);
      }
      REQUIRE(d.size() == i + 1U);
    }
    if (is_back) {
      assert_contents(d, {0U, 0x11111111U, 0x22222222U, 0x33333333U, 0x44444444U, 0x55555555U, 0x66666666U, 0x77777777U});
    } else {
      assert_contents(d, {0x77777777U, 0x66666666U, 0x55555555U, 0x44444444U, 0x33333333U, 0x22222222U, 0x11111111U, 0U});
    }
    d.clear();
    assert_empty(d);
  }
}

TEST_CASE("Three chunks") {
  for (int is_back = 0; is_back < 2; ++is_back) {
    for (int pop_back = 0; pop_back < 2; ++pop_back) {
      deque<unsigned int> d;
      for (unsigned int i = 0; i < 24U; ++i) {
        unsigned int n = 0x01010101U * i;
        if (is_back) {
          d.push_back(n);
        } else {
          d.push_front(n);
        }
        REQUIRE(d.front() == (is_back ? 0U : n));
        REQUIRE(d.back() == (is_back ? n : 0U));
        REQUIRE(d.size() == i + 1U);
      }
      auto it = d.begin();
      for (unsigned int i = 0; i < 24U; ++i, ((i & 1U) ? it++ : ++it)) {
        unsigned int n = 0x01010101U * (is_back ? i : 23U - i);
        REQUIRE(it != d.end());
        REQUIRE(*it == n);
      }
      REQUIRE(it == d.end());
      for (unsigned int i = 0; i < 24U; ++i) {
        pop_back ? d.pop_back() : d.pop_front();
        REQUIRE(d.size() == 23U - i);
        auto it = d.begin();
        for (unsigned int j = 0U; j < 23U - i; ++j, ((i & 1U) ? it++ : ++it)) {
          unsigned int first_index = (pop_back ? j : j + i + 1U);
          unsigned int n = 0x01010101U * (is_back ? first_index : 23U - first_index);
          REQUIRE(it != d.end());
          REQUIRE(*it == n);
        }
      }
    }
  }
}

namespace {
  void require_range(const deque<nontrivial> & d, int start, int num,
                     int copy_constructs, int move_constructs,
                     int copy_assigns, int move_assigns,
                     bool is_reversed = false)
  {
    REQUIRE(d.size() == num);
    auto it = d.begin();
    for (int i = 0; i < num; ++i, ++it) {
      REQUIRE(it != d.end());
      it->require((is_reversed ? num - i - 1 : i) + start,
                  copy_constructs, move_constructs,
                  copy_assigns, move_assigns);
    }
    REQUIRE(it == d.end());
  }
  void fill_nontrivial_deque(deque<nontrivial> & d, bool push_back, int start = 0, int num = 24) {
    REQUIRE(d.empty());
    for (int i = 0; i < num; ++i) {
      push_back ? d.push_back(start + i) : d.push_front(start + i);
      require_range_alive_non_exclusive(start, i + 1);
      require_range(d, start, i + 1,
                    0, 1, 0, 0,
                    !push_back);
    }
  }
}

TEST_CASE("Nontrivial push 1, destruct") {
  clear_alive();
  for (int push_front = 0; push_front < 2; ++push_front) {
    {
      deque<nontrivial> d;
      push_front ? d.push_front({1}) : d.push_back({1});
      REQUIRE(nontrivial::alive[1] == 1);
      d.front().require(1, 0, 1, 0, 0);
    }
    REQUIRE(nontrivial::alive[1] == 0);
  }
}

TEST_CASE("Nontrivial push 1, clear") {
  clear_alive();
  for (int push_front = 0; push_front < 2; ++push_front) {
    deque<nontrivial> d;
    push_front ? d.push_front({1}) : d.push_back({1});
    REQUIRE(nontrivial::alive[1] == 1);
    d.front().require(1, 0, 1, 0, 0);
    d.clear();
    REQUIRE(nontrivial::alive[1] == 0);
  }
}

TEST_CASE("Nontrivial fill chunk, clear") {
  clear_alive();
  for (int push_front = 0; push_front < 2; ++push_front) {
    deque<nontrivial> d;
    for (int i = 0; i < 8; ++i) {
      push_front ? d.push_front({i}) : d.push_back({i});
      require_range(d, 0, i + 1,
                    0, 1, 0, 0,
                    push_front);
      require_range_alive(0, i + 1, 1);
    }
    d.clear();
    require_none_alive();
  }
}

TEST_CASE("Nontrivial chunk edges") {
  clear_alive();
  deque<nontrivial> d;
  for (int first_push = 0; first_push < 2; ++first_push) {
    for (int second_push = 0; second_push < 2; ++second_push) {
      for (int first_pop = 0; first_pop < 2; ++first_pop) {
        for (int second_pop = 0; second_pop < 2; ++second_pop) {
          INFO("Push " << (first_push ? "front" : "back")
               << ", push " <<(second_push ? "front" : "back")
               << ", pop " <<(first_pop ? "front" : "back")
               << ", pop " <<(second_pop ? "front" : "back"));
          int expected_front = 0, expected_back = 0;
          first_push ? d.emplace_front(0) : d.emplace_back(0);
          REQUIRE(nontrivial::alive[0] == 1);
          d.front().require(0, 0, 0, 0, 0);
          if (second_push) {
            d.emplace_front(1);
            expected_front = 1;
          } else {
            d.emplace_back(1);
            expected_back = 1;
          }
          require_range_alive(0, 2);
          d.front().require(expected_front, 0, 0, 0, 0);
          d.back().require(expected_back, 0, 0, 0, 0);

          first_pop ? d.pop_front() : d.pop_back();
          d.front().require(first_pop ? expected_back : expected_front, 0, 0, 0, 0);
          REQUIRE(nontrivial::alive[expected_front] == 1 - first_pop);
          REQUIRE(nontrivial::alive[expected_back] == first_pop);
          second_pop ? d.pop_front() : d.pop_back();
          require_none_alive();
          assert_empty(d);
        }
      }
    }
  }
}

TEST_CASE("Nontrivial three chunks") {
  clear_alive();
  for (int is_back = 0; is_back < 2; ++is_back) {
    for (int pop_back = 0; pop_back < 2; ++pop_back) {
      deque<nontrivial> d;
      fill_nontrivial_deque(d, is_back);
      for (unsigned int i = 0; i < 24U; ++i) {
        pop_back ? d.pop_back() : d.pop_front();
        int first_index = (is_back != pop_back) ? i + 1 : 0;
        require_range(d, first_index, 23 - i,
                      0, 1, 0, 0,
                      !is_back);
        require_range_alive(first_index, 23 - i);
      }
    }
  }
}

TEST_CASE("Copy construct") {
  clear_alive();
  deque<nontrivial> d;
  fill_nontrivial_deque(d, true);

  deque<nontrivial> d2(d);

  require_range(d, 0, 24, 0, 1, 0, 0);
  require_range(d2, 0, 24, 1, 1, 0, 0);
  require_range_alive(0, 24, 2);

  d.clear();

  require_range(d2, 0, 24, 1, 1, 0, 0);
  require_range_alive(0, 24, 1);

  d2.clear();
  require_none_alive();
}

TEST_CASE("Copy assign") {
  clear_alive();
  deque<nontrivial> d;
  fill_nontrivial_deque(d, true);

  deque<nontrivial> d2;
  d2 = d;

  require_range(d, 0, 24, 0, 1, 0, 0);
  require_range(d2, 0, 24, 1, 1, 0, 0);
  require_range_alive(0, 24, 2);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
  d2 = d2;
#pragma clang diagnostic pop

  require_range(d, 0, 24, 0, 1, 0, 0);
  require_range(d2, 0, 24, 1, 1, 0, 0);
  require_range_alive(0, 24, 2);

  d.clear();

  require_range(d2, 0, 24, 1, 1, 0, 0);
  require_range_alive(0, 24, 1);

  d2.clear();
  require_none_alive();
}

TEST_CASE("Move assign") {
  clear_alive();
  deque<nontrivial> d;
  fill_nontrivial_deque(d, true);

  deque<nontrivial> d2;
  d2 = static_cast<deque<nontrivial> &&>(d);
  require_range(d2, 0, 24, 0, 1, 0, 0);
  require_range_alive(0, 24, 1);
  REQUIRE(d.empty());

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
  d2 = static_cast<deque<nontrivial> &&>(d2);
#pragma clang diagnostic pop

  require_range(d2, 0, 24, 0, 1, 0, 0);
  require_range_alive(0, 24, 1);

  d2.clear();

  require_none_alive();
}

TEST_CASE("Move construct") {
  {
    clear_alive();
    deque<nontrivial> d;
    fill_nontrivial_deque(d, true);

    deque<nontrivial> d2(static_cast<deque<nontrivial> &&>(d));
    require_range(d2, 0, 24, 0, 1, 0, 0);
    require_range_alive(0, 24, 1);
    REQUIRE(d.empty());
  }

  require_none_alive();
}

TEST_CASE("Move assign deletes old") {
  {
    deque<nontrivial> d;
    fill_nontrivial_deque(d, true, 0, 12);
    require_range_alive(0, 12, 1);

    deque<nontrivial> d2;
    fill_nontrivial_deque(d2, true, 12, 12);
    require_range_alive(0, 24, 1);

    d2 = static_cast<deque<nontrivial> &&>(d);
    require_range(d2, 0, 12, 0, 1, 0, 0);
    require_range_alive(0, 12, 1);
    REQUIRE(d.empty());
  }

  require_none_alive();
}

TEST_CASE("Copy assign deletes old") {
  {
    deque<nontrivial> d;
    fill_nontrivial_deque(d, true, 0, 12);
    require_range_alive(0, 12, 1);

    deque<nontrivial> d2;
    fill_nontrivial_deque(d2, true, 12, 12);
    require_range_alive(0, 24, 1);

    d2 = d;
    require_range(d, 0, 12, 0, 1, 0, 0);
    require_range(d2, 0, 12, 1, 1, 0, 0);
    require_range_alive(0, 12, 2);
  }

  require_none_alive();
}

TEST_SUITE_END();
