#if __has_include("lisp/air/exprs/expr_size_data.gen.hpp")
#include "lisp/air/exprs/expr_size_data.gen.hpp"
#else
#warning "Missing 'expr_size_data.gen.hpp'. Run 'j air-meta --generate expr-size-data'."
#include "lisp/air/exprs/expr_info.hpp"
namespace j::lisp::air::exprs {
  constinit const expr_size_info   expr_sizes_v[num_expr_types_v]{};
}
#endif
