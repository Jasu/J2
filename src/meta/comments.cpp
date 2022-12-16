#include "comments.hpp"
#include "strings/styling/styled_sink.hpp"
#include "strings/formatting/pad.hpp"

namespace j::meta {
  namespace s = strings;

  doc_comment::doc_comment(doc_comment && rhs) noexcept
    : comment(static_cast<strings::string &&>(rhs.comment)),
      section(rhs.section),
      title_end(rhs.title_end),
      summary_end(rhs.summary_end),
      has_newline(rhs.has_newline)
  {
    rhs.title_end = rhs.summary_end = 0U;
    rhs.has_newline = false;
  }

  doc_comment & doc_comment::operator=(doc_comment && rhs) noexcept {
    if (&rhs != this) {
      [[likely]];
      comment = static_cast<strings::string &&>(rhs.comment);
      section = rhs.section;
      title_end = rhs.title_end;
      summary_end = rhs.summary_end;
      has_newline = rhs.has_newline;
      rhs.title_end = rhs.summary_end = 0U;
      rhs.has_newline = false;
    }
    return *this;
  }

  void doc_comment::apply_defaults(const doc_comment & rhs) {
    if (!rhs.title_end || has_body()) {
      return;
    }
    if (!title_end) {
      comment = rhs.comment;
      title_end = rhs.title_end;
      summary_end = rhs.summary_end;
      return;
    }

    if (!rhs.has_summary()) {
      return;
    }
    if (!has_summary()) {
      comment += (rhs.comment.without_prefix(rhs.title_end));
      summary_end = rhs.summary_end - rhs.title_end + title_end;
      return;
    }

    if (rhs.has_body()) {
      comment += rhs.body(true);
    }
  }

  void doc_comment::on_comment_line(s::const_string_view text) noexcept {
    const u32_t len = text.size();
    if (len) {
      if (summary_end == title_end) {
        summary_end = title_end + len + 1U;
        if (!title_end) {
          title_end = len + 1U;
        }
      } else if (has_newline && summary_end != comment.size()) {
        comment.push_back('\n');
      }
      comment += text;
      comment.push_back('\n');
    }
    has_newline = !len;
  }

  J_A(NODISC,PURE) bool doc_comment::has_body() const noexcept {
    return summary_end != comment.size();
  }

  void doc_comment::clear() noexcept {
    comment.clear();
    title_end = summary_end = 0U;
    section = nullptr;
    has_newline = false;
  }

  J_A(NODISC,PURE) s::const_string_view doc_comment::title(bool with_newline) const noexcept {
    return title_end ? comment.prefix(title_end - !with_newline) : s::const_string_view{};
  }

  J_A(NODISC,PURE) s::const_string_view doc_comment::summary(bool with_newline) const noexcept {
    return title_end == summary_end ? s::const_string_view{} : comment.slice(title_end, summary_end - title_end - !with_newline);
  }

  J_A(NODISC,PURE) s::const_string_view doc_comment::body(bool with_newline) const noexcept {
    const u32_t sz = comment.size();
    return summary_end == sz ? s::const_string_view{} : comment.slice(summary_end, sz - summary_end - !with_newline);
  }

  void comment_writer::line_break() noexcept {
    auto g = to.enter(settings);
    if (to.line_break()) {
      set_last_written();
    }
  }

  void comment_writer::write_doc_comment(const doc_comment & comment, doc_comment_render_opts opts) noexcept {
    write_doc_comment_field(comment, doc_title, opts.title);
    write_doc_comment_field(comment, doc_after_title, opts.title);
    write_doc_comment_field(comment, doc_summary, opts.summary);
    write_doc_comment_field(comment, doc_after_summary, opts.summary);
    write_doc_comment_field(comment, doc_body, opts.body);
    write_doc_comment_field(comment, doc_after_body, opts.body);
  }

  void comment_writer::write_doc_comment_field(const doc_comment & comment, doc_comment_section section, doc_comment_section_render_opts opts) noexcept {
    enter_section(comment_section_key(section_doc_comment, section));
    switch (section) {
    case doc_title:
      if (comment) {
        if (opts.prefix || opts.suffix) {
          write_line(opts.prefix + comment.title(false) + opts.suffix);
        } else {
          write_line(comment.title(false));
        }
      } else if (opts.fallback) {
        write_line(opts.fallback);
      }
      break;
    case doc_summary:
      if (comment.has_summary()) {
        if (opts.prefix || opts.suffix) {
          write_line(opts.prefix + comment.summary(false) + opts.suffix);
        } else {
          write_line(comment.summary(false));
        }
      } else if (opts.fallback) {
        write_line(opts.fallback);
      }
      break;
    case doc_body:
      if (comment.has_body()) {
        if (opts.prefix || opts.suffix) {
          write_with_line_breaks(opts.prefix + comment.body(false) + opts.suffix);
        } else {
          write_with_line_breaks(comment.body(false));
        }
      } else if (opts.fallback) {
        write_with_line_breaks(opts.fallback);
      }
      break;
    case doc_after_title:
    case doc_after_summary:
    case doc_after_body:
      write_with_line_breaks(opts.after);
      break;
    }
  }

  void comment_writer::write_line(strings::const_string_view str, comment_section_key section) noexcept {
    enter_section(section);
    auto g = to.enter(settings);
    if (to.write_line(str)) {
      set_last_written();
    }
  }

  void comment_writer::write_with_line_breaks(strings::const_string_view str, comment_section_key section) noexcept {
    if (!str) {
      return;
    }
    enter_section(section);
    auto g = to.enter(settings);
    if (to.write_block_with_line_breaks(str)) {
      set_last_written();
    }
  }

  void comment_writer::reset() noexcept {
    current_section = comment_section_initial_custom;
    last_written_section = comment_section_none;
  }
}
