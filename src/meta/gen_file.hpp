#pragma once

#include "meta/expr_scope.hpp"
#include "strings/string.hpp"
#include "meta/cpp_codegen.hpp"
#include "meta/node.hpp"

namespace j::strings::inline formatting {
  class formatted_sink;
}
namespace j::files::inline paths {
  class path;
}
namespace j::meta {
  struct module;

  struct generated_file;

  struct generated_file final {
    strings::string suffix{};
    strings::string path{};

    codegen_template tpl{};
  };

  struct generated_file_set final : node {
    J_BOILERPLATE(generated_file_set, CTOR_NE_ND)

    generated_file files[2U]{};
    u8_t num_files = 0;

    generated_file_set(strings::const_string_view name, doc_comment && comment) noexcept;

    J_A(AI,RNN,NODISC) inline const generated_file * begin() const noexcept {
      return files;
    }

    J_A(AI,RNN,NODISC) inline const generated_file * end() const noexcept {
      return files + num_files;
    }

    void generate(module * J_NOT_NULL mod, strings::const_string_view suffix) const;
    void generate(module * J_NOT_NULL mod, strings::formatted_sink &, strings::const_string_view suffix) const;
    void generate(module * J_NOT_NULL mod, const files::path & to, strings::const_string_view suffix) const;

    expr_scope_init scope_init{};
  };
}
