// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/exprs/expr_type.hpp"
#include "lisp/air/debug/debug_formatters.hpp"
#include "lisp/values/static_objects.hpp"
#include "strings/styling/default_styles.hpp"
#include "strings/formatters/debug_enum_formatter.hpp"

namespace j::lisp::air {
  namespace s = j::strings;
  namespace f = s::formatters;
  namespace st = s::styles;
  namespace {
    constinit const static_lisp_str n_nop("nop");
    constinit const static_lisp_str n_ld_const("ld_const");
    constinit const static_lisp_str n_fn_arg("fn_arg");
    constinit const static_lisp_str n_fn_rest_arg("fn_rest_arg");
    constinit const static_lisp_str n_fn_sc_arg("fn_sc_arg");
    constinit const static_lisp_str n_fn_body("fn_body");
    constinit const static_lisp_str n_progn("progn");
    constinit const static_lisp_str n_prog1("prog1");
    constinit const static_lisp_str n_let("let");
    constinit const static_lisp_str n_b_if("b_if");
    constinit const static_lisp_str n_do_until("do_until");
    constinit const static_lisp_str n_lex_rd("lex_rd");
    constinit const static_lisp_str n_lex_rd_bound("lex_rd_bound");
    constinit const static_lisp_str n_lex_wr("lex_wr");
    constinit const static_lisp_str n_lex_wr_bound("lex_wr_bound");
    constinit const static_lisp_str n_sym_val_rd("sym_val_rd");
    constinit const static_lisp_str n_sym_val_wr("sym_val_wr");
    constinit const static_lisp_str n_type_assert("type_assert");
    constinit const static_lisp_str n_dyn_type_assert("dyn_type_assert");
    constinit const static_lisp_str n_ineg("ineg");
    constinit const static_lisp_str n_iadd("iadd");
    constinit const static_lisp_str n_isub("isub");
    constinit const static_lisp_str n_imul("imul");
    constinit const static_lisp_str n_idiv("idiv");
    constinit const static_lisp_str n_lor("lor");
    constinit const static_lisp_str n_land("land");
    constinit const static_lisp_str n_lnot("lnot");
    constinit const static_lisp_str n_to_bool("to_bool");
    constinit const static_lisp_str n_eq("eq");
    constinit const static_lisp_str n_neq("neq");
    constinit const static_lisp_str n_tag("tag");
    constinit const static_lisp_str n_untag("untag");
    constinit const static_lisp_str n_as_range("as_range");
    constinit const static_lisp_str n_as_vec("as_vec");
    constinit const static_lisp_str n_call("call");
    constinit const static_lisp_str n_full_call("full_call");
    constinit const static_lisp_str n_vec("vec");
    constinit const static_lisp_str n_make_vec("make_vec");
    constinit const static_lisp_str n_vec_append("vec_append");
    constinit const static_lisp_str n_vec_build("vec_build");
    constinit const static_lisp_str n_range_length("range_length");
    constinit const static_lisp_str n_range_get("range_get");
    constinit const static_lisp_str n_range_set("range_set");
    constinit const static_lisp_str n_range_copy("range_copy");
    constinit const static_lisp_str n_act_rec_alloc("act_rec_alloc");
    constinit const static_lisp_str n_act_rec_wr("act_rec_wr");
    constinit const static_lisp_str n_act_rec_rd("act_rec_rd");
    constinit const static_lisp_str n_act_rec_parent("act_rec_parent");
    constinit const static_lisp_str n_closure("closure");
    constinit const static_lisp_str n_none("none");
  }

