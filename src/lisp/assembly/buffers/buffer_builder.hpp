#pragma once

#include "lisp/assembly/buffers/buffer.hpp"
#include "lisp/assembly/linking/reloc.hpp"
#include "mem/buffer_builder.hpp"
#include "containers/pile.hpp"
#include "containers/ptr_set.hpp"
#include "strings/string_view.hpp"

namespace j::lisp::assembly::inline buffers {
  /// Record for a backpatch, i.e. a placeholder for offset to a label.
  struct backpatch_record final {
    void * write_field;
    reloc_field_options opts;
    u8_t metadata_size;
    i32_t offset;
    backpatch_record * next;

    backpatch_record(void * J_NOT_NULL write_field,
                     reloc_field_options opts,
                     u8_t metadata_size,
                     i32_t offset,
                     backpatch_record * next) noexcept;
  };

  /// Record describing a label, i.e. a location in code or data.
  struct label_record final {
    u32_t offset:31 = 0U;
    u32_t is_valid:1 = 0U;
    i32_t label_size = 0U;
    backpatch_record * first_backpatch = nullptr;

    inline strings::const_string_view get_label() const noexcept {
      return { (const char*)(this + 1), label_size };
    }
  };

  class buffer_builder final {
  public:
    J_BOILERPLATE(buffer_builder, CTOR_DEL, MOVE_DEL, COPY_DEL)
    J_ALWAYS_INLINE explicit buffer_builder(j::mem::bump_pool * J_NOT_NULL temp_pool) noexcept
      : temp_pool(temp_pool)
    { }

    /// Make the current code in the builder into a buffer.
    ///
    /// \remarks Invalidates the builder.
    [[nodiscard]] buffer build();

    /// Pre-reserve sz bytes without actually allocating it.
    ///
    /// The bytes should be committed with `commit`.
    J_INLINE_GETTER_NONNULL char * reserve(u32_t sz) {
      return m_builder.reserve(sz);
    }

    /// Commit bytes reserved with `reserve`.
    J_ALWAYS_INLINE void commit(char * J_NOT_NULL end) noexcept {
      m_builder.commit(end);
    }

    J_ALWAYS_INLINE_NONNULL char * allocate(u32_t sz) {
      return m_builder.allocate(sz);
    }

    J_ALWAYS_INLINE_NONNULL void * append(const void * J_NOT_NULL src, u32_t sz) {
      return m_builder.append(src, sz);
    }

    template<Integral T>
    J_ALWAYS_INLINE_NONNULL void * append(T num) {
      return m_builder.append<T>(num);
    }

    template<typename Ptr>
    J_ALWAYS_INLINE_NONNULL void * append(const j::mem::basic_memory_region<Ptr> & region) {
      return m_builder.append(region);
    }

    J_ALWAYS_INLINE void align(u32_t a) {
      m_builder.align(a);
    }

    J_INLINE_GETTER u32_t offset() const noexcept {
      return m_builder.size();
    }

    /// Allocate a layer object, not at any position.
    [[nodiscard]] J_RETURNS_NONNULL label_record * allocate_label(const strings::const_string_view & label);

    u32_t append_buffer(const buffer & buf) noexcept;

    /// Binds label to the current address.
    void push_label(label_record * J_NOT_NULL label);

    void append_reloc(reloc_source source, reloc_field_options opts, i64_t addend);

    void write_reloc(void * J_NOT_NULL addr, reloc_source source, reloc_field_options opts, i64_t addend);
    void add_reloc(void * J_NOT_NULL addr, reloc_source source, reloc_field_options opts);
    void add_reloc(u32_t offset, reloc_source source, reloc_field_options opts);
    void add_reloc(reloc rel, u32_t base = 0U);

    void * append_backpatch_rel(label_record * J_NOT_NULL label, reloc_field_options opts, i64_t addend, u8_t metadata_size = 0U);

    j::mem::buffer_builder m_builder;
    j::mem::bump_pool * temp_pool = nullptr;
    containers::pile<reloc> m_relocs;
    friend class function_builder;
  };
}
