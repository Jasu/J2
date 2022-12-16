// GENERATED FILE - EDITS WILL BE LOST.
#include "lisp/air/passes/air_passes.hpp"
#include "lisp/air/air_package.hpp"
#include "lisp/air/passes/trs_impl.hpp"
#include "lisp/air/passes/trs_write_expr.hpp"
#include "lisp/air/passes/air_pass.hpp"
#include "lisp/air/passes/const_fold.hpp"

namespace j::lisp::air::inline passes {
  using namespace j::lisp::air::exprs;
  namespace ag = air::static_ids;
  namespace {
    // Constants used by `trs_impl.hpp`.
    J_A(ND) constexpr inline bool  debug_print_enabled = false;
    J_A(ND) constexpr inline i32_t exit_token          = 49U;

    #define ERR 2147483647
    #define SR(N) ~(N|65536)
    #define R(N) ~N
    #define S(N) N

    J_A(ND) const i32_t gs[]{
    // Const MkVec EmptyVec Vec Append SymValRd LAndOr IAddConst ISubConst IMulConst IDivConst IDivConstBy AppendInner MergeVecs MergedVecs Progx Prog Proge LAndOrIn
       28,   39,   38,      38, 39,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       47,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          109,      0,         11,   0,   0,    0,
       99,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       56,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       51,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          102,      0,         11,   0,   0,    0,
       77,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       70,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    60,
       69,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          61,         0,        0,         11,   0,   0,    0,
       28,   92,   93,      8,  94,    36,      29,    35,       0,        0,        0,        0,          0,          0,        91,        11,   0,   0,    0,
       86,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   53,   55,      54, 18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       73,   27,   37,      8,  18,    36,      29,    74,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       78,   82,   80,      81, 18,    78,      29,    35,       0,        0,        0,        0,          0,          0,        0,         79,   0,   84,   0,
       87,   27,   37,      8,  18,    36,      88,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       70,   27,   37,      8,  18,    36,      29,    71,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       75,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       0,    0,    0,       0,  0,     0,       0,     0,        0,        0,        0,        0,          0,          0,        0,         0,    0,   0,    0,
       62,   62,   63,      63, 18,    62,      29,    35,       0,        0,        0,        0,          0,          0,        0,         64,   0,   65,   0,
       67,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       104,  27,   104,     8,  18,    104,     29,    35,       0,        0,        0,        0,          0,          0,        0,         104,  0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   49,  0,    0,
       40,   39,   38,      38, 39,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       0,    0,    0,       0,  0,     0,       0,     0,        0,        0,        0,        0,          0,          0,        0,         0,    98,  0,    0,
       41,   27,   37,      8,  18,    36,      29,    42,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       57,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    0,
       0,    0,    0,       0,  0,     0,       0,     0,        0,        0,        0,        0,          0,          0,        0,         0,    97,  0,    0,
       28,   27,   37,      8,  18,    36,      29,    35,       0,        0,        0,        0,          0,          0,        0,         11,   0,   0,    58,
       0,    0,    0,       0,  0,     0,       0,     0,        0,        0,        0,        0,          0,          0,        0,         0,    96,  0,    0,
    };
    J_A(ND) const i32_t * const gotos[]{
      gs + 456, gs + 494, gs + 380, gs + 532, gs + 456, gs + 0,   gs + 19,  gs + 475,
      gs + 380, gs + 95,  gs + 380, gs + 380, gs + 456, gs + 456, gs + 380, gs + 266,
      gs + 76,  gs + 551, gs + 380, gs + 589, gs + 171, gs + 209, gs + 456, gs + 456,
      gs + 456, gs + 399, gs + 418, gs + 380, gs + 380, gs + 380, gs + 190, gs + 95,
      gs + 380, gs + 456, gs + 456, gs + 380, gs + 380, gs + 380, gs + 380, gs + 380,
      gs + 380, gs + 342, gs + 285, gs + 361, gs + 209, gs + 456, gs + 95,  gs + 380,
      gs + 133, gs + 304, gs + 456, gs + 380, gs + 380, gs + 380, gs + 380, gs + 380,
      gs + 247, gs + 380, gs + 323, gs + 456, gs + 323, gs + 228, gs + 608, gs + 570,
      gs + 380, gs + 513, gs + 456, gs + 380, gs + 380, gs + 152, gs + 380, gs + 380,
      gs + 380, gs + 380, gs + 380, gs + 380, gs + 380, gs + 57,  gs + 380, gs + 380,
      gs + 380, gs + 380, gs + 380, gs + 380, gs + 380, gs + 380, gs + 380, gs + 380,
      gs + 380, gs + 380, gs + 114, gs + 456, gs + 380, gs + 380, gs + 380, gs + 380,
      gs + 437, gs + 437, gs + 437, gs + 380, gs + 380, gs + 380, gs + 456, gs + 38,
      gs + 380, gs + 380, gs + 380, gs + 380, gs + 380, gs + 456, gs + 380,
    };

    J_A(ND) const i32_t any_gotos[]{
      0,   2,   0,   43,  2,   2,   48,  50,  0,   2,   0,   0,   50,  50,  0,   2,   4,   2,   0,   59,  59,  50,  50,  4,
      12,  66,  68,  0,   0,   0,   4,   2,   0,   23,  2,   0,   0,   0,   0,   0,   0,   72,  2,   2,   50,  50,  76,  0,
      4,   83,  50,  0,   0,   0,   0,   0,   2,   0,   89,  50,  89,  95,  0,   0,   0,   0,   50,  0,   0,   2,   0,   0,
      0,   0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   50,  50,  0,   0,   0,   0,
      105, 105, 105, 0,   0,   0,   108, 50,  0,   0,   0,   0,   0,   108, 0,
    };

