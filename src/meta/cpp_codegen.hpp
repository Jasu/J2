#pragma once

#include "meta/errors.hpp"
#include "containers/vector.hpp"
#include "hzd/crc32.hpp"
#include "strings/string.hpp"
#include "meta/code_writer.hpp"
#include "meta/attr_value.hpp"

namespace j::meta {
  struct value;
  struct expr;
  struct expr_scope;

  enum tpl_stmt_type : u8_t {
    stmt_foreach = 0U,
    stmt_let,
    stmt_set,
    stmt_if,

    stmt_with_writer_settings,
    stmt_indent,
    stmt_indent_inner,

    stmt_max = stmt_indent_inner,
  };
  constexpr inline u8_t num_stmt_types_v J_A(ND) = stmt_max + 1U;


  enum tpl_part_type : u8_t {
    tpl_part_none = 0U,

    tpl_verbatim,
    tpl_newline,

    tpl_expr,

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wduplicate-enum"
    tpl_min_stmt,
#define J_STMT(N) tpl_##N = tpl_min_stmt + stmt_##N
    J_STMT(foreach),
    J_STMT(let),
    J_STMT(set),
    J_STMT(if),
    J_STMT(with_writer_settings),
    J_STMT(indent),
    J_STMT(indent_inner),
#undef J_STMT
#pragma clang diagnostic pop

    tpl_part_max = tpl_indent_inner,
  };

  enum class lbr_t : u8_t {
    normal,
    none,
    soft,
    hard,
  };
  J_A(ND) constexpr inline lbr_t default_break = lbr_t::normal;
  J_A(ND) constexpr inline lbr_t no_break = lbr_t::none;
  J_A(ND) constexpr inline lbr_t hard_break = lbr_t::hard;
  J_A(ND) constexpr inline lbr_t soft_break = lbr_t::soft;

  enum class stmt_if_t : u8_t { v };
  enum class stmt_name_expr_t : u8_t {
    foreach = tpl_foreach,
    let = tpl_let,
    set = tpl_set,
  };
  J_A(ND) constexpr inline auto stmt_if_v = stmt_if_t::v;
  J_A(ND) constexpr inline auto stmt_foreach_v = stmt_name_expr_t::foreach;
  J_A(ND) constexpr inline auto stmt_let_v = stmt_name_expr_t::let;
  J_A(ND) constexpr inline auto stmt_set_v = stmt_name_expr_t::set;

  J_A(AI,ND) constexpr inline tpl_stmt_type stmt_type(tpl_part_type t) noexcept {
    J_ASSERT(t >= tpl_min_stmt);
    return (tpl_stmt_type)(t - tpl_min_stmt);
  }

  struct codegen_template;

  struct tpl_part final {
    J_A(AI,ND) inline tpl_part() noexcept : subst_expr{nullptr} { }
    J_A(ND) inline tpl_part(tpl_part && rhs) noexcept {
      j::memcpy(this, &rhs, sizeof(tpl_part));
      rhs.type = tpl_part_none;
    }

    tpl_part & operator=(tpl_part && rhs) noexcept;
    tpl_part(const tpl_part & rhs);
    tpl_part & operator=(const tpl_part & rhs);
    ~tpl_part();

    explicit tpl_part(strings::const_string_view text, lbr_t break_before, lbr_t break_after) noexcept;
    explicit tpl_part(tpl_part_type type) noexcept;
    explicit tpl_part(expr * J_NOT_NULL e, lbr_t break_before, lbr_t break_after) noexcept;

    tpl_part(tpl_part_type type, codegen_template && tpl, i8_t indent) noexcept;

    tpl_part(stmt_name_expr_t, strings::string && name, expr * J_NOT_NULL e) noexcept;
    tpl_part(stmt_name_expr_t, strings::string && name, strings::string && index_name, expr * J_NOT_NULL e) noexcept;
    tpl_part(stmt_if_t, expr * J_NOT_NULL e) noexcept;
    tpl_part(codegen_template && tpl, const code_writer_settings & settings) noexcept;

    void clear() noexcept;