  constinit strings::const_string_view expr_type_names[num_expr_types_with_none_v]{
    "nop",           "ld_const",    "fn_arg",          "fn_rest_arg",
    "fn_sc_arg",     "fn_body",     "progn",           "prog1",
    "let",           "b_if",        "do_until",        "lex_rd",
    "lex_rd_bound",  "lex_wr",      "lex_wr_bound",    "sym_val_rd",
    "sym_val_wr",    "type_assert", "dyn_type_assert", "ineg",
    "iadd",          "isub",        "imul",            "idiv",
    "lor",           "land",        "lnot",            "to_bool",
    "eq",            "neq",         "tag",             "untag",
    "as_range",      "as_vec",      "call",            "full_call",
    "vec",           "make_vec",    "vec_append",      "vec_build",
    "range_length",  "range_get",   "range_set",       "range_copy",
    "act_rec_alloc", "act_rec_wr",  "act_rec_rd",      "act_rec_parent",
    "closure",       "none",
  };

  constinit strings::const_string_view expr_type_short_names[num_expr_types_with_none_v]{
    "nop",     "const",  "arg",     "rest",
    "sc-arg",  "body",   "progn",   "prog1",
    "let",     "if",     "until",   "lex-rd",
    "bnd-rd",  "lex-wr", "bnd-wr",  "sym-rd",
    "sym-wr",  "type!",  "dtype!",  "ineg",
    "iadd",    "isub",   "imul",    "idiv",
    "or",      "and",    "not",     "to-bool",
    "eq",      "neq",    "tag",     "untag",
    "as-rng",  "as-vec", "call",    "f-call",
    "vec",     "mk-vec", "append",  "quasi",
    "length",  "at",     "rng-set", "rng-cp",
    "mk-act",  "act-wr", "act-rd",  "act-par",
    "closure", "none",
  };

