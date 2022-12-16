#pragma once

#include "files/memory_mapping.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::inline values {
  struct lisp_str;
}

namespace j::lisp::sources {
  struct include_dir;

  enum class source_type : u32_t {
    /// Invalid value.
    none,
    /// The source is a physical file.
    file,
    /// The source is a static string, provided from C++.
    static_string,
    /// The source is a dynamic string, e.g. from REPL or a test case.
    dynamic_string,
  };

  struct source {
    /// Type of the source.
    source_type type:2 = source_type::none;
    /// Index of the source. For dynamic strings, this is unique per source type, not per source.
    u32_t index:12 = 0U;
    /// Index of the include directory, for file sources.
    u32_t include_dir_index:10 = 0U;
    /// "Reference count" for calls to `open()`.
    u32_t open_count:8 = 0U;

    /// Name of the source.
    ///
    /// For files, this is the path relative to the include directory.
    /// For static strings, this is an unique name of the source.
    /// For dynamic strings, this is the name of the source type, e.g. "REPL".
    lisp_str * name = nullptr;

    source(source_type type, u32_t index, u32_t include_dir_index, lisp_str * J_NOT_NULL name) noexcept;

    J_INLINE_GETTER constexpr explicit operator bool() const noexcept {
      return type != source_type::none;
    }

    J_INLINE_GETTER constexpr bool operator!() const noexcept {
      return type == source_type::none;
    }

    J_INLINE_GETTER constexpr bool empty() const noexcept {
      return type == source_type::none;
    }

    strings::const_string_view name_str() const noexcept;

    virtual ~source();

    J_ALWAYS_INLINE void open() {
      if (!open_count++) {
        do_open();
      }
    }

    J_ALWAYS_INLINE void close() noexcept {
      J_ASSERT(open_count);
      if (!--open_count) {
        do_close();
      }
    }

    virtual strings::const_string_view get_source() = 0;
  protected:
    virtual void do_open() = 0;
    virtual void do_close() noexcept = 0;
  };

  struct file_source final : source {
    file_source(u16_t index,
                include_dir * J_NOT_NULL dir,
                lisp_str * J_NOT_NULL path);


    strings::const_string_view get_source() override;

    include_dir * include_dir = nullptr;
    files::memory_mapping mapping;

  protected:
    void do_open() noexcept override;
    void do_close() noexcept override;
  };

  struct string_source final : source {
    strings::const_string_view src;

    string_source(bool is_dynamic,
                  u16_t index,
                  lisp_str * J_NOT_NULL name,
                  strings::const_string_view src) noexcept;

    strings::const_string_view get_source() override;

  protected:
    void do_open() override;
    void do_close() noexcept override;
  };
}
