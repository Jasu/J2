#pragma once

#include "lisp/assembly/linking/reloc_type.hpp"
#include "lisp/common/id.hpp"
#include "lisp/functions/calling_convention.hpp"
#include "containers/trivial_array_fwd.hpp"

namespace j::lisp::assembly::inline linking {
  /// Specification of a relocation field - field type, shift.
  ///
  /// Does not include the actual value to be relocated. That is in the `reloc` object.
  struct reloc_field_options final {
    reloc_type type:4 = reloc_type::none;
    /// Shift to add to the relocation input.
    i8_t shift:4 = 0;
  };

  enum class reloc_source_type : u8_t {
    /// Invalid relocation source.
    none = 0,
    /// Address of a function constant.
    constant_addr,
    /// Address of another function.
    fn_addr,
    /// Address of a variable.
    var_addr,
    /// Value of a variable.
    var_value_tagged,
    /// Value of a variable.
    var_value_untagged,
  };

  struct reloc_source final {
    bool operator==(reloc_source rhs) const noexcept {
      if (source_type != rhs.source_type) {
        return false;
      }
      switch (source_type) {
      case reloc_source_type::none: return true;
      case reloc_source_type::constant_addr:
        return constant_offset == rhs.constant_offset;
      case reloc_source_type::var_value_untagged:
      case reloc_source_type::var_value_tagged:
      case reloc_source_type::fn_addr:
        if (cc != rhs.cc) {
          return false;
        }
        [[fallthrough]];
      case reloc_source_type::var_addr:
        return source_id == rhs.source_id;
      }
    }

    /// Type of the relocation source.
    reloc_source_type source_type = reloc_source_type::none;
    lisp::functions::calling_convention cc = lisp::functions::calling_convention::none;
    union {
      /// ID of the source for all reloc types but `constant_addr`.
      id source_id = {};
      u32_t constant_offset;
    };
  };

  /// A relocation record.
  struct reloc final {
    /// Offset of the relocation field.
    u32_t offset:18 = 0;
    /// Type of the relocation source.
    reloc_source_type source_type:3 = reloc_source_type::none;
    lisp::functions::calling_convention cc:3 = lisp::functions::calling_convention::none;
    /// Reocation field options.
    reloc_field_options opts;
    union {
      /// ID of the source for all reloc types but `constant_addr`.
      id source_id = {};
      u32_t constant_offset;
    };

    J_ALWAYS_INLINE void set_base(u32_t base) noexcept {
      J_ASSERT(base + this->offset < (1U << 18));
      this->offset += base;
    }

    bool operator==(const reloc & rhs) const noexcept {
      return offset == rhs.offset
        && source_type == rhs.source_type && cc == rhs.cc && opts.type == rhs.opts.type && opts.shift == rhs.opts.shift && constant_offset == rhs.constant_offset;
    }
  };

  /// Applies relocation to write_field.
  ///
  /// \param write_field Position to write the relocation to.
  /// \param final_field_pos Address of the written field during execution.
  /// \param opts Type of the relocation field.
  /// \param value Target to write to the relocation target.
  void apply_reloc(
    void * J_NOT_NULL write_field,
    const void * J_NOT_NULL final_field_pos,
    reloc_field_options opts,
    const void * J_NOT_NULL value) noexcept;

  /// Applies relocation to write_field.
  ///
  /// \param write_field Position to write the relocation to.
  /// \param opts Type of the relocation field.
  /// \param value Target to write to the relocation target.
  void apply_reloc(void * J_NOT_NULL write_field,
                   reloc_field_options opts,
                   const void * J_NOT_NULL value) noexcept;

  /// Applies a relative relocatoin to write_field.
  ///
  /// \param write_field Position to write the relocation to.
  /// \param opts Type of the relocation field.
  /// \param offset Offset from the start of the field to target.
  void apply_reloc_rel(
    void * J_NOT_NULL write_field,
    reloc_field_options opts,
    i64_t offset) noexcept;
}

J_DECLARE_EXTERN_TRIVIAL_ARRAY(j::lisp::assembly::linking::reloc);

namespace j::lisp::assembly::inline linking {
  using relocs_t = trivial_array<j::lisp::assembly::linking::reloc>;
}
