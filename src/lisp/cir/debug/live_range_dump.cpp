#include "lisp/cir/debug/live_range_dump.hpp"
#include "lisp/cir/ssa/bb.hpp"
#include "logging/global.hpp"
#include "lisp/cir/ssa/cir_ssa.hpp"
#include "lisp/cir/liveness/live_range.hpp"
#include "containers/vector.hpp"
#include "containers/pair.hpp"
#include "files/ofile.hpp"
#include "graphviz/edge.hpp"
#include "graphviz/graph.hpp"
#include "graphviz/node.hpp"
#include "graphviz/serialization.hpp"
#include "mem/shared_ptr.hpp"
#include "strings/format.hpp"

namespace j::lisp::cir::inline debug {
  namespace l = lisp;
  namespace g = graphviz;
  namespace a = g::attributes;
  namespace s = strings;

  namespace {
    const g::node_attributes g_default_op_attributes{
      a::shape = a::node_shape::plaintext,
      a::font_name = "Roboto Condensed",
      a::font_size = 9.5,
    };

    enum class live_range_state : u8_t {
      none,
      initial,
      active,
      inactive,
      spilt,
    };

    struct J_TYPE_HIDDEN live_range_rec final {
      const op * op;
      const live_range * range;
      u32_t index;
      op_index min;
      op_index max;
      g::node * begin = nullptr;
      g::node * end = nullptr;
      bool needs_start = false;
      bool needs_end = false;
      bool starts_from_input = false;
      bool ends_at_input = false;
      live_range_state state = live_range_state::initial;
    };

    enum class mov_type : u8_t {
      none,
      spill,
      unspill,
      reg_reg,
    };

    enum class op_position : u8_t {
      none,
      middle,
      bb_begin,
      bb_end,
    };

    enum class lr_action : u8_t {
      none,
      start,
      end,
      phi_enter,
      phi_leave,
    };

    struct J_TYPE_HIDDEN lr_status_op final {
      op_type op = op_type::none;
      mov_type mov = mov_type::none;
      op_position pos = op_position::none;
    };

    struct J_TYPE_HIDDEN lr_status_slot final {
      live_range_state state = live_range_state::none;
      loc_type loc = loc_type::none;
      lr_action action = lr_action::none;
    };

    [[nodiscard]] pair<loc, loc> get_move_locs(const op & op) noexcept {
      if (op.type == op_type::mov) {
        auto d = reinterpret_cast<const op_result_input*>(op.static_begin());
        if (d->use.def == &op) {
          return {d->use.loc_in, d->use.loc_out};
        }
      }
      return {};
    }
    // [[nodiscard]] lr_status_slot get_slot_status(const op & op, const bb * J_NOT_NULL bb) noexcept {

    // }

    // [[nodiscard]] lr_status_op get_op_status(const op & op, const bb * J_NOT_NULL bb) noexcept {
    //   auto locs = get_move_locs(op);
    //   mov_type mov = mov_type::none;
    //   if (locs.first.is_reg() && locs.second.is_reg()) {
    //     mov = mov_type::reg_reg;
    //   } else if (locs.first.is_reg() && locs.second.is_spill_slot()) {
    //     mov = mov_type::spill;
    //   } else if (locs.first.is_spill_slot() && locs.second.is_reg()) {
    //     mov = mov_type::unspill;
    //   }
    //   return {
    //     op.type,
    //     mov,
    //     &op == bb->first_op ? op_position::bb_begin : &op == bb->last_op ? op_position::bb_end : op_position::middle,
    //   };
    // }

    using live_range_map = vector<live_range_rec>;

    const char * const live_range_dead_bg_color = "#EEEEFF";
    const char * const live_range_begin_bg_color = "#C0FF94";
    const char * const live_range_phi_begin_bg_color = "#88FFB8";
    const char * const live_range_phi_end_bg_color = "#FFB888";
    const char * const live_range_end_bg_color = "#FDB7A0";
    const char * const live_range_active_bg_color = "#C5F4C8";
    const char * const live_range_inactive_bg_color = "#B8A0A0";
    const char * const live_range_excluded_bg_color = "#EDECF1";
    const char * const live_range_used_bg_color = "#FFFFA3";
    const char * const live_range_mov_reg_bg_color = "#FFA0FF";
    const char * const live_range_mov_spill_bg_color = "#FFFF50";
    const char * const live_range_spilled_bg_color = "#FDE090";

    const char * const reg_unallocated_bg_color = "#F8E0D4";
    const char * const reg_int_reg_bg_color = "#F4D0F4";
    const char * const reg_fp_reg_bg_color = "#F0F0C0";
    const char * const reg_spill_slot_bg_color = "#F8F8C3";