  constinit const lisp_str * expr_type_lisp_names[num_expr_types_with_none_v]{
    &n_nop.value,           &n_ld_const.value,    &n_fn_arg.value,          &n_fn_rest_arg.value,
    &n_fn_sc_arg.value,     &n_fn_body.value,     &n_progn.value,           &n_prog1.value,
    &n_let.value,           &n_b_if.value,        &n_do_until.value,        &n_lex_rd.value,
    &n_lex_rd_bound.value,  &n_lex_wr.value,      &n_lex_wr_bound.value,    &n_sym_val_rd.value,
    &n_sym_val_wr.value,    &n_type_assert.value, &n_dyn_type_assert.value, &n_ineg.value,
    &n_iadd.value,          &n_isub.value,        &n_imul.value,            &n_idiv.value,
    &n_lor.value,           &n_land.value,        &n_lnot.value,            &n_to_bool.value,
    &n_eq.value,            &n_neq.value,         &n_tag.value,             &n_untag.value,
    &n_as_range.value,      &n_as_vec.value,      &n_call.value,            &n_full_call.value,
    &n_vec.value,           &n_make_vec.value,    &n_vec_append.value,      &n_vec_build.value,
    &n_range_length.value,  &n_range_get.value,   &n_range_set.value,       &n_range_copy.value,
    &n_act_rec_alloc.value, &n_act_rec_wr.value,  &n_act_rec_rd.value,      &n_act_rec_parent.value,
    &n_closure.value,       &n_none.value,
  };
  namespace {
    J_A(ND,NODESTROY) const f::debug_enum_entry expr_entries[]{
      [(u8_t)expr_type::nop]             = {"nop",     st::gray           },
      [(u8_t)expr_type::ld_const]        = {"const",   st::light_gray     },
      [(u8_t)expr_type::fn_arg]          = {"arg",     st::green          },
      [(u8_t)expr_type::fn_rest_arg]     = {"rest",    st::bright_green   },
      [(u8_t)expr_type::fn_sc_arg]       = {"sc-arg",  st::bright_green   },
      [(u8_t)expr_type::fn_body]         = {"body",    st::cyan           },
      [(u8_t)expr_type::progn]           = {"progn",   st::light_gray     },
      [(u8_t)expr_type::prog1]           = {"prog1",   st::light_gray     },
      [(u8_t)expr_type::let]             = {"let",     st::bright_green   },
      [(u8_t)expr_type::b_if]            = {"if",      st::magenta        },
      [(u8_t)expr_type::do_until]        = {"until",   st::bright_magenta },
      [(u8_t)expr_type::lex_rd]          = {"lex-rd",  st::green          },
      [(u8_t)expr_type::lex_rd_bound]    = {"bnd-rd",  st::bright_green   },
      [(u8_t)expr_type::lex_wr]          = {"lex-wr",  st::bright_yellow  },
      [(u8_t)expr_type::lex_wr_bound]    = {"bnd-wr",  st::bright_yellow  },
      [(u8_t)expr_type::sym_val_rd]      = {"sym-rd",  st::bright_green   },
      [(u8_t)expr_type::sym_val_wr]      = {"sym-wr",  st::bright_yellow  },
      [(u8_t)expr_type::type_assert]     = {"type!",   st::bright_red     },
      [(u8_t)expr_type::dyn_type_assert] = {"dtype!",  st::bright_red     },
      [(u8_t)expr_type::ineg]            = {"ineg",    st::bright_cyan    },
      [(u8_t)expr_type::iadd]            = {"iadd",    st::bright_cyan    },
      [(u8_t)expr_type::isub]            = {"isub",    st::bright_cyan    },
      [(u8_t)expr_type::imul]            = {"imul",    st::bright_cyan    },
      [(u8_t)expr_type::idiv]            = {"idiv",    st::bright_cyan    },
      [(u8_t)expr_type::lor]             = {"or",      st::bright_blue    },
      [(u8_t)expr_type::land]            = {"and",     st::bright_blue    },
      [(u8_t)expr_type::lnot]            = {"not",     st::bright_blue    },
      [(u8_t)expr_type::to_bool]         = {"to-bool", st::bright_blue    },
      [(u8_t)expr_type::eq]              = {"eq",      st::yellow         },
      [(u8_t)expr_type::neq]             = {"neq",     st::yellow         },
      [(u8_t)expr_type::tag]             = {"tag",     st::green          },
      [(u8_t)expr_type::untag]           = {"untag",   st::yellow         },
      [(u8_t)expr_type::as_range]        = {"as-rng",  st::green          },
      [(u8_t)expr_type::as_vec]          = {"as-vec",  st::green          },
      [(u8_t)expr_type::call]            = {"call",    st::bright_red     },
      [(u8_t)expr_type::full_call]       = {"f-call",  st::bright_red     },
      [(u8_t)expr_type::vec]             = {"vec",     st::bright_magenta },
      [(u8_t)expr_type::make_vec]        = {"mk-vec",  st::bright_magenta },
      [(u8_t)expr_type::vec_append]      = {"append",  st::bright_magenta },
      [(u8_t)expr_type::vec_build]       = {"quasi",   st::bright_magenta },
      [(u8_t)expr_type::range_length]    = {"length",  st::magenta        },
      [(u8_t)expr_type::range_get]       = {"at",      st::magenta        },
      [(u8_t)expr_type::range_set]       = {"rng-set", st::bright_red     },
      [(u8_t)expr_type::range_copy]      = {"rng-cp",  st::magenta        },
      [(u8_t)expr_type::act_rec_alloc]   = {"mk-act",  st::bright_red     },
      [(u8_t)expr_type::act_rec_wr]      = {"act-wr",  st::bright_red     },
      [(u8_t)expr_type::act_rec_rd]      = {"act-rd",  st::bright_green   },
      [(u8_t)expr_type::act_rec_parent]  = {"act-par", st::bright_green   },
      [(u8_t)expr_type::closure]         = {"closure", st::bright_cyan    },
      [(u8_t)expr_type::none]            = {"none",    st::error          },
    };

    J_A(ND,NODESTROY) const f::debug_enum_formatter<expr_type> expr_type_fmt(expr_entries);
  }

  const strings::formatter_known_length<expr_type> & g_expr_type_formatter = expr_type_fmt;
}