    J_A(ND) const i32_t anys_gotos[]{
      0,  0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  2,  52, 0,  0,  0,  0,  0,  2,  2,  2,  2,  0,
      0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  52, 0,  0,
      0,  0,  85, 0,  0,  0,  0,  0,  0,  0,  0,  85, 0,  0,  0,  0,  0,  0,  85, 0,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  52, 2,  0,  0,  0,  0,
      0,  0,  0,  0,  0,  0,  0,  52, 0,  0,  0,  0,  0,  0,  0,
    };

    J_A(ND) const i32_t exit_actions[]{
      ERR,    ERR,    SR(1),  ERR,    ERR,    ERR,    ERR,    SR(4),
      R(5),   ERR,    SR(6),  R(7),   R(8),   SR(9),  SR(10), ERR,
      ERR,    ERR,    R(12),  SR(4),  SR(4),  SR(9),  SR(9),  ERR,
      ERR,    SR(4),  ERR,    R(15),  R(16),  R(17),  ERR,    ERR,
      SR(4),  ERR,    ERR,    R(20),  R(21),  R(22),  SR(23), SR(24),
      SR(25), ERR,    ERR,    ERR,    SR(9),  SR(9),  ERR,    R(26),
      ERR,    SR(1),  R(8),   SR(27), SR(28), SR(29), SR(30), SR(31),
      ERR,    SR(32), SR(33), SR(34), SR(35), SR(36), R(3),   R(3),
      R(37),  R(3),   SR(34), SR(39), SR(40), ERR,    SR(41), SR(42),
      SR(43), SR(44), SR(45), SR(46), SR(47), ERR,    S(100), R(37),
      S(101), S(101), S(100), R(38),  SR(50), R(51),  SR(52), R(53),
      R(54),  R(55),  SR(9),  R(57),  R(58),  R(59),  R(60),  R(61),
      S(106), S(107), SR(62), SR(63), R(64),  R(65),  SR(66), R(56),
      R(48),  R(49),  R(67),  R(68),  R(69),  S(110), R(70),
    };

