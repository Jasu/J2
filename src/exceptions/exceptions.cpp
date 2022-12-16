#include "exceptions/exceptions.hpp"
#include "logging/global.hpp"
#include "meta/errors.hpp"
#include "exceptions/backtrace.hpp"
#include "strings/string.hpp"

#include <cerrno>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexceptions"

namespace j::exceptions {
  namespace {
    thread_local tag_guard * cur_tag_stack = nullptr;
  }

  tag_guard::tag_guard(tags::tag && tag) noexcept
    : tag(static_cast<tags::tag &&>(tag)),
      prev(cur_tag_stack)
  {
    cur_tag_stack = this;
  }

  tag_guard::~tag_guard() {
    if (cur_tag_stack == this) {
      cur_tag_stack = prev;
    }
  }

  exception::~exception() {
  }

  logic_error_exception::logic_error_exception() noexcept { }
  out_of_range_exception::out_of_range_exception() noexcept { }
  invalid_argument_exception::invalid_argument_exception() noexcept { }
  not_implemented_exception::not_implemented_exception() noexcept { }
  bad_alloc_exception::bad_alloc_exception() noexcept { }
  system_error_exception::system_error_exception() noexcept { }

  void add_default_exception_tags(exception & e, i32_t skip_frames) noexcept {
    if (errno) {
      e.add_tag(err_errno(errno));
      errno = 0;
    }
    for (; cur_tag_stack; cur_tag_stack = cur_tag_stack->prev) {
      e.add_tag(static_cast<tags::tag &&>(cur_tag_stack->tag));
    }
    e.add_tag(backtrace(::j::backtrace::get_backtrace(2 + skip_frames)));
  }

  J_A(NORET) void throw_system_error_exception(i32_t skip_frames, const char * msg) {
    throw add_default_tags(system_error_exception() << message(msg), skip_frames);
  }

