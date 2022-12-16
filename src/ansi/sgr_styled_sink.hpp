#pragma once

#include "strings/styling/styled_sink.hpp"
#include "mem/shared_ptr_fwd.hpp"
#include "strings/styling/style.hpp"

namespace j::ansi {
  class sgr_styled_sink final : public strings::styling::styled_sink {
  public:
    sgr_styled_sink(mem::shared_ptr<streams::sink> sink) noexcept;

    u32_t write(const char * J_NOT_NULL from, u32_t num_bytes) noexcept override;

    bool is_tty() const override;
    J_RETURNS_NONNULL char * get_write_buffer(u32_t num_bytes) override;
    void flush() override;

    void set_style(const strings::style & style) override;
    strings::style get_style() const override;
    u32_t write_styled(strings::styling::replace_style_tag,
                       const strings::style & style,
                       const char * J_NOT_NULL from, u32_t num_bytes) override;

    sgr_styled_sink(const sgr_styled_sink & rhs) = delete;
    sgr_styled_sink(sgr_styled_sink && rhs) = delete;
    sgr_styled_sink & operator=(const sgr_styled_sink & rhs) = delete;
    sgr_styled_sink & operator=(sgr_styled_sink && rhs) noexcept = delete;

    u32_t get_column() noexcept override;
    void reset_column() noexcept override;
    void pad_to_column(u32_t i) override;

    ~sgr_styled_sink();

    u32_t column = 0U;
  private:
    static inline constexpr u32_t buffer_size_v = 256UL;
    char * m_write_ptr;
    mem::shared_ptr<streams::sink> m_sink;
    strings::style m_current_style;
    mutable bool m_has_is_tty = false;
    mutable bool m_is_tty = false;
    char m_buffer[buffer_size_v];

    J_RETURNS_NONNULL const char * buffer_end() const noexcept;
    u32_t buffer_free() const noexcept;
  };
}