    J_A(ND) const i32_t as[]{
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(46),  SR(0),  S(16),  S(16),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), SR(2),  SR(2),  SR(2),  SR(2),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), R(20),  R(20),  R(20),  R(20),  S(7),  S(25),
      R(20), R(20), S(1),   R(20),  R(20),  R(20),  R(20), R(20),
      R(20), R(20), R(20),  S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  R(20), R(20),
      R(20), R(20), R(20),  R(20),  R(20),  R(20),  R(20), R(20),
      S(15), R(20), R(20),  R(20),  R(20),  R(20),  R(20), R(20),
      R(20), S(0),
      S(32), S(10), R(16),  R(16),  R(16),  R(16),  S(7),  S(25),
      R(16), R(16), S(1),   R(16),  R(16),  R(16),  R(16), R(16),
      R(16), R(16), R(16),  S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  R(16), R(16),
      R(16), R(16), R(16),  R(16),  R(16),  R(16),  R(16), R(16),
      S(15), R(16), R(16),  R(16),  R(16),  R(16),  R(16), R(16),
      R(16), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      SR(2), SR(2), S(24),  S(12),  S(45),  SR(2),  S(44), S(44),
      S(15), S(23), S(33),  SR(2),  S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  SR(0),  S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      SR(0), SR(0), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  SR(0),  S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(90),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      R(57), R(57), R(57),  R(57),  R(57),  R(57),  R(57), R(57),
      R(57), R(57), R(57),  R(57),  R(57),  R(57),  R(57), R(57),
      R(57), R(57), R(57),  R(57),  R(57),  R(57),  R(57), R(57),
      R(57), R(57), R(57),  R(57),  R(57),  R(57),  R(57), R(57),
      R(57), R(57), R(57),  R(57),  S(103), R(57),  R(57), R(57),
      R(57), R(57), R(57),  R(57),  R(57),  R(57),  R(57), R(57),
      R(57), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      S(4),  SR(2), S(24),  S(12),  S(45),  SR(2),  S(44), S(44),
      S(15), S(23), S(33),  SR(2),  S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   S(17),  S(3),   S(30),  S(30), S(30),
      S(20), S(19), SR(19), SR(0),  SR(18), SR(18), S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
      S(32), S(10), S(2),   S(2),   S(2),   S(12),  S(7),  S(25),
      S(12), S(6),  S(1),   S(2),   S(2),   S(4),   S(4),  S(14),
      S(4),  S(4),  S(4),   SR(11), S(3),   S(30),  S(30), S(30),
      S(20), S(19), S(31),  S(9),   S(16),  S(16),  S(4),  S(4),
      S(4),  S(34), S(24),  S(12),  S(13),  S(26),  S(21), S(22),
      S(15), S(23), S(33),  S(5),   S(4),   S(23),  S(4),  S(4),
      S(23), S(0),
    };
    #define A(IDX) IDX
    #define U(IDX) (IDX << 1) + 1
    J_A(ND) const i32_t actions[]{
      A(250),     A(300),     U(-65538),  A(250),     A(250),     A(200),     A(0),       U(-4),
      U(-6),      A(50),      U(-65543),  U(-8),      A(250),     A(250),     U(-65547),  A(350),
      A(250),     A(600),     U(-13),     U(-14),     U(-14),     U(-15),     A(250),     A(250),
      A(250),     A(250),     A(250),     U(-16),     U(-17),     U(-18),     A(250),     A(550),
      U(-65541),  A(250),     A(500),     U(-21),     U(-22),     U(-23),     U(-65560),  U(-65561),
      U(-65562),  A(250),     A(100),     A(250),     U(-3),      U(-3),      A(550),     U(-27),
      A(250),     A(250),     A(250),     U(-65564),  U(-65565),  U(-65566),  U(-65567),  U(-65568),
      A(150),     U(-65569),  A(250),     A(250),     A(250),     A(400),     U(-4),      U(-4),
      U(-38),     U(-4),      U(-39),     U(-65576),  U(-65577),  A(150),     U(-65578),  U(-65579),
      U(-65580),  U(-65581),  U(-65582),  U(-65583),  U(-65584),  A(150),     U(-49),     U(-49),
      U(-49),     U(-6),      U(-16),     U(-50),     U(-65587),  U(-52),     U(-65589),  U(-54),
      U(-55),     U(-56),     U(-57),     A(450),     U(-59),     U(-60),     U(-61),     U(-62),
      A(250),     A(250),     A(250),     U(-65600),  U(-65),     U(-66),     A(250),     U(-57),
      U(-49),     U(-50),     U(-68),     U(-69),     U(-70),     A(250),     U(-71),
    };
    #undef A
    #undef U

    // Size: 32
    // Used by: Const, MkVec, SymValRd, IAddConst, ISubConst, IMulConst, IDivConst, IDivConstBy
    struct alignas(8) J_TYPE_HIDDEN nt_captures_3 final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const nt_captures_3 & d = *reinterpret_cast<const nt_captures_3*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" {} ", d.imm1);
        format_expr(d.Term);
      }

      split_tsa Fx;
      lisp_imm imm1 = lisp_imm{undefined_v};
      expr* Term = nullptr;
    };

    // Size: 24
    // Used by: EmptyVec, Vec
    struct alignas(8) J_TYPE_HIDDEN nt_captures_2 final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const nt_captures_2 & d = *reinterpret_cast<const nt_captures_2*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" ");
        format_expr(d.Term);
      }

      split_tsa Fx;
      expr* Term = nullptr;
    };

    // Size: 32
    // Used by: Append, AppendInner, MergeVecs, MergedVecs
    struct alignas(8) J_TYPE_HIDDEN nt_captures_2a final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const nt_captures_2a & d = *reinterpret_cast<const nt_captures_2a*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" ");
        format_stack_alloc(d.Collect.stack, d.Collect.marker, 1);
      }

      split_tsa Fx;
      term_stack_allocation Collect;
    };

    // Size: 48
    // Used by: LAndOr
    struct alignas(8) J_TYPE_HIDDEN LAndOr final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const LAndOr & d = *reinterpret_cast<const LAndOr*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" ");
        format_stack_alloc(d.Collect.stack, d.Collect.marker, 1);
        J_DEBUG_INL(" {} {}", d.Val, d.Op);
      }

      split_tsa Fx;
      term_stack_allocation Collect;
      lisp_imm Val = lisp_imm{undefined_v};
      expr_type Op;
    };

    // Size: 16
    // Used by: Progx, Prog, Proge
    struct alignas(8) J_TYPE_HIDDEN nt_captures_1a final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        format_stack_alloc(reinterpret_cast<const nt_captures_1a*>(data)->Fx.stack, reinterpret_cast<const nt_captures_1a*>(data)->Fx.marker, 2);
      }

      split_tsa Fx;
    };

    // Size: 40
    // Used by: LAndOrIn
    struct alignas(8) J_TYPE_HIDDEN LAndOrIn final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const LAndOrIn & d = *reinterpret_cast<const LAndOrIn*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" ");
        format_stack_alloc(d.Collect.stack, d.Collect.marker, 1);
        J_DEBUG_INL(" {}", d.Val);
      }

      split_tsa Fx;
      term_stack_allocation Collect;
      lisp_imm Val = lisp_imm{undefined_v};
    };

    constexpr nt_info nt_infos[]{
      { "Any" }, // 0
      { "Anys" }, // 1
      { "Const", &nt_captures_3::format  }, // 2
      { "MkVec", &nt_captures_3::format  }, // 3
      { "EmptyVec", &nt_captures_2::format  }, // 4
      { "Vec", &nt_captures_2::format  }, // 5
      { "Append", &nt_captures_2a::format  }, // 6
      { "SymValRd", &nt_captures_3::format  }, // 7
      { "LAndOr", &LAndOr::format  }, // 8
      { "IAddConst", &nt_captures_3::format  }, // 9
      { "ISubConst", &nt_captures_3::format  }, // 10
      { "IMulConst", &nt_captures_3::format  }, // 11
      { "IDivConst", &nt_captures_3::format  }, // 12
      { "IDivConstBy", &nt_captures_3::format  }, // 13
      { "AppendInner", &nt_captures_2a::format  }, // 14
      { "MergeVecs", &nt_captures_2a::format  }, // 15
      { "MergedVecs", &nt_captures_2a::format  }, // 16
      { "Progx", &nt_captures_1a::format  }, // 17
      { "Prog", &nt_captures_1a::format  }, // 18
      { "Proge", &nt_captures_1a::format  }, // 19
      { "LAndOrIn", &LAndOrIn::format  }, // 20
    };

    void reduce(parser_state & s, trs_writer & w, i32_t action) {
      using enum expr_type;
      using et = expr_type;
      const bool is_shift_reduce = action & (1 << 16);
      if (is_shift_reduce) {
        s.shift(0);
        action -= 1 << 16;
      }

      switch (action) {
      case 0:
        // ·/ $1 ⇛  $1
        replace(s.stack[0].e, s.iter.e->in_expr(s.iter.index));
        s.rewind(1, -1);
      break;
      case 1:
        // ⋯ ⇛ Any
        s.reduce_any(-1);
      break;
      case 2:
        // ·$1 ⇛  $1
        replace(s.stack[-1].e, s.stack->e);
        s.rewind(2, -2);
      break;
      case 3:
        // 𝛆 ⇛ Prog(AllocateSplit(\"s.side_fx_stack\"))
        s.reduce<nt_captures_1a>(0, 18, allocate_split(s.side_fx_stack));
      break;
      case 4:
        // ⋯$1 ⇛ Const(AllocateSplit(\"s.side_fx_stack\") If($1.Is(lor) False If($1.Is(land) True lisp_imm{nil_v})) nullptr)
        s.reduce<nt_captures_3>(-1, 2, allocate_split(s.side_fx_stack), s.stack->e && s.stack->e->type == lor ? lisp_f : (s.stack->e && s.stack->e->type == land ? lisp_t : (lisp_imm{nil_v})), nullptr);
      break;
      case 5:
        // Vec($1 $2) ⇛  ApplyFx($2 $1)
        s.replace_top(s.wrap_fx(s.field<expr*>(0, 16), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 6:
        // ⋯$1 ⇛ Const(AllocateSplit(\"s.side_fx_stack\") $1.ConstVal $1)
        s.reduce<nt_captures_3>(-1, 2, allocate_split(s.side_fx_stack), *(lisp_imm*)(s.stack->e + 1), s.stack->e);
      break;
      case 7:
        // Progx($1) ⇛  ApplyProgx($1)
        s.replace_top(s.wrap_progx(s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 8:
        // 𝛆 ⇛ Anys
        s.reduce(0, 1);
      break;
      case 9:
        // ⋯ ⇛ EmptyVec(AllocateSplit(\"s.side_fx_stack\") nullptr)
        s.reduce<nt_captures_2>(-1, 4, allocate_split(s.side_fx_stack), nullptr);
      break;
      case 10:
        if ((*(id*)(s.stack->e + 1)).raw == 66343) {
          // ⋯$1 ⇛ [$1.SymRdId == '%air:*empty-vec*] EmptyVec(AllocateSplit(\"s.side_fx_stack\") $1)
          s.reduce<nt_captures_2>(-1, 4, allocate_split(s.side_fx_stack), s.stack->e);
        } else {
          // ⋯$1 ⇛ SymValRd(AllocateSplit(\"s.side_fx_stack\") $1.SymRdId $1)
          s.reduce<nt_captures_3>(-1, 7, allocate_split(s.side_fx_stack), lisp_imm((*(id*)(s.stack->e + 1))), s.stack->e);
        }
      break;
      case 11:
        // ··/ $1 ⇛  $1
        replace(s.stack[-1].e, s.iter.e->in_expr(s.iter.index));
        s.rewind(2, -2);
      break;
      case 12:
        // Append($1 $2) ⇛  ApplyFx(Many(vec_append Release($2)) $1)
        s.replace_top(s.wrap_fx(s.wrap_many(vec_append, s.field<term_stack_allocation>(0, 16).release()), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 13:
        // 𝛆 ⇛ LAndOrIn(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\") lisp_imm{undefined_v})
        s.reduce<LAndOrIn>(0, 20, allocate_split(s.side_fx_stack), allocate(s.collect_stack), lisp_imm{undefined_v});
      break;
      case 14:
        // 𝛆 ⇛ AppendInner(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\"))
        s.reduce<nt_captures_2a>(0, 14, allocate_split(s.side_fx_stack), allocate(s.collect_stack));
      break;
      case 15:
        // MkVec($1…3) ⇛  ApplyFx(If($2.IsI64 mk-vec(MkConst($2 $3)) $3) $1)
        s.replace_top(s.wrap_fx(s.field<lisp_imm>(0, 16).is_i64() ? w.wr<>(et::make_vec, s.mk_const(s.field<lisp_imm>(0, 16), s.field<expr*>(0, 24))) : (s.field<expr*>(0, 24)), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 16:
        // Const($1…3) ⇛  ApplyFx(MkConst($2 $3) $1)
        s.replace_top(s.wrap_fx(s.mk_const(s.field<lisp_imm>(0, 16), s.field<expr*>(0, 24)), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 17:
        // LAndOr($1 $2·$4) ⇛  ApplyFx($4($2) $1)
        s.replace_top(s.wrap_fx(w.wr<>(s.field<expr_type>(0, 40), s.field<term_stack_allocation>(0, 16)), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 18:
        // ·$1 / $2 $3 ⇛  If($1.Is(eq) neq eq)($2 $3)
        replace(s.stack[-1].e, w.wr<>(s.stack->e && s.stack->e->type == eq ? neq : eq, s.iter.e->in_expr(s.iter.index), s.iter.e->in_expr(s.iter.index + 1)));
        s.rewind(2, -2);
      break;
      case 19:
        // $1·/ $2 ⇛  If($2.Is(lnot to_bool eq neq) || $1.Is(b_if) $2 to-bool($2))
        replace(s.stack[-1].e, (s.iter.e->in_expr(s.iter.index) && (s.iter.e->in_expr(s.iter.index)->type == lnot || s.iter.e->in_expr(s.iter.index)->type == to_bool || s.iter.e->in_expr(s.iter.index)->type == eq || s.iter.e->in_expr(s.iter.index)->type == neq)) || (s.stack[-1].e && s.stack[-1].e->type == b_if) ? s.iter.e->in_expr(s.iter.index) : (w.wr<>(et::to_bool, s.iter.e->in_expr(s.iter.index))));
        s.rewind(2, -2);
      break;
      case 20:
        // IAddConst($1…3) ⇛  iadd(ApplyFx($3 $1) const($2))
        s.replace_top(w.wr<>(et::iadd,
          s.wrap_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0)),
          w.wrc(s.field<lisp_imm>(0, 16))
        ));
        s.reduce_any(1);
      break;
      case 21:
        // SymValRd($1…3) ⇛  ApplyFx(If($3.Empty sym-rd($2.AsId) $3) $1)
        s.replace_top(s.wrap_fx(!s.field<expr*>(0, 24) ? w.wr<id>(et::sym_val_rd, id(raw_tag, s.field<lisp_imm>(0, 16).raw)) : (s.field<expr*>(0, 24)), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 22:
        // EmptyVec($1 $2) ⇛  If($2.Empty sym-rd('%air:*empty-vec*) $2).PreFx($1)
        s.replace_top(s.wrap_pre_fx(!s.field<expr*>(0, 16) ? w.wr<id>(et::sym_val_rd, 66343) : (s.field<expr*>(0, 16)), s.field<split_tsa>(0, 0)));
        s.reduce_any(1);
      break;
      case 23:
        // ⋯Blob(24):$1· ⇛ Copy(-1)
        s.copy_reduce(-1, 1, 24);
      break;
      case 24:
        // ⋯Blob(32):$1· ⇛ Copy(-1)
        s.copy_reduce(-1, 1, 32);
      break;
      case 25:
        if ((bool)s.field<lisp_imm>(1, 16)) {
          // ⋯Blob(32):$4· ⇛ [$2.IsTruthy] Copy(-1)
          s.copy_reduce(-1, 1, 32);
        } else {
          // ⋯Const($1·$3)· ⇛  until(progn(Release($1) $3))
          replace(s.stack[-2].e, w.wr<>(et::do_until, w.progn(s.field<split_tsa>(1, 0).release(), s.field<expr*>(1, 24))));
          s.reduce_any(-1);
        }
      break;
      case 26:
        // ·Const($2 $1·) / $3 $4 ⇛  If($1.IsTruthy $3 $4).PreFx($2)
        replace(s.stack[-1].e, s.wrap_pre_fx((bool)s.field<lisp_imm>(0, 16) ? s.iter.e->in_expr(s.iter.index) : (s.iter.e->in_expr(s.iter.index + 1)), s.field<split_tsa>(0, 0)));
        s.rewind(2, -1);
      break;
      case 27:
        // ⋯Const($3 $2·) $1 ⇛ Const($3 Fold($1 $2) nullptr)
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(1, 16)), nullptr);
      break;
      case 28:
        // ⋯$1 ⇛ Vec(AllocateSplit(\"s.side_fx_stack\") $1)
        s.reduce<nt_captures_2>(-1, 5, allocate_split(s.side_fx_stack), s.stack->e);
      break;
      case 29:
        if (s.field<lisp_imm>(1, 16).is_i64()) {
          // ⋯MkVec($1 $2·)· ⇛ [$2.IsI64] Const($1 $2 nullptr)
          s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), nullptr);
        } else {
          // ⋯MkVec($1·$3)· ⇛  ApplyFx($3.In(0) $1)
          replace(s.stack[-2].e, s.wrap_fx(s.field<expr*>(1, 24)->in_expr(0), s.field<split_tsa>(1, 0)));
          s.reduce_any(-1);
        }
      break;
      case 30:
        // ⋯Vec($1 $2)· ⇛ Const($1.AsPreFx() $2.NumInputs nullptr)
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), lisp_imm{(u64_t)(s.field<expr*>(1, 16)->num_inputs) << 1}, nullptr);
      break;
      case 31:
        // ⋯EmptyVec($1·)· ⇛ Const($1.AsPreFx() 0 nullptr)
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), lisp_imm{0}, nullptr);
      break;
      case 32:
        // ⋯Const($2 $1·)· ⇛ Const($2 $1 * -1 nullptr)
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * -1)}, nullptr);
      break;
      case 33:
        if (!s.field<term_stack_allocation>(1, 16)) {
          // ⋯LAndOrIn($2·$4) $1 ⇛ [$3.Empty] Const($2.AsPreFx() If($4.HasValue $4 If($1.Is(land) True False)) nullptr)
          s.field<term_stack_allocation>(1, 16).release();
          s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), !s.field<lisp_imm>(1, 32).is_undefined() ? s.field<lisp_imm>(1, 32) : (s.stack->e && s.stack->e->type == land ? lisp_t : lisp_f), nullptr);
        } else if (s.field<term_stack_allocation>(1, 16).size() == 1) {
          // ⋯LAndOrIn($2 $3·)· ⇛ [$3.Single]  Release($3).First.PreFx($2)
          replace(s.stack[-2].e, s.wrap_pre_fx(s.field<term_stack_allocation>(1, 16).release().front(), s.field<split_tsa>(1, 0)));
          s.reduce_any(-1);
        } else {
          // ⋯LAndOrIn($2 $3 $4) $1 ⇛ LAndOr($2 $3 $4 $1.Type)
          s.reduce<LAndOr>(-1, 8, s.field<split_tsa>(1, 0), s.field<term_stack_allocation>(1, 16), s.field<lisp_imm>(1, 32), s.stack->e->type);
        }
      break;
      case 34:
        // ⋯$1· ⇛  $1
        replace(s.stack[-2].e, s.stack[-1].e);
        s.reduce_any(-1);
      break;
      case 35:
        if (!s.field<term_stack_allocation>(1, 16)) {
          // ⋯LAndOrIn($2·$4) $1 ⇛ [$3.Empty] Const($2.AsPreFx() If($4.HasValue $4 If($1.Is(land) True False)) nullptr)
          s.field<term_stack_allocation>(1, 16).release();
          s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), !s.field<lisp_imm>(1, 32).is_undefined() ? s.field<lisp_imm>(1, 32) : (s.stack->e && s.stack->e->type == land ? lisp_t : lisp_f), nullptr);
        } else if (s.field<term_stack_allocation>(1, 16).size() == 1) {
          // ⋯LAndOrIn($2 $3·)· ⇛ [$3.Single]  Release($3).First.PreFx($2)
          replace(s.stack[-2].e, s.wrap_pre_fx(s.field<term_stack_allocation>(1, 16).release().front(), s.field<split_tsa>(1, 0)));
          s.reduce_any(-1);
        } else {
          // ⋯LAndOrIn($2 $3 $4) $1 ⇛ LAndOr($2 $3 $4 $1.Type)
          s.reduce<LAndOr>(-1, 8, s.field<split_tsa>(1, 0), s.field<term_stack_allocation>(1, 16), s.field<lisp_imm>(1, 32), s.stack->e->type);
        }
      break;
      case 36:
        if (!s.field<term_stack_allocation>(1, 16)) {
          // ⋯AppendInner($2·)· ⇛ [$3.Empty] EmptyVec($2.AsPreFx() nullptr)
          s.field<term_stack_allocation>(1, 16).release();
          s.reduce<nt_captures_2>(-1, 4, s.field<split_tsa>(1, 0).to_pre_fx(), nullptr);
        } else {
          // ⋯AppendInner:$1· ⇛ Copy(-1)
          s.copy_reduce(-1, 1, 32, 6);
        }
      break;
      case 37:
        // Progx:$1 ⇛ Copy(1)
        s.copy_reduce(1, 0, 16, 19);
      break;
      case 38:
        // $1 ⇛ Proge(AllocFx($1).AsPreFx())
        s.reduce<nt_captures_1a>(1, 19, allocate_split(s.side_fx_stack, s.stack->e).to_pre_fx());
      break;
      case 39:
        if (!(s.field<lisp_imm>(1, 16)).raw) {
          // ⋯Const($3··)· ⇛ [$2 == 0] EmptyVec($3 nullptr)
          s.reduce<nt_captures_2>(-1, 4, s.field<split_tsa>(1, 0), nullptr);
        } else {
          // ⋯Const($3 $2·) $1 ⇛ MkVec($3 $2 $1)
          s.reduce<nt_captures_3>(-1, 3, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), s.stack->e);
        }
      break;
      case 40:
        // ⋯$1 ⇛ MkVec(AllocateSplit(\"s.side_fx_stack\") lisp_imm{undefined_v} $1)
        s.reduce<nt_captures_3>(-1, 3, allocate_split(s.side_fx_stack), lisp_imm{undefined_v}, s.stack->e);
      break;
      case 41:
        // ⋯Const($2 $3·) Const($4 $5·) $1 ⇛ Const($2 !+$ $4 Fold($1 $3 $5) nullptr)
        s.reduce<nt_captures_3>(-1, 2, split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0))), s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(2, 16), s.field<lisp_imm>(1, 16)), nullptr);
      break;
      case 42:
        if (!(lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}).raw) {
          // ⋯Const($1··) IAddConst($3·$5)· ⇛ [$2 + $4 == 0]  ApplyFx($5 $1 !+$ $3)
          replace(s.stack[-3].e, s.wrap_fx(s.field<expr*>(1, 24), split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)))));
          s.reduce_any(-1);
        } else {
          // ⋯Const($1 $2·) IAddConst($3 $4 $5)· ⇛ IAddConst($1 !+$ $3 $2 + $4 $5)
          s.reduce<nt_captures_3>(-1, 9, split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0))), lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}, s.field<expr*>(1, 24));
        }
      break;
      case 43:
        if (!(s.field<lisp_imm>(2, 16)).raw) {
          // ⋯Const($1··) $3· ⇛ [$2 == 0]  ApplyFx($3 $1)
          replace(s.stack[-3].e, s.wrap_fx(s.stack[-1].e, s.field<split_tsa>(2, 0)));
          s.reduce_any(-1);
        } else {
          // ⋯Const($1 $2·) $3· ⇛ IAddConst($1 $2 $3)
          s.reduce<nt_captures_3>(-1, 9, s.field<split_tsa>(2, 0), s.field<lisp_imm>(2, 16), s.stack[-1].e);
        }
      break;
      case 44:
        if (!(lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}).raw) {
          // ⋯IAddConst($1·$3) Const($4··)· ⇛ [$2 + $5 == 0]  ApplyFx($3 $1 $+! $4.AsPostFx())
          replace(s.stack[-3].e, s.wrap_fx(s.field<expr*>(2, 24), split_tsa::append_fx((s.field<split_tsa>(2, 0)), s.field<split_tsa>(1, 0).to_post_fx())));
          s.reduce_any(-1);
        } else {
          // ⋯IAddConst($1…3) Const($4 $5·)· ⇛ IAddConst($1 $+! $4.AsPostFx() $2 + $5 $3)
          s.reduce<nt_captures_3>(-1, 9, split_tsa::append_fx((s.field<split_tsa>(2, 0)), s.field<split_tsa>(1, 0).to_post_fx()), lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}, s.field<expr*>(2, 24));
        }
      break;
      case 45:
        if (!(lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}).raw) {
          // ⋯IAddConst($1·$3) IAddConst($4·$6)· ⇛ [$2 + $5 == 0]  iadd(ApplyFx($3 $1) ApplyFx($6 $4))
          replace(s.stack[-3].e, w.wr<>(et::iadd,
            s.wrap_fx(s.field<expr*>(2, 24), s.field<split_tsa>(2, 0)),
            s.wrap_fx(s.field<expr*>(1, 24), s.field<split_tsa>(1, 0))
          ));
          s.reduce_any(-1);
        } else {
          // ⋯IAddConst($1…3) IAddConst($4 $5 $6)· ⇛ IAddConst(AllocateSplit(\"s.side_fx_stack\") $2 + $5 iadd(ApplyFx($3 $1) ApplyFx($6 $4)))
          s.reduce<nt_captures_3>(-1, 9, allocate_split(s.side_fx_stack), lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (s.field<lisp_imm>(1, 16)).raw}, w.wr<>(et::iadd,
            s.wrap_fx(s.field<expr*>(2, 24), s.field<split_tsa>(2, 0)),
            s.wrap_fx(s.field<expr*>(1, 24), s.field<split_tsa>(1, 0))
          ));
        }
      break;
      case 46:
        if (!(s.field<lisp_imm>(1, 16)).raw) {
          // ⋯$1 Const($2··)· ⇛ [$3 == 0]  ApplyFx($1 $2)
          replace(s.stack[-3].e, s.wrap_fx(s.stack[-2].e, s.field<split_tsa>(1, 0)));
          s.reduce_any(-1);
        } else {
          // ⋯$1 Const($2 $3·)· ⇛ IAddConst($2.AsPostFx() $3 $1)
          s.reduce<nt_captures_3>(-1, 9, s.field<split_tsa>(1, 0).to_post_fx(), s.field<lisp_imm>(1, 16), s.stack[-2].e);
        }
      break;
      case 47: {
        // ·$1·/ $2 $3 ⇛  $2 $3 · /· $1
        expr* it1 = s.iter.e->in_expr(s.iter.index);
        replace(s.stack[-2].e, s.stack[-1].e);
        s.iter.e->set_input(s.iter.index, s.iter.e->in_expr(s.iter.index + 1));
        s.iter.e->set_input(s.iter.index + 1, it1);
        s.rewind(3, 0);
      } break;
      case 48:
        // Prog Const($1··) ⇛ { Fx +=! $1; Pop 1 }
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        s.pop(1);
      break;
      case 49:
        // Prog $1 ⇛ { Fx += $1; Pop 1 }
        s.field<split_tsa>(1, 0).push((s.stack->e));
        s.pop(1);
      break;
      case 50:
        // ⋯Prog($1) Proge($2)· ⇛ Progx($1 !+$ $2)
        s.reduce<nt_captures_1a>(-1, 17, split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0))));
      break;
      case 51:
        // ·· ⇛ Anys
        s.reduce(2, 1);
      break;
      case 52:
        // ⋯Const($3 $2·) Const($5 $4·) $1 ⇛ Const($3 $+! $5.AsPreFx() Fold($1.Type $2 $4) nullptr)
        s.reduce<nt_captures_3>(-1, 2, split_tsa::append_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0))).to_pre_fx(), s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(2, 16), s.field<lisp_imm>(1, 16)), nullptr);
      break;
      case 53:
        if ((s.iter.e && s.iter.e->type == lor) == (bool)s.field<lisp_imm>(0, 16)) {
          // LAndOrIn Const($2 $1·) ⇛ [s.iter.e.Is(lor) == $1.IsTruthy] { Val = $1; Fx +=! $2; Trunc(0); Pop 1 }
          s.field<lisp_imm>(1, 32) = (s.field<lisp_imm>(0, 16));
          s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          s.iter.e->num_inputs = s.iter.index;
          s.update_lookahead();
          s.pop(1);
        } else {
          // LAndOrIn Const($2 $1·) ⇛ { Val = $1; Fx +=! $2; Pop 1 }
          s.field<lisp_imm>(1, 32) = (s.field<lisp_imm>(0, 16));
          s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          s.pop(1);
        }
      break;
      case 54:
        if (s.iter.e->type == (s.field<expr_type>(0, 40))) {
          // LAndOrIn LAndOr($4 $3 $2·) ⇛ [s.iter.e.Type == $1] { Val = $2; Collect += $3; Fx +=! $4; Pop 1 }
          s.field<lisp_imm>(1, 32) = (s.field<lisp_imm>(0, 32));
          s.field<term_stack_allocation>(1, 16) += (s.field<term_stack_allocation>(0, 16));
          s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          s.pop(1);
        } else {
          // LAndOrIn LAndOr($4 $3 $2 $1) ⇛ { Val = $2; Collect += ApplyFx($1(Release($3)) $4); Pop 1 }
          s.field<lisp_imm>(1, 32) = (s.field<lisp_imm>(0, 32));
          s.field<term_stack_allocation>(1, 16).push(s.wrap_fx(w.wr<>(s.field<expr_type>(0, 40), s.field<term_stack_allocation>(0, 16).release()), s.field<split_tsa>(0, 0)));
          s.pop(1);
        }
      break;
      case 55:
        // LAndOrIn $1 ⇛ { Collect += ApplyFxClear($1 Fx); Val = lisp_imm{undefined_v}; Pop 1 }
        s.field<term_stack_allocation>(1, 16).push(s.wrap_fx_clear(s.stack->e, s.field<split_tsa>(1, 0)));
        s.field<lisp_imm>(1, 32) = (lisp_imm{undefined_v});
        s.pop(1);
      break;
      case 56:
        // 𝛆 ⇛ MergeVecs(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\"))
        s.reduce<nt_captures_2a>(0, 15, allocate_split(s.side_fx_stack), allocate(s.collect_stack));
      break;
      case 57:
        // AppendInner MergedVecs($1 $2) ⇛ { Collect += vec(Release($2)); Fx +=! $1; Pop 1 }
        s.field<term_stack_allocation>(1, 16).push((w.wr<>(et::vec, s.field<term_stack_allocation>(0, 16).release())));
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        s.pop(1);
      break;
      case 58:
        // AppendInner MkVec($1…3) ⇛ { Collect += If($3.Empty mk-vec(const($2)) $3); Fx +=! $1; Pop 1 }
        s.field<term_stack_allocation>(1, 16).push((!s.field<expr*>(0, 24) ? w.wr<>(et::make_vec, w.wrc(s.field<lisp_imm>(0, 16))) : (s.field<expr*>(0, 24))));
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        s.pop(1);
      break;
      case 59:
        // AppendInner EmptyVec($1·) ⇛ { Fx +=! $1; Pop 1 }
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        s.pop(1);
      break;
      case 60:
        // AppendInner Append($1 $2) ⇛ { Collect += $2; Fx +=! $1; Pop 1 }
        s.field<term_stack_allocation>(1, 16) += (s.field<term_stack_allocation>(0, 16));
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        s.pop(1);
      break;
      case 61:
        // AppendInner $1 ⇛ { Collect += ApplyFxClear($1 Fx); Pop 1 }
        s.field<term_stack_allocation>(1, 16).push(s.wrap_fx_clear(s.stack->e, s.field<split_tsa>(1, 0)));
        s.pop(1);
      break;
      case 62:
        // ⋯Proge($1) Prog($2)· ⇛ Progx($1 $+! $2)
        s.reduce<nt_captures_1a>(-1, 17, split_tsa::append_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0))));
      break;
      case 63: {
        if (((s.field<lisp_imm>(2, 16)).raw == false_v && (s.field<lisp_imm>(1, 16)).raw == true_v) || ((s.field<lisp_imm>(2, 16)).raw == true_v && (s.field<lisp_imm>(1, 16)).raw == false_v)) {
          // ⋯$2 Const(·$3·)·· ⇛ [$3 == False && $6 == True || $3 == True && $6 == False]  If($3 == True to_bool lnot)($2)
          s.field<split_tsa>(1, 0).release();
          s.field<split_tsa>(2, 0).release();
          replace(s.stack[-4].e, w.wr<>((s.field<lisp_imm>(2, 16)).raw == true_v ? to_bool : lnot, s.stack[-3].e));
          s.reduce_any(-1);
        } else {
          // ⋯$2 Const($5 $3 $4) Const($8 $6 $7)· ⇛  if($2 MkConst($3 $4 $5) MkConst($6 $7 $8))
          split_span rel_0 = s.field<split_tsa>(1, 0).release();
          split_span rel_1 = s.field<split_tsa>(2, 0).release();
          replace(s.stack[-4].e, w.wr<>(et::b_if,
            s.stack[-3].e,
            s.wrap_pre_fx(s.mk_const(s.field<lisp_imm>(2, 16), s.field<expr*>(2, 24)), rel_1),
            s.wrap_pre_fx(s.mk_const(s.field<lisp_imm>(1, 16), s.field<expr*>(1, 24)), rel_0)
          ));
          s.reduce_any(-1);
        }
      } break;
      case 64:
        // ⋯Prog($1) Const:$3($2··)· ⇛ { $2 +=! $1; Copy(-1) }
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(2, 0)));
        s.copy_reduce(-1, 1, 32);
      break;
      case 65:
        // ⋯Prog($1) EmptyVec:$3($2·)· ⇛ { $2 +=! $1; Copy(-1) }
        s.field<split_tsa>(1, 0).inplace_concat_fx((s.field<split_tsa>(2, 0)));
        s.copy_reduce(-1, 1, 24);
      break;
      case 66:
        // ⋯MergeVecs:$1· ⇛ Copy(-1)
        s.copy_reduce(-1, 1, 32, 16);
      break;
      case 67:
        // ⋯Const:$3($2··) Prog($1)· ⇛ { $2 +=! $1; Copy(-1) }
        s.field<split_tsa>(2, 0).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        s.copy_reduce(-1, 2, 32);
      break;
      case 68:
        // ⋯EmptyVec:$3($2·) Prog($1)· ⇛ { $2 +=! $1; Copy(-1) }
        s.field<split_tsa>(2, 0).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        s.copy_reduce(-1, 2, 24);
      break;
      case 69:
        // MergeVecs $1 ⇛ { Collect += $1; Pop 1 }
        s.field<term_stack_allocation>(1, 16).push((s.stack->e));
        s.pop(1);
      break;
      case 70:
        // MergedVecs·MergeVecs($1 $2)· ⇛ { Collect += $2; Fx +=! $1; Pop 3 }
        s.field<term_stack_allocation>(3, 16) += (s.field<term_stack_allocation>(1, 16));
        s.field<split_tsa>(3, 0).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        s.pop(3);
      break;
      default: J_FAIL("Unexpected reduction {}.", action);
      }
    }
  }

  void trs_simplify(j::mem::bump_pool & pool, expr * J_NOT_NULL root) {
    parser_state s{pool, root};
    trs_writer w{pool};
    do {
      i32_t action;
      if (s.lookahead == exit_token) {
        action = exit_actions[s.stack->state];
      } else {
        action = actions[s.stack->state];
        if (action & 1) {
          action >>= 1;
        } else {
          action = as[action + s.lookahead];
          if (J_UNLIKELY(action == ERR)) {
            s.fail("Error action");
          }
        }
      }
      if (action >= 0) {
        s.shift(action);
      } else {
        reduce(s, w, ~action);
      }
    } while (!s.done);
    s.check_empty();
  }

  static void apply_trs_simplify_pass(air_pass_context & ctx) {
    trs_simplify(*ctx.pool, ctx.fn->body);
  }

  const air_pass trs_simplify_pass{apply_trs_simplify_pass, "trs-simplify", "Run term replacement", true};
}