    const colors::rgb8 live_edge_color = colors::rgb8(0x08, 0xD0, 0x64);

    const char * const table_border_color = "#484860";
    const colors::rgb8 guide_edge_color = colors::rgb8(0x48, 0x48, 0x60);

    s::string format_loc(loc loc) {
      switch (loc.type()) {
      case loc_type::none:
        return "∅";
      case loc_type::spill_slot:
        return s::format("≣ {}", (u32_t)loc.spill_slot_index());
      case loc_type::fp_reg:
      case loc_type::gpr:
        return s::format("{}", loc.as_phys_reg());
      }
    }

    g::node * add_node(g::graph & root,
                       live_range_map & live_ranges,
                       const op & op,
                       const bb * bb) {
      s::string table = s::format("< <table color=\"{}\" cellspacing=\"0\" border=\"2\" cellborder=\"1\" cellpadding=\"2\">", table_border_color);
      s::string up = "<tr>";
      s::string down = "<tr>";

      if (bb) {
        up += s::format("<td rowspan=\"2\" bgcolor=\"#484860\"><font color=\"white\">{}</font></td>", bb->name);
      }

      up += s::format("<td rowspan=\"2\" bgcolor=\"{}\" sides=\"TLB\"><font color=\"{}\">{}</font></td>",
                      op.has_result ? table_border_color : live_range_excluded_bg_color,
                      op.has_result ? "white" : table_border_color,
                      op.type);


      up += s::format("<td rowspan=\"2\" port=\"num\" height=\"22\" width=\"24\" sides=\"TRB\" bgcolor=\"#484860\"><font color=\"{}\" point-size=\"11\"><b>{}</b></font></td>",
                      op.has_result ? "#FFFF9A" : "#E0E0C0", op.index);



      for (auto & it : live_ranges) {
        strings::string lr_reg = "",  lr_use = "";
        const char * bgcolor = live_range_dead_bg_color;
        const char * bgcolor_use = nullptr;
        const_segment_iterator seg_pre, seg_post;
        if (op.index) {
          seg_pre = it.range->segment_at(op.index);
          seg_post = it.range->segment_at(op.index.as_post());
        }
        if (&op == it.op) {
          it.needs_start = true;
          bgcolor = live_range_begin_bg_color;
          it.starts_from_input = false;
          if (seg_post) {
            // Phi output, starts at BB begin really.
            if (seg_post->index_begin == op.index) {
              it.starts_from_input = true;
              bgcolor = live_range_phi_begin_bg_color;
            }
            lr_reg = format_loc(seg_post->loc);
          }
          it.state = live_range_state::active;
        } else if (!op.index) {
          switch (it.state) {
          case live_range_state::spilt:
            bgcolor = live_range_spilled_bg_color;
            break;
          case live_range_state::active:
            bgcolor = live_range_active_bg_color;
            break;
          case live_range_state::inactive:
            bgcolor = live_range_inactive_bg_color;
            break;
          case live_range_state::initial:
          case live_range_state::none:
            break;
          }
          if (op.type == op_type::mov) {
            auto d = reinterpret_cast<const op_result_input*>(op.static_begin());
            if (d->use.def == it.op) {
              bgcolor_use = d->use.loc_in.is_reg() ? live_range_mov_reg_bg_color : live_range_mov_spill_bg_color;
              bgcolor = d->use.loc_out.is_reg() ? live_range_mov_reg_bg_color : live_range_mov_spill_bg_color;
              lr_use = format_loc(d->use.loc_in);
              lr_reg = format_loc(d->use.loc_out);
              it.needs_start = true;
              it.needs_end = true;
              it.ends_at_input = true;

              it.state = d->use.loc_out.is_spill_slot() ? live_range_state::spilt : live_range_state::active;
            }
          }
        } else if (op.index == it.max || op.index.as_post() == it.max) {
          if (seg_pre) {
            lr_reg = format_loc(seg_pre->loc);
          }
          it.state = live_range_state::initial;
          bgcolor = live_range_end_bg_color;
          if (it.begin) {
            it.needs_end = true;
            it.ends_at_input = op.index.as_post() != it.max;
            if (!it.ends_at_input) {
              bgcolor = live_range_phi_end_bg_color;
            }
          }
        } else if (it.range->contains(op.index)) {
          bgcolor = live_range_active_bg_color;
          if (seg_pre && seg_pre->loc.is_spill_slot()) {
            bgcolor_use = live_range_spilled_bg_color;
          }
          if (seg_post && seg_post->loc.is_spill_slot()) {
            bgcolor = live_range_spilled_bg_color;
            it.state = live_range_state::spilt;
          }
          if (it.end) {
            J_ASSERT(seg_pre);
            lr_reg = format_loc(seg_pre->loc);

            bgcolor = live_range_begin_bg_color;
            it.needs_start = true;
          }
        } else if (op.index >= it.min && op.index < it.max) {
          if (it.begin) {
            if (seg_pre) {
              lr_reg = format_loc(seg_pre->loc);
            }

            it.needs_end = true;
            bgcolor = live_range_end_bg_color;
          }
          bgcolor = live_range_inactive_bg_color;
          it.state = live_range_state::inactive;
        } else {
          switch (it.state) {
          case live_range_state::spilt:
            bgcolor = live_range_spilled_bg_color;
            break;
          case live_range_state::active:
            bgcolor = live_range_active_bg_color;
            break;
          case live_range_state::inactive:
            bgcolor = live_range_inactive_bg_color;
            break;
          case live_range_state::initial:
          case live_range_state::none:
            break;
          }
        }

        u32_t oper_idx = 0;
        for (auto & op_res : op.op_results()) {
          if (op_res.use.def == it.op) {
            lr_use = s::format("In {}", oper_idx);
            bgcolor_use = live_range_used_bg_color;
          }
          if (op_res.use.loc_out) {
            auto seg = it.range->segment_at(op.index);
            if (seg && seg->loc == op_res.use.loc_out && !lr_reg) {
              lr_reg = format_loc(seg->loc);
            }
          }
          ++oper_idx;
        }

        if (!bgcolor_use) {
          bgcolor_use = bgcolor;
        }

        up += s::format("<td sides=\"{}\" bgcolor=\"{}\" port=\"i{}\"  width=\"33\">{}</td>",
                        bgcolor == bgcolor_use ? "LTR" : "LTRB",
                        bgcolor_use,
                        it.index,
                        lr_use);
        down += s::format("<td sides=\"{}\" bgcolor=\"{}\" port=\"o{}\">{}</td>", bgcolor == bgcolor_use ? "LBR" : "LTRB", bgcolor, it.index, lr_reg);
      }

      if (op.has_result) {
        const char * bgcolor = reg_unallocated_bg_color;
        switch (op.result.loc_out.type()) {
        case loc_type::spill_slot:
          bgcolor = reg_spill_slot_bg_color;
          break;
        case loc_type::fp_reg:
          bgcolor = reg_fp_reg_bg_color;
          break;
        case loc_type::gpr:
          bgcolor = reg_int_reg_bg_color;
          break;
        case loc_type::none:
          break;
        }
        up += s::format("<td rowspan=\"2\" bgcolor=\"{}\">{}</td>", bgcolor, format_loc(op.result.loc_out));
      }

      for (u8_t i = 0U; i < op.num_aux_regs; ++i) {
        up += s::format("<td rowspan=\"2\" bgcolor=\"#A0A0F0\">{}</td>", format_loc(op.aux_reg(i)));
      }

      table += up + "</tr>" + down + "</tr></table> >";
      g::node * const result = root.add_node(a::label_is_html, a::label = table);
      for (auto & it : live_ranges) {
        if (it.needs_end) {
          // J_ASSUME_NOT_NULL(it.begin);
          J_WARNING_IF(!it.begin, "Edge ended without begin");
          if (it.begin) {
            root.add_edge(g::endpoint(it.begin, strings::format(it.starts_from_input ? "i{}:n" : "o{}", it.index)),
                          g::endpoint(result, strings::format(it.ends_at_input ? "i{}" : "o{}:s", it.index)),
                          a::edge_color = live_edge_color,
                          a::arrow_direction = a::direction::both,
                          a::arrow_tail = a::arrow::box,
                          a::arrow_head = op.index == it.max ? a::arrow::box : a::arrow::obox,
                          a::arrow_size = 0.7f,
                          a::width = 3.f,
                          a::weight = 0);
            it.needs_end = false;
            it.begin = nullptr;
            it.end = result;
          }
        }
        if (it.needs_start) {
          it.needs_start = false;
          it.needs_end = false;
          it.end = nullptr;
          it.begin = result;
        }
      }
      return result;
    }

