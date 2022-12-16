#pragma once

#include "meta/code_writer.hpp"
#include "strings/string.hpp"
#include "strings/formatting/formatted_sink.hpp"

namespace j::meta {
  struct doc_comment final {
    J_BOILERPLATE(doc_comment, CTOR_NE_ND, COPY_NE_ND)

    doc_comment(doc_comment && rhs) noexcept;

    doc_comment & operator=(doc_comment && rhs) noexcept;

    void apply_defaults(const doc_comment & rhs);

    void on_comment_line(strings::const_string_view text) noexcept;
    void clear() noexcept;

    J_A(NODISC,PURE) strings::const_string_view title(bool with_newline = false) const noexcept;
    J_A(NODISC,PURE) strings::const_string_view summary(bool with_newline = false) const noexcept;
    J_A(NODISC,PURE) strings::const_string_view body(bool with_newline = false) const noexcept;

    J_A(AI,NODISC) explicit inline operator bool() const noexcept {
      return title_end;
    }
    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return !title_end;
    }

    J_A(NODISC,AI) inline bool has_section() const noexcept {
      return section;
    }

    J_A(NODISC,AI) inline bool has_summary() const noexcept {
      return summary_end != title_end;
    }

    J_A(NODISC,PURE) bool has_body() const noexcept;

  private:
    strings::string comment{};
  public:
    doc_comment * section = nullptr;
  private:
    u32_t title_end = 0U;
    u32_t summary_end:31 = 0U;
    bool has_newline:1 = false;
  };

  enum comment_section_type : u32_t {
    section_none,
    section_doc_comment,
    section_custom,
    section_initial_custom,
    section_attr,
  };

  enum doc_comment_section : u32_t {
    doc_title,
    doc_after_title,
    doc_summary,
    doc_after_summary,
    doc_body,
    doc_after_body,
  };

  struct comment_section_key final {
    J_BOILERPLATE(comment_section_key, CTOR_CE, COPY_CE, EQ_CE)

    comment_section_type type:8 = section_none;
    u32_t id:24 = 0U;

    J_A(AI) constexpr inline explicit operator bool() const noexcept {
      return type != section_none;
    }

    J_A(AI) constexpr inline bool operator!() const noexcept {
      return type == section_none;
    }

    J_A(AI) constexpr inline explicit comment_section_key(comment_section_type type, u32_t id = 0U) noexcept
    : type(type),
      id(id)
    { }
  };

  J_A(ND) constexpr inline comment_section_key comment_section_none{};
  J_A(ND) constexpr inline comment_section_key comment_section_initial_custom{section_initial_custom};

  J_A(AI,NODISC) constexpr inline comment_section_key comment_section_custom(u32_t id) noexcept {
    return comment_section_key(section_custom, id);
  }

  struct doc_comment_section_render_opts final {
    strings::const_string_view fallback{};
    strings::const_string_view prefix{};
    strings::const_string_view suffix{};
    strings::const_string_view after{};
  };

  struct doc_comment_render_opts final {
    doc_comment_section_render_opts title;
    doc_comment_section_render_opts summary;
    doc_comment_section_render_opts body;
  };

  struct comment_writer final {
    J_A(AI) inline explicit comment_writer(code_writer & to) noexcept
      : to(to)
    { }

    J_A(AI) inline explicit comment_writer(code_writer & to, const code_writer_settings & settings) noexcept
      : to(to),
        settings(settings)
    { }

    void line_break() noexcept;

    inline void enter_section(comment_section_key section) noexcept {
      if (section) {
        current_section = section;
        to.is_between_sections = last_written_section && last_written_section != current_section;
      }
    }

    J_A(AI,ND) inline void set_last_written() noexcept {
      last_written_section = current_section;
      to.is_between_sections = false;
    }

    void write_line(strings::const_string_view str, comment_section_key section = comment_section_none) noexcept;

    void write_with_line_breaks(strings::const_string_view str, comment_section_key section = comment_section_none) noexcept;

    template<typename... Args>
    J_A(AI,ND) inline void write_line_formatted(strings::const_string_view format, Args && ... args) noexcept {
      auto g = to.enter(settings);
      if (to.write_line_formatted(format, static_cast<Args &&>(args)...)) {
        set_last_written();
      }
    }

    void write_doc_comment(const doc_comment & comment, doc_comment_render_opts = {}) noexcept;

    void write_doc_comment_field(const doc_comment & comment, doc_comment_section section, doc_comment_section_render_opts = {}) noexcept;

    void reset() noexcept;

    code_writer & to;
    code_writer_settings settings;
    comment_section_key current_section = comment_section_initial_custom;
    comment_section_key last_written_section{};
  };
}