  J_A(NORET) void throw_system_error_exception(i32_t skip_frames, const char * msg, int fd) {
    throw add_default_tags(system_error_exception() << message(msg) << value(fd), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception].
  J_A(NORET) void throw_out_of_range(i32_t skip_frames) {
    throw add_default_tags(out_of_range_exception() << message("Index out of range."), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, u32_t index) {
    throw add_default_tags(out_of_range_exception() << message("Index {} out of range.") << value(index), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, u32_t index, u32_t max) {
    throw add_default_tags(out_of_range_exception() << message("Index {} out of range. Max {}") << value(index) << value(max), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0_skip(i32_t skip_frames, i32_t index) {
    throw add_default_tags(out_of_range_exception() << message("Index {} out of range.") << value(index), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0_skip(i32_t skip_frames, i32_t index, i32_t max) {
    throw add_default_tags(out_of_range_exception() << message("Index {} out of range. Max {}") << value(index) << value(max), skip_frames);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0(i32_t index) {
    throw_out_of_range_0_skip(1, index);
  }

  /// Throws [j::exceptions::out_of_range_exception] with the index specified.
  J_A(NORET) void throw_out_of_range_0(i32_t index, i32_t max) {
    throw_out_of_range_0_skip(1, index, max);
  }

  J_A(NORET) void fail_out_of_range() noexcept {
    throw_out_of_range(1);
  }

  J_A(NORET) void fail_out_of_range(i32_t skip_frames, u32_t index) noexcept {
    throw_out_of_range(skip_frames + 1, index);
  }

  J_A(NORET) void fail_out_of_range(i32_t skip_frames, u32_t index, u32_t max) noexcept {
    throw_out_of_range(skip_frames + 1, index, max);
  }

  J_A(NORET) void fail_out_of_range_0(i32_t index) noexcept {
    throw_out_of_range_0_skip(1, index);
  }

  J_A(NORET) void fail_out_of_range_0(i32_t index, i32_t max) noexcept {
    throw_out_of_range_0_skip(1, index, max);
  }

  /// Throws [j::exceptions::logic_error_exception] with the index specified.
  J_A(NORET) void throw_out_of_range(i32_t skip_frames, strings::const_string_view index) {
    throw add_default_tags(out_of_range_exception() << message("Index {} out of range.") << value(index), skip_frames);
  }

  J_A(NORET) void throw_out_of_range_any(mem::any && index, i32_t skip_frames) {
    throw add_default_tags(out_of_range_exception()
                           << message("Index {} out of range.")
                           << value(static_cast<mem::any &&>(index)), skip_frames);
  }

  J_A(NORET) void throw_logic_error_exception(const char * msg, i32_t skip_frames) {
    throw_logic_error_exception(logic_error_exception() << message(msg), skip_frames + 1);
  }

  J_A(NORET) void fail_logic_error_exception(const char * msg, i32_t skip_frames) noexcept {
    throw_logic_error_exception(logic_error_exception() << message(msg), skip_frames + 1);
  }

  J_A(NORET) void throw_logic_error_exception_fmt(const char * msg, u32_t count, mem::any * anys, i32_t skip_frames) {
    auto e = add_default_tags(logic_error_exception() << message(msg), skip_frames);
    for (u32_t i = 0; i < count; ++i) {
      e << value(static_cast<mem::any &&>(*anys++));
    }
    throw static_cast<logic_error_exception &&>(e);
  }

  J_A(NORET) void fail_logic_error_exception_fmt(const char * msg, u32_t count, mem::any * anys, i32_t skip_frames) noexcept {
    throw_logic_error_exception_fmt(msg, count, anys, skip_frames + 1);
  }

  J_A(NORET) void throw_exception(const char * msg, i32_t skip_frames) {
    throw add_default_tags(exception() << message(msg), skip_frames);
  }


  J_A(NORET) void fail_exception(const char * msg, i32_t skip_frames) noexcept {
    throw_exception(msg, skip_frames + 1);
  }

  J_A(NORET) void fail_todo(const char * todo, i32_t skip_frames) noexcept {
    throw add_default_tags(exception() << message("{#bright_red,bold}TODO:{/} {#bright_yellow}{}{/}") << value(todo), skip_frames);
  }

  J_A(NORET) void throw_exception_fmt(const char * msg, u32_t count, mem::any * anys, i32_t skip_frames) {
    auto e = add_default_tags(exception() << message(msg), skip_frames);
    for (u32_t i = 0; i < count; ++i) {
      e << value(static_cast<mem::any &&>(*anys++));
    }
    throw e;
  }

  J_A(NORET) void fail_exception_fmt(const char * msg, u32_t count, mem::any * anys, i32_t skip_frames) noexcept {
    throw_exception_fmt(msg, count, anys, skip_frames + 1);
  }

  J_A(NORET) void throw_not_null(const char * param, i32_t skip_frames) {
    throw add_default_tags(logic_error_exception() << message("{#bright_red,bold}{}{/} {#bright_yellow,bold}cannot be null.{/}")
                           << value(param), skip_frames);
  }

  J_A(NORET) void fail_not_null(const char * J_NOT_NULL param, const char * J_NOT_NULL at, i32_t skip_frames) noexcept {
    throw add_default_tags(logic_error_exception() << message("{#bright_red,bold}{}{/} {#bright_yellow,bold}cannot be null{/} in {#bright_magenta,bold}{}{/}")
                           << value(param) << value(at), skip_frames);
  }

  namespace detail {
    J_A(NORET) void throw_range_signed(
      const char * msg, i64_t left, i64_t val, i64_t right, i32_t skip_frames)
    {
      throw add_default_tags(logic_error_exception() << message("{#bright_red,bold}{} failed with{/} {#bright_yellow,bold}{} <= {} < {}{/}")
                             << value(msg) << value(left) << value(val) << value(right), skip_frames);
    }

    J_A(NORET) void fail_range_signed(
      const char * msg, i64_t left, i64_t val, i64_t right, i32_t skip_frames) noexcept
    {
      throw_range_signed(msg, left, val, right, skip_frames + 1);
    }

    J_A(NORET) void throw_range_unsigned(
      const char * msg, u64_t left, u64_t val, u64_t right, i32_t skip_frames)
    {
      throw add_default_tags(logic_error_exception() << message("{#bright_red,bold}{} failed with{/} {#bright_yellow,bold}{} <= {} < {}{/}")
                             << value(msg) << value(left) << value(val) << value(right), skip_frames);
    }

    J_A(NORET) void fail_range_unsigned(
      const char * msg, u64_t left, u64_t val, u64_t right, i32_t skip_frames) noexcept
    {
      throw_range_unsigned(msg, left, val, right, skip_frames + 1);
    }


    J_A(NORET) void throw_range_pointer(
      const char * msg, const void * left, const void * val, const void * right, i32_t skip_frames)
    {
      throw add_default_tags(logic_error_exception()
                             << message("{#bright_red,bold}{} failed with{/} {#bright_yellow,bold}{} <= {} < {}{/}")
                             << value(msg) << value(left) << value(val) << value(right), skip_frames);
    }

    J_A(NORET) void fail_range_pointer(
      const char * msg, const void * left, const void * val, const void * right, i32_t skip_frames) noexcept
    {
      throw_range_pointer(msg, left, val, right, skip_frames + 1);
    }
  }

  J_A(NORET) void fail_exception(exception && e, i32_t skip_frames) noexcept {
    throw_exception(static_cast<exception &&>(e), skip_frames + 1);
  }

  J_A(NORET) void fail_logic_error_exception(exception && e, i32_t skip_frames) noexcept {
    throw_logic_error_exception(static_cast<exception &&>(e), skip_frames + 1);
  }
  J_A(NORET) void fail_logic_error_exception(logic_error_exception && e, i32_t skip_frames) noexcept {
    throw_logic_error_exception(static_cast<logic_error_exception &&>(e), skip_frames + 1);
  }

#pragma clang diagnostic pop

  const tags::tag_definition<const char *> function_name{"Function"};
  const tags::tag_definition<strings::string> service_name{"Service"};
  const tags::tag_definition<strings::string> message{"Message"};
  const tags::tag_definition<strings::string> description{"Description"};
  const tags::tag_definition<int> err_errno{"Errno"};
  const tags::tag_definition<mem::any> value{"Value"};
  const tags::tag_definition<::j::backtrace::stored_backtrace> backtrace{"Backtrace"};
}