    tpl_part_type type = tpl_part_none;
    lbr_t break_before = default_break;
    lbr_t break_after = default_break;
    union {
      expr * subst_expr;
      strings::string content;
      struct {
        codegen_template * body;
        union {
          struct {
            expr * e;
            codegen_template * else_body;
          } ifelse;
          struct {
            expr * e;
            strings::string name;
            strings::string index_name;
          } let;
          code_writer_settings writer_settings;
          i8_t indent;
        };
      } sub;
    };
    source_location loc{};
    [[nodiscard]] bool operator==(const tpl_part & rhs) const noexcept;
    [[nodiscard]] bool operator<(const tpl_part & rhs) const noexcept;
  };

  struct tpl_part_hash final {
    J_A(FLATTEN,NODISC) u32_t operator()(const tpl_part & part) const noexcept;
  };

  struct codegen_template final {
    vector<tpl_part> parts;
    J_A(ND) codegen_template() noexcept = default;
    J_A(ND) codegen_template(codegen_template && rhs) noexcept = default;
    J_A(ND) codegen_template & operator=(codegen_template && rhs) noexcept = default;

    codegen_template(const codegen_template & rhs) noexcept;
    explicit codegen_template(strings::const_string_view str, lbr_t break_before = no_break, lbr_t break_after = no_break) noexcept;
    explicit codegen_template(strings::string && str, lbr_t break_before = no_break, lbr_t break_after = no_break) noexcept;
    J_A(AI,ND) inline explicit codegen_template(const char * J_NOT_NULL str, lbr_t break_before = no_break, lbr_t break_after = no_break) noexcept
      : codegen_template(strings::const_string_view(str), break_before, break_after)
    { }

    codegen_template & operator=(const codegen_template & rhs) noexcept;

    ~codegen_template();

    void rtrim(lbr_t br) noexcept;

    tpl_part & push_back(const tpl_part & part);
    tpl_part & push_back(expr * J_NOT_NULL epr, lbr_t break_before = no_break, lbr_t break_after = no_break);
    void push_back(strings::const_string_view verbatim, lbr_t break_before = no_break, lbr_t break_after = no_break);
    void push_back_newline();
    tpl_part & push_back_stmt(tpl_part_type type, i8_t indent);
    tpl_part & push_back_stmt(const code_writer_settings & settings);
    tpl_part & push_back_stmt(stmt_name_expr_t, strings::string && name, expr * J_NOT_NULL e) noexcept;
    tpl_part & push_back_stmt(stmt_name_expr_t, strings::string && name, strings::string && idx_name, expr * J_NOT_NULL e) noexcept;
    tpl_part & push_back_stmt(stmt_name_expr_t, strings::const_string_view name, expr * J_NOT_NULL e) noexcept;
    tpl_part & push_back_stmt(stmt_name_expr_t, strings::const_string_view name, strings::const_string_view idx_name, expr * J_NOT_NULL e) noexcept;
    tpl_part & push_back_stmt(stmt_if_t, expr * J_NOT_NULL e) noexcept;

    void expand(code_writer & to, expr_scope & ctx) const;
    void expand_value(code_writer & to, expr_scope & ctx, const attr_value & value) const;

    J_A(AI,RNN,NODISC) inline const tpl_part * begin() const noexcept {
      return parts.begin();
    }
    J_A(AI,RNN,NODISC) inline const tpl_part * end() const noexcept {
      return parts.end();
    }

    J_A(AI,NODISC) inline u32_t size() const noexcept {
      return parts.size();
    }
    J_A(AI,NODISC) inline explicit operator bool() const noexcept {
      return (bool)parts;
    }
    J_A(AI,NODISC) inline bool operator!() const noexcept {
      return !(bool)parts;
    }

    [[nodiscard]] bool operator==(const codegen_template & rhs) const noexcept;
    [[nodiscard]] bool operator<(const codegen_template & rhs) const noexcept;
  };

  struct tpl_hash final {
    J_A(FLATTEN,NI,NODISC) u32_t operator()(const codegen_template & tpl) const noexcept {
      u32_t result = 17U + tpl.size();
      for (auto & part : tpl) {
        result = crc32(result, (tpl_part_hash{})(part));
      }
      return result;
    }
  };
}
