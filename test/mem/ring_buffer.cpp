#include <detail/preamble.hpp>

#include "mem/ring_buffer.hpp"

TEST_SUITE_BEGIN("Mem - Ring buffer");
#define J_BUF_INFO(BUF) \
  INFO("Buf: ", BUF.capacity, "b " \
        "Readable ", BUF.readable_size(), "b at ", BUF.readable_index(), ", " \
        "Writable ", BUF.writable_size(), "b at ", BUF.writable_index())

namespace j::mem::test {
  namespace {
    const char * const test_str =
      "Dr. Thurston has already been quoted to the effect that the admitted "
      "safety of a water-tube boiler is the result of the division of its "
      "contents into small portions. In boilers using a water-leg construction, "
      "while the danger from explosion will be largely limited to the tubes, "
      "there is the danger, however, that such legs may explode due to the "
      "deterioration of their stays, and such an explosion might be almost as "
      "disastrous as that of a shell boiler.";

    struct J_TYPE_HIDDEN buf_expectation final {
      i32_t readable_size = -1;
      i32_t readable_index = -1;
      i32_t writable_size = -1;
      i32_t writable_index = -1;
    };

    void assert_ptrs(const ring_buffer & buf) {
      J_BUF_INFO(buf);
      REQUIRE_UNARY(buf.base);
      REQUIRE_UNARY(buf.capacity);
      REQUIRE_UNARY(!(buf.capacity & (buf.capacity - 1U)));
      REQUIRE_EQ(buf.capacity, buf.readable_size() + buf.writable_size());
    }

    void assert_buf_state(const ring_buffer & buf, buf_expectation exp) {
      assert_ptrs(buf);
      if (exp.writable_size >= 0) { REQUIRE_EQ(buf.writable_size(), exp.writable_size); }
      if (exp.writable_index >= 0) { REQUIRE_EQ(buf.writable_index(), exp.writable_index); }

      if (exp.readable_size >= 0) { REQUIRE_EQ(buf.readable_size(), exp.readable_size); }
      if (exp.readable_index >= 0) { REQUIRE_EQ(buf.readable_index(), exp.readable_index); }
    }
  }

  TEST_CASE("Empty buffer") {
    ring_buffer buf(1024);
    assert_buf_state(buf, {
        .readable_size = 0U,
        .readable_index = 0U,
        .writable_size = (i32_t)buf.capacity,
        .writable_index = 0U,
     });
  }

  TEST_CASE("Small R/W") {
    ring_buffer buf(512);
    REQUIRE_EQ(168, buf.write(test_str, 168));
    buf_expectation exp = {
      .readable_size = 168U,
      .readable_index = 0U,
      .writable_size = (i32_t)buf.capacity - 168,
      .writable_index = 168,
    };
    assert_buf_state(buf, exp);
    char buf2[200];
    ::j::memzero(buf2, 200);
    REQUIRE_EQ(168, buf.read(buf2, 168));
    exp.readable_size = 0;
    exp.readable_index = 168;
    exp.writable_size = buf.capacity;
    assert_buf_state(buf, exp);
    REQUIRE_UNARY(!::j::memcmp(buf2, test_str, 168));
  }
}


TEST_SUITE_END();