    strings::string format_reg_mask(loc_specifier mask, bool dark = false) {
      const char * allowed_bg = dark ? "#484888" : "#F8F8FF";
      if (mask.get_mask(false) == loc_mask::any_gpr) {
        allowed_bg = dark ? "#707070" : "#E0FFE0";
      } else if (!mask.mask) {
        allowed_bg = dark ? "#EF4444" : "#FF6666";
      }
      return strings::format("<font color=\"{}\" point-size=\"10.0\">{}</font>", allowed_bg, mask);
    }
  }

  void live_range_dump(const files::path & path, const cir_ssa & ssa) {
    live_range_map lrmap;
    u32_t index = 0;
    for (auto & bb : ssa.in_reverse_postorder()) {
      for (auto & op : bb) {
        if (!op.live_range) {
          continue;
        }
        lrmap.push_back(live_range_rec{
            .op = &op,
            .range = &op.live_range,
            .index = index,
            .min = op.live_range.lower_bound(),
            .max = op.live_range.upper_bound(),
          });
        ++index;
      }
    }

    g::graph root_graph{
      a::css_class = "live-range-dump",
      a::splines = a::spline_type::line,
      a::is_directional,
      a::default_node_attributes = g_default_op_attributes,
      a::rank_direction = a::rank_dir::top_to_bottom,
      a::output_order = a::output_mode::nodes_first,
      a::ranksep = 0.1,
      a::nodesep = 0.1,
      a::margin = 0.05,
    };

    struct J_TYPE_HIDDEN cir_op_bb_pair {
      const cir::op * op = nullptr;
      const cir::bb * bb = nullptr;
    };
    cir_op_bb_pair ops_by_index[ssa.num_ops];

    g::node * prev = nullptr, * first = nullptr;
    for (auto & bb : ssa.in_reverse_postorder()) {
      u32_t index_in_bb = 0;
      for (auto & op : bb) {
        if (op.index) {
          ops_by_index[op.index.instruction_index()] = { &op, &bb, };
        }
        g::node * cur = add_node(root_graph, lrmap, op, index_in_bb == 0 ? &bb : nullptr);
        if (prev) {
          root_graph.add_edge(g::endpoint(prev, "num"),
                              g::endpoint(cur, "num"),
                              a::edge_style = index_in_bb ? a::style::no_value : a::style::invis ,
                              a::arrow_direction = a::direction::none,
                              a::width = 2,
                              a::edge_color = guide_edge_color);
        } else {
          first = cur;
        }
        prev = cur;
        ++index_in_bb;
      }
    }

    s::string label = s::format("< <table port=\"tbl\" color=\"{}\" cellspacing=\"0\" border=\"2\" cellborder=\"1\" cellpadding=\"2\">", table_border_color);
    prev = nullptr;
    for (auto & lr : lrmap) {
      bool is_first = true;
      // if (prev) {
      //   root_graph.add_edge(a::source_node = prev,
      //                       a::source_port = "tbl",
      //                       a::target_port = "tbl",
      //                       a::edge_style = a::style::invis ,
      //                       a::arrow_direction = a::direction::none);
      // }
      for (auto seg : *lr.range) {
        auto begin = ops_by_index[seg.index_begin.instruction_index()];
        auto end = ops_by_index[seg.index_end.instruction_index()];
        const char * bg = is_first ? table_border_color : live_range_excluded_bg_color;
        label += s::format(
          "<tr>"
          "<td bgcolor=\"{}\" sides=\"{}\"><font color=\"#FFFF9A\" point-size=\"12.0\"> {}</font></td>"
          "<td bgcolor=\"{}\" sides=\"TLB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TB\" align=\"left\"><font color=\"{}\">{}</font></td>"
          "<td bgcolor=\"{}\" sides=\"TLB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TRB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TLB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TB\" align=\"left\">{}</td>"
          "<td bgcolor=\"{}\" sides=\"TRB\" align=\"left\">{}</td>"
          "</tr>",
          bg, is_first ? "TBL" : "L", is_first ? s::format("{}", lr.index) : "",
          bg, format_reg_mask(seg.loc_specifier, !is_first),
          bg, is_first ? "#FFFFE0" : "#202020", seg.loc,
          live_range_begin_bg_color, seg.index_begin,
          live_range_begin_bg_color, begin.bb ? begin.bb->name : "NULL",
          live_range_begin_bg_color, begin.op ? s::format("{}", begin.op->type) : "NULL",
          live_range_end_bg_color, seg.index_end,
          live_range_end_bg_color, end.bb ? end.bb->name : "NULL",
          live_range_end_bg_color, end.op ? s::format("{}", end.op->type) : "NULL");
        is_first = false;
      }
    }
    label += "</table> >";

    auto * range_table = root_graph.add_node(a::label_is_html, a::label = label);

    if (first) {
      root_graph.add_edge(range_table, first,
                          a::edge_style = a::style::invis ,
                          a::arrow_direction = a::direction::none);
    }

    g::serialize(root_graph, j::mem::make_shared<files::ofile>(path));
  }
}
