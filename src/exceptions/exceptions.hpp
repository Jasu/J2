#pragma once

#include "exceptions/assert.hpp"
#include "tags/tag.hpp"
#include "tags/tag_container.hpp"
#include "strings/string.hpp"
#include "hzd/type_traits.hpp"

namespace j::exceptions {
  struct tag_guard final {
    J_BOILERPLATE(tag_guard, MOVE_DEL)
    explicit tag_guard(tags::tag && tag) noexcept;
    ~tag_guard();

    tags::tag tag;
    tag_guard * prev;
  };

  struct exception : public tags::tag_container {
  public:
    J_BOILERPLATE(exception, CTOR_NE_ND, COPY_DEL, MOVE_NE_ND)
    virtual ~exception();
  };

  struct logic_error_exception : public exception {
    logic_error_exception() noexcept;
  };
  struct out_of_range_exception : public logic_error_exception {
    out_of_range_exception() noexcept;
  };
  struct invalid_argument_exception : public logic_error_exception {
    invalid_argument_exception() noexcept;
  };

  struct not_implemented_exception : public logic_error_exception {
    not_implemented_exception() noexcept;
  };

  struct bad_alloc_exception : public exception {
    bad_alloc_exception() noexcept;
  };

  struct system_error_exception : public exception {
    system_error_exception() noexcept;
  };

  extern const tags::tag_definition<const char *> function_name;

  extern const tags::tag_definition<strings::string> message;
  extern const tags::tag_definition<mem::any> value;

  extern const tags::tag_definition<strings::string> service_name;
  extern const tags::tag_definition<strings::string> description;
  extern const tags::tag_definition<int> err_errno;

  void add_default_exception_tags(exception & e, i32_t skip_frames = 0) noexcept;

  template<typename E>
  J_A(AI,ND) inline E && add_default_tags(E && e, i32_t skip_frames = 0) noexcept {
    add_default_exception_tags(e, skip_frames);
    return static_cast<E &&>(e);
  }

  template<typename E>
  [[noreturn]] void throw_exception(E && e, i32_t skip_frames = 0) {
    add_default_exception_tags(e, skip_frames + 1);
    throw static_cast<E &&>(e);
  }
  template<typename E>
  [[noreturn]] void throw_logic_error_exception(E && e, i32_t skip_frames = 0) {
    add_default_exception_tags(e, skip_frames + 1);
    throw static_cast<E &&>(e);
  }
  [[noreturn]] void fail_exception(exception && e, i32_t skip_frames = 0) noexcept;
  [[noreturn]] void fail_logic_error_exception(exception && e, i32_t skip_frames = 0) noexcept;
  [[noreturn]] void fail_logic_error_exception(logic_error_exception && e, i32_t skip_frames = 0) noexcept;

  void handle_terminate();
}

#define J_SCOPE_INFO(...) j::exceptions::tag_guard J_UNIQUE(__jtg)(__VA_ARGS__)
