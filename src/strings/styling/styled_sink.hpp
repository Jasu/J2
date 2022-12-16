#pragma once

#include "streams/buffered_sink.hpp"
#include "strings/string_view.hpp"

namespace j::strings::inline styling {
  struct style;
  struct styled_string;

  enum class replace_style_tag { v };
  inline constexpr replace_style_tag replace_style{replace_style_tag::v};

  class styled_sink : public streams::buffered_sink {
  public:
    /// Set the current output style.
    virtual void set_style(const style & style) = 0;
    virtual style get_style() const = 0;
    virtual u32_t write_styled(const styled_string & str);
    virtual u32_t write_styled(const style & style, const styled_string & str);
    virtual u32_t write_styled(const style & style, const char * J_NOT_NULL from, u32_t num_bytes);
    virtual u32_t write_styled(replace_style_tag, const style & style, const char * J_NOT_NULL from, u32_t num_bytes) = 0;

    virtual u32_t get_column() noexcept = 0;
    virtual void reset_column() noexcept = 0;
    virtual void pad_to_column(u32_t i) = 0;

    u32_t write_styled(const style & style, strings::const_string_view str);
    u32_t write_styled(const style & style, const char * J_NOT_NULL str);
    u32_t write_styled(replace_style_tag t, const style & style, strings::const_string_view str);
    u32_t write_styled(replace_style_tag t, const style & style, const char * J_NOT_NULL str);
  };
}
