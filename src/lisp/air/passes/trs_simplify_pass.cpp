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
    // Const MkVec EmptyVec Vec  Append SymValRd LAndOr IAddConst IMulConst IDivConst IDivConstBy AppendInner MergeVecs MergedVecs Progx Prog Proge LAndOrIn
       116,  18,   116,     30,  39,    116,     36,    28,       29,       37,       19,         0,          0,        0,         116,  0,   0,    0,      
       52,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       48,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   65,   67,      66,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         77,         0,        0,         38,   0,   0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          121,      0,         38,   0,   0,    0,      
       78,   18,   35,      30,  39,    40,      36,    90,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    50,     
       88,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       74,   46,   46,      46,  46,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          122,      0,         38,   0,   0,    0,      
       80,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       97,   102,  97,      101, 39,    97,      36,    100,      29,       37,       19,         0,          0,        0,         98,   0,   103,  0,      
       64,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       75,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       114,  18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       96,   18,   35,      30,  39,    40,      36,    95,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       0,    0,    0,       0,   0,     0,       0,     0,        0,        0,        0,          0,          0,        0,         0,    105, 0,    0,      
       58,   18,   35,      30,  39,    40,      36,    59,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       104,  18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   110,  111,     30,  112,   40,      36,    28,       29,       37,       19,         0,          0,        108,       38,   0,   0,    0,      
       55,   18,   35,      30,  39,    40,      36,    56,       57,       57,       57,         0,          0,        0,         38,   0,   0,    0,      
       78,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       68,   70,   70,      70,  39,    70,      36,    69,       29,       37,       19,         0,          0,        0,         71,   0,   72,   0,      
       93,   18,   35,      30,  39,    40,      36,    92,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       82,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       26,   46,   46,      46,  46,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       0,    0,    0,       0,   0,     0,       0,     0,        0,        0,        0,          0,          0,        0,         0,    107, 0,    0,      
       26,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   63,  0,    0,      
       53,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       0,    0,    0,       0,   0,     0,       0,     0,        0,        0,        0,          0,          0,        0,         0,    106, 0,    0,      
       83,   18,   35,      30,  39,    40,      84,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
       0,    0,    0,       0,   0,     0,       0,     0,        0,        0,        0,          0,          0,        0,         0,    0,   0,    0,      
       41,   18,   35,      30,  39,    40,      36,    28,       29,       37,       19,         0,          0,        0,         38,   0,   0,    0,      
    };
    J_A(ND) const i32_t * const gotos[]{
      gs + 54,  gs + 612, gs + 594, gs + 54,  gs + 486, gs + 36,  gs + 144, gs + 18, 
      gs + 540, gs + 594, gs + 396, gs + 342, gs + 54,  gs + 54,  gs + 594, gs + 522,
      gs + 252, gs + 594, gs + 594, gs + 594, gs + 72,  gs + 432, gs + 180, gs + 54, 
      gs + 270, gs + 54,  gs + 594, gs + 54,  gs + 594, gs + 594, gs + 594, gs + 18, 
      gs + 90,  gs + 54,  gs + 54,  gs + 594, gs + 594, gs + 594, gs + 594, gs + 594,
      gs + 594, gs + 414, gs + 216, gs + 54,  gs + 54,  gs + 90,  gs + 594, gs + 18, 
      gs + 594, gs + 468, gs + 576, gs + 54,  gs + 594, gs + 414, gs + 162, gs + 594,
      gs + 594, gs + 594, gs + 126, gs + 450, gs + 306, gs + 54,  gs + 594, gs + 234,
      gs + 360, gs + 594, gs + 594, gs + 594, gs + 324, gs + 558, gs + 558, gs + 594,
      gs + 504, gs + 54,  gs + 594, gs + 594, gs + 594, gs + 378, gs + 594, gs + 594,
      gs + 594, gs + 594, gs + 288, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594,
      gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594,
      gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594,
      gs + 594, gs + 0,   gs + 0,   gs + 0,   gs + 54,  gs + 108, gs + 594, gs + 594,
      gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594, gs + 594,
      gs + 198, gs + 54,  gs + 54,  gs + 594, gs + 594,
    };

    J_A(ND) const i32_t any_gotos[]{
      0,   42,  0,   2,   2,   49,  51,  2,   54,  0,   2,   60,  61,  61,  0,   61,  3,   0,   0,   0,   2,   73,  2,   61, 
      76,  3,   0,   12,  0,   0,   0,   2,   61,  25,  2,   0,   0,   0,   0,   0,   0,   79,  2,   61,  61,  61,  0,   81, 
      0,   3,   85,  61,  0,   87,  2,   0,   0,   0,   91,  94,  2,   61,  0,   99,  2,   0,   0,   0,   0,   0,   0,   0,  
      0,   61,  0,   0,   0,   113, 0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
      0,   0,   0,   0,   0,   0,   0,   0,   0,   117, 117, 117, 61,  61,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  
      61,  123, 123, 0,   0,  
    };

    J_A(ND) const i32_t anys_gotos[]{
      0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  2,  62, 0,  2,  0,  0,  0,  0,  0,  2,  0,  2, 
      0,  0,  0,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 2,  2,  0,  0, 
      0,  0,  0,  86, 0,  0,  0,  0,  0,  0,  0,  0,  0,  86, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      0,  86, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  2,  62, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      62, 0,  0,  0,  0, 
    };

    J_A(ND) const i32_t exit_actions[]{
      ERR,    ERR,    SR(0),  ERR,    ERR,    ERR,    SR(4),  ERR,    ERR,    SR(5),  ERR,    ERR,    R(7),   SR(8),  SR(9),  SR(4),  
      ERR,    SR(4),  R(11),  R(12),  ERR,    SR(4),  ERR,    SR(8),  ERR,    ERR,    R(13),  ERR,    R(14),  R(15),  R(16),  ERR,    
      R(19),  ERR,    ERR,    R(20),  R(21),  R(22),  R(23),  R(24),  R(25),  ERR,    ERR,    SR(8),  SR(8),  R(19),  SR(26), ERR,    
      R(27),  ERR,    SR(28), SR(29), SR(30), ERR,    ERR,    SR(31), SR(32), S(89),  ERR,    ERR,    ERR,    R(7),   SR(33), SR(0),  
      ERR,    SR(34), SR(35), SR(36), R(10),  R(37),  R(10),  R(38),  R(10),  SR(29), SR(40), SR(41), SR(42), SR(43), SR(44), SR(45), 
      SR(46), SR(47), ERR,    R(48),  R(49),  R(50),  R(51),  SR(52), SR(53), R(54),  SR(55), SR(56), SR(57), SR(58), SR(59), SR(60), 
      SR(61), S(115), R(38),  R(39),  S(115), S(115), S(115), SR(64), SR(65), S(118), S(119), SR(66), R(67),  SR(8),  R(69),  R(70),  
      R(71),  R(72),  SR(73), R(74),  R(62),  R(63),  R(75),  R(76),  R(68),  SR(77), S(124), R(78),  R(79),  
    };

    J_A(ND) const i32_t as[]{
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  SR(1),  SR(1), SR(1),  SR(1),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      SR(1), SR(1), S(27), S(12), S(43),  SR(1), S(45), S(44), S(20), S(25), S(33),  SR(1), S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  R(13), R(13), R(13),  R(13), S(15), S(21), R(13), R(13), S(22),  R(13), R(13),  R(13),  R(13), R(13), 
      R(13), R(13), R(13), S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  R(13), R(13), 
      R(13), R(13), R(13), R(13), R(13),  R(13), R(13), R(13), S(20), R(13), R(13),  R(13), R(13),  R(13),  R(13), R(13), 
      R(13), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      S(3),  SR(1), S(27), S(12), S(43),  SR(1), S(45), S(44), S(20), S(25), S(33),  SR(1), S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      R(67), R(67), R(67), R(67), R(67),  R(67), R(67), R(67), R(67), R(67), R(67),  R(67), R(67),  R(67),  R(67), R(67), 
      R(67), R(67), R(67), R(67), R(67),  R(67), R(67), R(67), R(67), R(67), R(67),  R(67), R(67),  R(67),  R(67), R(67), 
      R(67), R(67), R(67), R(67), S(120), R(67), R(67), R(67), R(67), R(67), R(67),  R(67), R(67),  R(67),  R(67), R(67), 
      R(67), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(47),  SR(2), S(16),  S(16),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  SR(18), SR(2), SR(17), SR(17), S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  SR(6), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(109), S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  SR(2),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      S(3),  S(34), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  S(4),  S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
      S(17), S(9),  S(2),  S(2),  S(2),   S(12), S(15), S(21), S(12), S(5),  S(22),  S(2),  S(2),   S(3),   S(3),  S(14), 
      S(3),  S(3),  S(3),  S(10), S(11),  S(11), S(1),  S(8),  S(6),  S(6),  S(31),  S(7),  S(16),  S(16),  S(3),  S(3),  
      SR(2), SR(2), S(27), S(12), S(13),  S(24), S(32), S(23), S(20), S(25), S(33),  SR(2), S(3),   S(25),  S(3),  S(3),  
      S(25), S(0),  
    };
    #define A(IDX) IDX
    #define U(IDX) (IDX << 1) + 1
    J_A(ND) const i32_t actions[]{
      A(500),     A(500),     U(-65537),  A(500),     A(50),      A(250),     U(-4),      A(0),      
      A(500),     U(-65542),  A(350),     A(500),     A(500),     A(500),     U(-65546),  U(-11),    
      A(500),     U(-65541),  U(-12),     U(-13),     A(550),     A(500),     A(450),     A(500),    
      A(500),     A(500),     U(-14),     A(500),     U(-15),     U(-16),     U(-17),     A(300),    
      U(-20),     A(500),     A(150),     U(-21),     U(-22),     U(-23),     U(-24),     U(-25),    
      U(-26),     A(500),     A(500),     U(-2),      U(-2),      U(-2),      U(-65563),  A(300),    
      U(-28),     A(500),     A(500),     A(500),     U(-65567),  A(500),     A(500),     U(-65568), 
      U(-65569),  U(89),      A(500),     A(500),     A(500),     A(500),     U(-65570),  A(500),    
      A(100),     U(-65571),  U(-65572),  U(-65573),  U(-11),     U(-38),     U(-11),     U(-39),    
      U(-11),     U(-40),     U(-65577),  U(-65578),  U(-65579),  A(400),     U(-65581),  U(-65582), 
      U(-65583),  U(-65584),  A(100),     U(-49),     U(-50),     U(-51),     U(-52),     U(-65589), 
      U(-65590),  U(-55),     U(-65592),  U(-65593),  U(-65594),  U(-65595),  U(-65596),  U(-65597), 
      U(-65598),  U(-63),     U(-63),     U(-64),     U(-15),     U(-17),     U(-12),     U(-65601), 
      U(-65602),  A(500),     A(500),     A(500),     A(200),     U(-69),     U(-70),     U(-71),    
      U(-72),     U(-73),     U(-65610),  U(-75),     U(-63),     U(-64),     U(-76),     U(-77),    
      U(-69),     A(500),     A(500),     U(-79),     U(-80),    
    };
    #undef A
    #undef U

    // Size: 32
    // Used by: Const, MkVec, SymValRd, IMulConst, IDivConst, IDivConstBy
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

    // Size: 40
    // Used by: IAddConst
    struct alignas(8) J_TYPE_HIDDEN IAddConst final {
      J_A(NI) static void format(const void * J_NOT_NULL data) noexcept {
        const IAddConst & d = *reinterpret_cast<const IAddConst*>(data);
        format_stack_alloc(d.Fx.stack, d.Fx.marker, 2);
        J_DEBUG_INL(" {} ", d.Const);
        format_expr(d.Term);
        J_DEBUG_INL(" {}", d.Sign);
      }

      split_tsa Fx;
      lisp_imm Const = lisp_imm{undefined_v};
      expr* Term = nullptr;
      i8_t Sign = 0;
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
      { "IAddConst", &IAddConst::format  }, // 9
      { "IMulConst", &nt_captures_3::format  }, // 10
      { "IDivConst", &nt_captures_3::format  }, // 11
      { "IDivConstBy", &nt_captures_3::format  }, // 12
      { "AppendInner", &nt_captures_2a::format  }, // 13
      { "MergeVecs", &nt_captures_2a::format  }, // 14
      { "MergedVecs", &nt_captures_2a::format  }, // 15
      { "Progx", &nt_captures_1a::format  }, // 16
      { "Prog", &nt_captures_1a::format  }, // 17
      { "Proge", &nt_captures_1a::format  }, // 18
      { "LAndOrIn", &LAndOrIn::format  }, // 19
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
        // ⋯ ⇛ Any
        // Reduction
        s.reduce_any(-1);
      break;
      case 1:
        // ·$1 ⇛  $1
        // Reduction
        replace(s.stack[-1].e, s.stack->e);
        s.rewind(2, -2);
      break;
      case 2:
        // ·/ $1 ⇛  $1
        // Reduction
        replace(s.stack[0].e, s.iter.e->in_expr(s.iter.index));
        s.rewind(1, -1);
      break;
      case 3:
        // 𝛆 ⇛ LAndOrIn(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\") lisp_imm{undefined_v})
        // Reduction
        s.reduce<LAndOrIn>(0, 19, allocate_split(s.side_fx_stack), allocate(s.collect_stack), lisp_imm{undefined_v});
      break;
      case 4:
        // ⋯$1 ⇛ Const(AllocateSplit(\"s.side_fx_stack\") If($1.Is(lor) False If($1.Is(land) True lisp_imm{nil_v})) nullptr)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, allocate_split(s.side_fx_stack), ((s.stack->e->type == lor) ? lisp_f : (((s.stack->e->type == land) ? lisp_t : (lisp_imm{nil_v})))), nullptr);
      break;
      case 5:
        // ⋯$1 ⇛ Const(AllocateSplit(\"s.side_fx_stack\") $1.ConstVal $1)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, allocate_split(s.side_fx_stack), *(lisp_imm*)(s.stack->e + 1), s.stack->e);
      break;
      case 6:
        // ··/ $1 ⇛  $1
        // Reduction
        replace(s.stack[-1].e, s.iter.e->in_expr(s.iter.index));
        s.rewind(2, -2);
      break;
      case 7:
        // 𝛆 ⇛ Anys
        // Reduction
        s.reduce(0, 1);
      break;
      case 8:
        // ⋯ ⇛ EmptyVec(AllocateSplit(\"s.side_fx_stack\") nullptr)
        // Reduction
        s.reduce<nt_captures_2>(-1, 4, allocate_split(s.side_fx_stack), nullptr);
      break;
      case 9:
        if ((*(id*)(s.stack->e + 1)).raw == 66343) {
          // ⋯$1 ⇛ [$1.SymRdId == '%air:*empty-vec*] EmptyVec(AllocateSplit(\"s.side_fx_stack\") $1)
          // Reduction
          s.reduce<nt_captures_2>(-1, 4, allocate_split(s.side_fx_stack), s.stack->e);
        } else {
          // ⋯$1 ⇛ SymValRd(AllocateSplit(\"s.side_fx_stack\") $1.SymRdId $1)
          // Reduction
          s.reduce<nt_captures_3>(-1, 7, allocate_split(s.side_fx_stack), lisp_imm((*(id*)(s.stack->e + 1))), s.stack->e);
        }
      break;
      case 10:
        // 𝛆 ⇛ Prog(AllocateSplit(\"s.side_fx_stack\"))
        // Reduction
        s.reduce<nt_captures_1a>(0, 17, allocate_split(s.side_fx_stack));
      break;
      case 11: {
        // MkVec($1…3) ⇛  ApplyFx(If($2.IsI64 mk-vec(MkConst($2 $3)) $3) $1)
        // Reduction
        expr* stmt_0 = s.wrap_fx((s.field<lisp_imm>(0, 16).is_i64() ? w.wr<>(et::make_vec, s.mk_const(s.field<lisp_imm>(0, 16), s.field<expr*>(0, 24))) : (s.field<expr*>(0, 24))), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 12: {
        // IDivConstBy($1…3) ⇛  ApplyFx(idiv(const($2) $3) $1)
        // Reduction
        expr* stmt_0 = s.wrap_fx(w.wr<>(et::idiv, w.wrc(s.field<lisp_imm>(0, 16)), s.field<expr*>(0, 24)), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 13: {
        // Const($1…3) ⇛  ApplyFx(MkConst($2 $3) $1)
        // Reduction
        expr* stmt_0 = s.wrap_fx(s.mk_const(s.field<lisp_imm>(0, 16), s.field<expr*>(0, 24)), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 14: {
        if (!(s.field<lisp_imm>(0, 16)).raw) {
          // IAddConst($1·$3 $4) ⇛ [$2 == 0]  ApplyFx(MaybeINeg($4 ApplyPreFx($3 $1)) $1)
          // Reduction
          expr* stmt_0 = s.apply_pre_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          expr* stmt_1 = s.wrap_fx(s.maybe_ineg(s.field<i8_t>(0, 32), stmt_0), s.field<split_tsa>(0, 0));
          s.replace_top(stmt_1);
        } else if ((s.field<i8_t>(0, 32)) == -1 && (i64_t)(s.field<lisp_imm>(0, 16)).raw < 0) {
          // IAddConst($1 $2 $3·) ⇛ [$4 == -1 && $2 < 0]  ineg(iadd(ApplyFx($3 $1) const($2.Neg)))
          // Reduction
          expr* stmt_0 = s.wrap_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          s.replace_top(w.wr<>(et::ineg,
            w.wr<>(et::iadd, stmt_0, w.wrc(lisp_imm{(u64_t)-(s.field<lisp_imm>(0, 16).raw)}))
          ));
        } else if ((s.field<i8_t>(0, 32)) == -1) {
          // IAddConst($1 $2 $3·) ⇛ [$4 == -1]  isub(const($2) ApplyFx($3 $1))
          // Reduction
          expr* stmt_0 = s.wrap_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          s.replace_top(w.wr<>(et::isub, w.wrc(s.field<lisp_imm>(0, 16)), stmt_0));
        } else {
          // IAddConst($1 $2 $3·) ⇛  ApplyFx(If($2 < 0 isub iadd)($3 ApplyPostFxAsPreFx(const(If($2 < 0 $2.Neg $2)) $1)) $1)
          // Reduction
          expr* stmt_0 = s.apply_post_fx_as_pre_fx(w.wrc(((i64_t)(s.field<lisp_imm>(0, 16)).raw < 0 ? lisp_imm{(u64_t)-(s.field<lisp_imm>(0, 16).raw)} : (s.field<lisp_imm>(0, 16)))), s.field<split_tsa>(0, 0));
          expr* stmt_1 = s.wrap_fx(w.wr<>(((i64_t)(s.field<lisp_imm>(0, 16)).raw < 0 ? isub : iadd), s.field<expr*>(0, 24), stmt_0), s.field<split_tsa>(0, 0));
          s.replace_top(stmt_1);
        }
      } break;
      case 15: {
        // IMulConst($1…3) ⇛  ApplyFx(imul($3 ApplyPostFxAsPreFx(const($2) $1)) $1)
        // Reduction
        expr* stmt_0 = s.apply_post_fx_as_pre_fx(w.wrc(s.field<lisp_imm>(0, 16)), s.field<split_tsa>(0, 0));
        expr* stmt_1 = s.wrap_fx(w.wr<>(et::imul, s.field<expr*>(0, 24), stmt_0), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_1);
      } break;
      case 16: {
        // Vec($1 $2) ⇛  ApplyFx($2 $1)
        // Reduction
        expr* stmt_0 = s.wrap_fx(s.field<expr*>(0, 16), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 17:
        // ·$1 / $2 $3 ⇛  If($1.Is(eq) neq eq)($2 $3)
        // Reduction
        replace(s.stack[-1].e, w.wr<>(((s.stack->e->type == eq) ? neq : eq), s.iter.e->in_expr(s.iter.index), s.iter.e->in_expr(s.iter.index + 1)));
        s.rewind(2, -2);
      break;
      case 18:
        // $1·/ $2 ⇛  If($2.Is(lnot to_bool eq neq) || $1.Is(b_if) $2 to-bool($2))
        // Reduction
        replace(s.stack[-1].e, ((((s.iter.e->in_expr(s.iter.index)->type == lnot || s.iter.e->in_expr(s.iter.index)->type == to_bool || s.iter.e->in_expr(s.iter.index)->type == eq || s.iter.e->in_expr(s.iter.index)->type == neq))) || ((s.stack[-1].e->type == b_if)) ? s.iter.e->in_expr(s.iter.index) : (w.wr<>(et::to_bool, s.iter.e->in_expr(s.iter.index)))));
        s.rewind(2, -2);
      break;
      case 19:
        // 𝛆 ⇛ AppendInner(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\"))
        // Reduction
        s.reduce<nt_captures_2a>(0, 13, allocate_split(s.side_fx_stack), allocate(s.collect_stack));
      break;
      case 20: {
        // EmptyVec($1 $2) ⇛  If($2.Empty sym-rd('%air:*empty-vec*) $2).PreFx($1)
        // Reduction
        expr* stmt_0 = s.wrap_pre_fx((!s.field<expr*>(0, 16) ? w.wr<id>(et::sym_val_rd, 66343) : (s.field<expr*>(0, 16))), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 21: {
        // LAndOr($1 $2·$4) ⇛  ApplyFx($4($2) $1)
        // Reduction
        expr* stmt_0 = (w.wr<>(s.field<expr_type>(0, 40), s.field<term_stack_allocation>(0, 16)));
        s.field<term_stack_allocation>(0, 16).release();
        expr* stmt_1 = s.wrap_fx(stmt_0, s.field<split_tsa>(0, 0));
        s.replace_top(stmt_1);
      } break;
      case 22: {
        // IDivConst($1…3) ⇛  ApplyFx(idiv($3 ApplyPostFxAsPreFx(const($2) $1)) $1)
        // Reduction
        expr* stmt_0 = s.apply_post_fx_as_pre_fx(w.wrc(s.field<lisp_imm>(0, 16)), s.field<split_tsa>(0, 0));
        expr* stmt_1 = s.wrap_fx(w.wr<>(et::idiv, s.field<expr*>(0, 24), stmt_0), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_1);
      } break;
      case 23: {
        // Progx($1) ⇛  ApplyProgx($1)
        // Reduction
        expr* stmt_0 = s.wrap_progx(s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 24: {
        // Append($1 $2) ⇛  ApplyFx(Many(vec_append Release($2)) $1)
        // Reduction
        span<expr*> stmt_0 = s.field<term_stack_allocation>(0, 16).release();
        expr* stmt_1 = s.wrap_fx(s.wrap_many(vec_append, stmt_0), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_1);
      } break;
      case 25: {
        // SymValRd($1…3) ⇛  ApplyFx(If($3.Empty sym-rd($2.AsId) $3) $1)
        // Reduction
        expr* stmt_0 = s.wrap_fx((!s.field<expr*>(0, 24) ? w.wr<id>(et::sym_val_rd, id(raw_tag, s.field<lisp_imm>(0, 16).raw)) : (s.field<expr*>(0, 24))), s.field<split_tsa>(0, 0));
        s.replace_top(stmt_0);
      } break;
      case 26:
        // ⋯Blob(0):$1· ⇛ Copy(-1)
        // Reduction
        s.copy_reduce(-1, 1);
      break;
      case 27:
        // ·Const($2 $1·) / $3 $4 ⇛  If($1.IsTruthy $3 $4).PreFx($2)
        // Reduction
        replace(s.stack[-1].e, s.wrap_pre_fx(((bool)s.field<lisp_imm>(0, 16) ? s.iter.e->in_expr(s.iter.index) : (s.iter.e->in_expr(s.iter.index + 1))), s.field<split_tsa>(0, 0)));
        s.rewind(2, -1);
      break;
      case 28: {
        if (!s.field<term_stack_allocation>(1, 16)) {
          // ⋯LAndOrIn($2·$4) $1 ⇛ [$3.Empty] Const($2.AsPreFx() If($4.HasValue $4 If($1.Is(land) True False)) nullptr)
          // Reduction
          s.field<term_stack_allocation>(1, 16).release();
          s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), (!s.field<lisp_imm>(1, 32).is_undefined() ? s.field<lisp_imm>(1, 32) : (((s.stack->e->type == land) ? lisp_t : lisp_f))), nullptr);
        } else if (s.field<term_stack_allocation>(1, 16).size() == 1) {
          // ⋯LAndOrIn($2 $3·)· ⇛ [$3.Single]  $3.First.PreFx($2)
          // Reduction
          s.field<term_stack_allocation>(1, 16).release();
          expr* stmt_0 = s.wrap_pre_fx(s.field<term_stack_allocation>(1, 16)[0], s.field<split_tsa>(1, 0));
          replace(s.stack[-2].e, stmt_0);
          s.reduce_any(-1);
        } else {
          // ⋯LAndOrIn($2 $3 $4) $1 ⇛ LAndOr($2 $3 $4 $1.Type)
          // Reduction
          s.reduce<LAndOr>(-1, 8, s.field<split_tsa>(1, 0), s.field<term_stack_allocation>(1, 16), s.field<lisp_imm>(1, 32), s.stack->e->type);
        }
      } break;
      case 29:
        // ⋯$1· ⇛  $1
        // Reduction
        replace(s.stack[-2].e, s.stack[-1].e);
        s.reduce_any(-1);
      break;
      case 30:
        // ⋯Const($3 $2·) $1 ⇛ Const($3 Fold($1 $2) nullptr)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(1, 16)), nullptr);
      break;
      case 31:
        // ⋯Const($2 $1·)· ⇛ Const($2 $1.Neg nullptr)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), lisp_imm{(u64_t)-(s.field<lisp_imm>(1, 16).raw)}, nullptr);
      break;
      case 32:
        // ⋯IAddConst($1…4)· ⇛ IAddConst($1 $2.Neg $3 $4.Neg)
        // Reduction
        s.reduce<IAddConst>(-1, 9, s.field<split_tsa>(1, 0), lisp_imm{(u64_t)-(s.field<lisp_imm>(1, 16).raw)}, s.field<expr*>(1, 24), (i8_t)-(s.field<i8_t>(1, 32)));
      break;
      case 33:
        // ⋯$1 ⇛ Vec(AllocateSplit(\"s.side_fx_stack\") $1)
        // Reduction
        s.reduce<nt_captures_2>(-1, 5, allocate_split(s.side_fx_stack), s.stack->e);
      break;
      case 34: {
        if (s.field<lisp_imm>(1, 16).is_i64()) {
          // ⋯MkVec($1 $2·)· ⇛ [$2.IsI64] Const($1 $2 nullptr)
          // Reduction
          s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), nullptr);
        } else {
          // ⋯MkVec($1·$3)· ⇛  ApplyFx($3.In(0) $1)
          // Reduction
          expr* stmt_0 = s.wrap_fx(s.field<expr*>(1, 24)->in_expr(0), s.field<split_tsa>(1, 0));
          replace(s.stack[-2].e, stmt_0);
          s.reduce_any(-1);
        }
      } break;
      case 35:
        // ⋯Vec($1 $2)· ⇛ Const($1.AsPreFx() $2.NumInputs nullptr)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), lisp_imm{(u64_t)(s.field<expr*>(1, 16)->num_inputs) << 1}, nullptr);
      break;
      case 36:
        // ⋯EmptyVec($1·)· ⇛ Const($1.AsPreFx() 0 nullptr)
        // Reduction
        s.reduce<nt_captures_3>(-1, 2, s.field<split_tsa>(1, 0).to_pre_fx(), lisp_imm{0}, nullptr);
      break;
      case 37: {
        if ((s.field<i8_t>(0, 32)) == -1 && (i64_t)(s.field<lisp_imm>(0, 16)).raw < 0) {
          // IAddConst($1 $2 $3·) ⇛ [$4 == -1 && $2 < 0]  ineg(iadd(ApplyFx($3 $1) const($2.Neg)))
          // Reduction
          expr* stmt_0 = s.wrap_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          s.replace_top(w.wr<>(et::ineg,
            w.wr<>(et::iadd, stmt_0, w.wrc(lisp_imm{(u64_t)-(s.field<lisp_imm>(0, 16).raw)}))
          ));
        } else if (!(s.field<lisp_imm>(0, 16)).raw) {
          // IAddConst($1·$3 $4) ⇛ [$2 == 0]  ApplyFx(MaybeINeg($4 ApplyPreFx($3 $1)) $1)
          // Reduction
          expr* stmt_0 = s.apply_pre_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          expr* stmt_1 = s.wrap_fx(s.maybe_ineg(s.field<i8_t>(0, 32), stmt_0), s.field<split_tsa>(0, 0));
          s.replace_top(stmt_1);
        } else if ((s.field<i8_t>(0, 32)) == -1) {
          // IAddConst($1 $2 $3·) ⇛ [$4 == -1]  isub(const($2) ApplyFx($3 $1))
          // Reduction
          expr* stmt_0 = s.wrap_fx(s.field<expr*>(0, 24), s.field<split_tsa>(0, 0));
          s.replace_top(w.wr<>(et::isub, w.wrc(s.field<lisp_imm>(0, 16)), stmt_0));
        } else {
          // 𝛆 ⇛ Prog(AllocateSplit(\"s.side_fx_stack\"))
          // Reduction
          s.reduce<nt_captures_1a>(0, 17, allocate_split(s.side_fx_stack));
        }
      } break;
      case 38:
        // Progx:$1 ⇛ Copy(1)
        // Reduction
        s.copy_reduce(1, 0, 16, 18);
      break;
      case 39:
        // $1 ⇛ Proge(AllocFx($1).AsPreFx())
        // Reduction
        s.reduce<nt_captures_1a>(1, 18, allocate_split(s.side_fx_stack, s.stack->e).to_pre_fx());
      break;
      case 40: {
        if ((bool)s.field<lisp_imm>(1, 16)) {
          // ⋯Blob(0):$4· ⇛ [$2.IsTruthy] Copy(-1)
          // Reduction
          s.copy_reduce(-1, 1);
        } else {
          // ⋯Const($1·$3)· ⇛  until(progn(Release($1) $3))
          // Reduction
          split_span stmt_0 = s.field<split_tsa>(1, 0).release();
          replace(s.stack[-2].e, w.wr<>(et::do_until, w.progn(stmt_0, s.field<expr*>(1, 24))));
          s.reduce_any(-1);
        }
      } break;
      case 41:
        if (!(s.field<lisp_imm>(1, 16)).raw) {
          // ⋯Const($3··)· ⇛ [$2 == 0] EmptyVec($3 nullptr)
          // Reduction
          s.reduce<nt_captures_2>(-1, 4, s.field<split_tsa>(1, 0), nullptr);
        } else {
          // ⋯Const($3 $2·) $1 ⇛ MkVec($3 $2 $1)
          // Reduction
          s.reduce<nt_captures_3>(-1, 3, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), s.stack->e);
        }
      break;
      case 42:
        // ⋯$1 ⇛ MkVec(AllocateSplit(\"s.side_fx_stack\") lisp_imm{undefined_v} $1)
        // Reduction
        s.reduce<nt_captures_3>(-1, 3, allocate_split(s.side_fx_stack), lisp_imm{undefined_v}, s.stack->e);
      break;
      case 43: {
        if (!s.field<term_stack_allocation>(1, 16)) {
          // ⋯AppendInner($2·)· ⇛ [$3.Empty] EmptyVec($2.AsPreFx() nullptr)
          // Reduction
          s.field<term_stack_allocation>(1, 16).release();
          s.reduce<nt_captures_2>(-1, 4, s.field<split_tsa>(1, 0).to_pre_fx(), nullptr);
        } else {
          // ⋯AppendInner:$1· ⇛ Copy(-1)
          // Reduction
          s.copy_reduce(-1, 1, 32, 6);
        }
      } break;
      case 44: {
        // ⋯Const($2 $3·) Const($4 $5·) $1 ⇛ Const($2 !+$ $4 Fold($1 $3 $5) nullptr)
        // Reduction
        split_tsa stmt_0 = split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<nt_captures_3>(-1, 2, stmt_0, s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(2, 16), s.field<lisp_imm>(1, 16)), nullptr);
      } break;
      case 45:
        // ⋯Const($1 $2·) $3· ⇛ IMulConst($1 $2 $3)
        // Reduction
        s.reduce<nt_captures_3>(-1, 10, s.field<split_tsa>(2, 0), s.field<lisp_imm>(2, 16), s.stack[-1].e);
      break;
      case 46:
        // ⋯$3 Const($1 $2·)· ⇛ IMulConst($1 $2 $3)
        // Reduction
        s.reduce<nt_captures_3>(-1, 10, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), s.stack[-2].e);
      break;
      case 47: {
        // ·$1·/ $2 $3 ⇛  $2 $3 · /· $1
        // Reduction
        expr* it1 = s.iter.e->in_expr(s.iter.index);
        replace(s.stack[-2].e, s.stack[-1].e);
        s.iter.e->set_input(s.iter.index, s.iter.e->in_expr(s.iter.index + 1));
        s.iter.e->set_input(s.iter.index + 1, it1);
        s.rewind(3, 0);
      } break;
      case 48:
        if (((s.iter.e->type == lor)) == (bool)s.field<lisp_imm>(0, 16)) {
          // LAndOrIn Const($2 $1·) ⇛ [s.iter.e.Is(lor) == $1.IsTruthy] { Val = $1; Fx +=! $2; Trunc(0); Pop 1 }
          // Reduction
          (s.field<lisp_imm>(1, 32)) = (s.field<lisp_imm>(0, 16));
          // Reduction
          (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          // Reduction
          s.iter.e->num_inputs = s.iter.index;
          s.update_lookahead();
          // Reduction
          s.pop(1);
        } else {
          // LAndOrIn Const($2 $1·) ⇛ { Val = $1; Fx +=! $2; Pop 1 }
          // Reduction
          (s.field<lisp_imm>(1, 32)) = (s.field<lisp_imm>(0, 16));
          // Reduction
          (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          // Reduction
          s.pop(1);
        }
      break;
      case 49: {
        if (s.iter.e->type == (s.field<expr_type>(0, 40))) {
          // LAndOrIn LAndOr($4 $3 $2·) ⇛ [s.iter.e.Type == $1] { Val = $2; Collect += $3; Fx +=! $4; Pop 1 }
          // Reduction
          (s.field<lisp_imm>(1, 32)) = (s.field<lisp_imm>(0, 32));
          // Reduction
          (s.field<term_stack_allocation>(1, 16)) += (s.field<term_stack_allocation>(0, 16));
          // Reduction
          (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
          // Reduction
          s.pop(1);
        } else {
          // LAndOrIn LAndOr($4 $3 $2 $1) ⇛ { Val = $2; Collect += ApplyFx($1($3) $4); Pop 1 }
          // Reduction
          (s.field<lisp_imm>(1, 32)) = (s.field<lisp_imm>(0, 32));
          // Reduction
          expr* stmt_0 = s.wrap_fx(w.wr<>(s.field<expr_type>(0, 40), s.field<term_stack_allocation>(0, 16)), s.field<split_tsa>(0, 0));
          (s.field<term_stack_allocation>(1, 16)).push(stmt_0);
          // Reduction
          s.field<term_stack_allocation>(0, 16).release();
          s.pop(1);
        }
      } break;
      case 50:
        // LAndOrIn $1 ⇛ { Collect += ApplyFxClear($1 Fx); Val = lisp_imm{undefined_v}; Pop 1 }
        // Reduction
        (s.field<term_stack_allocation>(1, 16)).push(s.wrap_fx_clear(s.stack->e, s.field<split_tsa>(1, 0)));
        // Reduction
        (s.field<lisp_imm>(1, 32)) = (lisp_imm{undefined_v});
        // Reduction
        s.pop(1);
      break;
      case 51:
        // ·· ⇛ Anys
        // Reduction
        s.reduce(2, 1);
      break;
      case 52:
        // ⋯Const($1 $2·) $3· ⇛ IDivConstBy($1 $2 $3)
        // Reduction
        s.reduce<nt_captures_3>(-1, 12, s.field<split_tsa>(2, 0), s.field<lisp_imm>(2, 16), s.stack[-1].e);
      break;
      case 53:
        // ⋯$3 Const($1 $2·)· ⇛ IDivConst($1 $2 $3)
        // Reduction
        s.reduce<nt_captures_3>(-1, 11, s.field<split_tsa>(1, 0), s.field<lisp_imm>(1, 16), s.stack[-2].e);
      break;
      case 54:
        // ⋯IMulConst:$4(·$2·)· ⇛ { $2 = $2.Neg; Copy(-1) }
        // Reduction
        (s.field<lisp_imm>(1, 16)) = lisp_imm{(u64_t)-(s.field<lisp_imm>(1, 16).raw)};
        // Reduction
        s.copy_reduce(-1, 1);
      break;
      case 55: {
        // ⋯Const($2 $3·) IAddConst($4 $5 $6 $7) $1 ⇛ IAddConst($2 !+$ $4 $3 + $5 * If($1.Is(iadd) 1 -1) $6 $7 * If($1.Is(iadd) 1 -1))
        // Reduction
        split_tsa stmt_0 = split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<IAddConst>(-1, 9, stmt_0, lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * (((s.stack->e->type == iadd) ? 1 : -1)))}).raw}, s.field<expr*>(1, 24), (i8_t)((s.field<i8_t>(1, 32)) * (((s.stack->e->type == iadd) ? 1 : -1))));
      } break;
      case 56:
        // ⋯Const($2 $3·) $4 $1 ⇛ IAddConst($2 $3 $4 If($1.Is(iadd) 1 -1))
        // Reduction
        s.reduce<IAddConst>(-1, 9, s.field<split_tsa>(2, 0), s.field<lisp_imm>(2, 16), s.stack[-1].e, ((s.stack->e->type == iadd) ? (i8_t)1 : (i8_t)-1));
      break;
      case 57: {
        // ⋯IAddConst($2 $3 $4 $5) IAddConst($6 $7 $8 $9) $1 ⇛ IAddConst($2 +=! $6 $3 + $7 * If($1.Is(iadd) 1 -1) If($5 * If($1.Is(iadd) 1 -1) != $9 isub iadd)($4 MidPreFx($8 $2 $6)) $5)
        // Reduction
        expr* stmt_0 = s.apply_mid_pre_fx(s.field<expr*>(1, 24), s.field<split_tsa>(2, 0), s.field<split_tsa>(1, 0));
        split_tsa stmt_1 = (s.field<split_tsa>(2, 0)).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        s.reduce<IAddConst>(-1, 9, stmt_1, lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * (((s.stack->e->type == iadd) ? 1 : -1)))}).raw}, w.wr<>((((i8_t)((s.field<i8_t>(2, 32)) * (((s.stack->e->type == iadd) ? 1 : -1)))) != (s.field<i8_t>(1, 32)) ? isub : iadd), s.field<expr*>(2, 24), stmt_0), s.field<i8_t>(2, 32));
      } break;
      case 58: {
        // ⋯IAddConst($2 $3 $4 $5) Const($6 $7·) $1 ⇛ IAddConst($2 $+! $6 $3 + $7 * If($1.Is(iadd) 1 -1) $4 $5)
        // Reduction
        split_tsa stmt_0 = split_tsa::append_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<IAddConst>(-1, 9, stmt_0, lisp_imm{(s.field<lisp_imm>(2, 16)).raw + (lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * (((s.stack->e->type == iadd) ? 1 : -1)))}).raw}, s.field<expr*>(2, 24), s.field<i8_t>(2, 32));
      } break;
      case 59:
        // ⋯IAddConst($2 $3 $4 $5) $6 $1 ⇛ IAddConst($2 $3 If($5 == If($1.Is(iadd) 1 -1) iadd isub)($4 ApplyPostFxAsPreFx($6 $2)) $5)
        // Reduction
        s.reduce<IAddConst>(-1, 9, s.field<split_tsa>(2, 0), s.field<lisp_imm>(2, 16), w.wr<>(((s.field<i8_t>(2, 32)) == ((s.stack->e->type == iadd) ? 1 : -1) ? iadd : isub),
          s.field<expr*>(2, 24),
          s.apply_post_fx_as_pre_fx(s.stack[-1].e, s.field<split_tsa>(2, 0))
        ), s.field<i8_t>(2, 32));
      break;
      case 60: {
        // ⋯$2 IAddConst($3 $4 $5 $6) $1 ⇛ IAddConst(AllocateSplit(\"s.side_fx_stack\") $4 * If($1.Is(iadd) 1 -1) If($6 == If($1.Is(iadd) 1 -1) iadd isub)($2 ApplyFx($5 $3)) 1)
        // Reduction
        expr* stmt_0 = s.wrap_fx(s.field<expr*>(1, 24), s.field<split_tsa>(1, 0));
        s.reduce<IAddConst>(-1, 9, allocate_split(s.side_fx_stack), lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * (((s.stack->e->type == iadd) ? 1 : -1)))}, w.wr<>(((s.field<i8_t>(1, 32)) == ((s.stack->e->type == iadd) ? 1 : -1) ? iadd : isub), s.stack[-2].e, stmt_0), (i8_t)1);
      } break;
      case 61:
        // ⋯$2 Const($3 $4·) $1 ⇛ IAddConst($3.AsPostFx() $4 * If($1.Is(iadd) 1 -1) $2 1)
        // Reduction
        s.reduce<IAddConst>(-1, 9, s.field<split_tsa>(1, 0).to_post_fx(), lisp_imm{(u64_t)((s.field<lisp_imm>(1, 16)).raw * (((s.stack->e->type == iadd) ? 1 : -1)))}, s.stack[-2].e, (i8_t)1);
      break;
      case 62:
        // Prog Const($1··) ⇛ { Fx +=! $1; Pop 1 }
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        // Reduction
        s.pop(1);
      break;
      case 63:
        // Prog $1 ⇛ { Fx += $1; Pop 1 }
        // Reduction
        (s.field<split_tsa>(1, 0)).push((s.stack->e));
        // Reduction
        s.pop(1);
      break;
      case 64: {
        // ⋯Prog($1) Proge($2)· ⇛ Progx($1 !+$ $2)
        // Reduction
        split_tsa stmt_0 = split_tsa::prepend_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<nt_captures_1a>(-1, 16, stmt_0);
      } break;
      case 65: {
        // ⋯Const($3 $2·) Const($5 $4·) $1 ⇛ Const($3 $+! $5.AsPreFx() Fold($1.Type $2 $4) nullptr)
        // Reduction
        split_tsa stmt_0 = split_tsa::append_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<nt_captures_3>(-1, 2, stmt_0.to_pre_fx(), s.fold_table.fold(s.stack->e->type, s.field<lisp_imm>(2, 16), s.field<lisp_imm>(1, 16)), nullptr);
      } break;
      case 66: {
        // ⋯Proge($1) Prog($2)· ⇛ Progx($1 $+! $2)
        // Reduction
        split_tsa stmt_0 = split_tsa::append_fx((s.field<split_tsa>(2, 0)), (s.field<split_tsa>(1, 0)));
        s.reduce<nt_captures_1a>(-1, 16, stmt_0);
      } break;
      case 67: {
        // AppendInner MergedVecs($1 $2) ⇛ { Collect += vec(Release($2)); Fx +=! $1; Pop 1 }
        // Reduction
        span<expr*> stmt_0 = s.field<term_stack_allocation>(0, 16).release();
        (s.field<term_stack_allocation>(1, 16)).push((w.wr<>(et::vec, stmt_0)));
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        // Reduction
        s.pop(1);
      } break;
      case 68:
        // 𝛆 ⇛ MergeVecs(AllocateSplit(\"s.side_fx_stack\") Allocate(\"s.collect_stack\"))
        // Reduction
        s.reduce<nt_captures_2a>(0, 14, allocate_split(s.side_fx_stack), allocate(s.collect_stack));
      break;
      case 69:
        // AppendInner MkVec($1…3) ⇛ { Collect += If($3.Empty mk-vec(const($2)) $3); Fx +=! $1; Pop 1 }
        // Reduction
        (s.field<term_stack_allocation>(1, 16)).push(((!s.field<expr*>(0, 24) ? w.wr<>(et::make_vec, w.wrc(s.field<lisp_imm>(0, 16))) : (s.field<expr*>(0, 24)))));
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        // Reduction
        s.pop(1);
      break;
      case 70:
        // AppendInner EmptyVec($1·) ⇛ { Fx +=! $1; Pop 1 }
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        // Reduction
        s.pop(1);
      break;
      case 71:
        // AppendInner Append($1 $2) ⇛ { Collect += $2; Fx +=! $1; Pop 1 }
        // Reduction
        (s.field<term_stack_allocation>(1, 16)) += (s.field<term_stack_allocation>(0, 16));
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(0, 0)));
        // Reduction
        s.pop(1);
      break;
      case 72:
        // AppendInner $1 ⇛ { Collect += ApplyFxClear($1 Fx); Pop 1 }
        // Reduction
        (s.field<term_stack_allocation>(1, 16)).push(s.wrap_fx_clear(s.stack->e, s.field<split_tsa>(1, 0)));
        // Reduction
        s.pop(1);
      break;
      case 73: {
        if (((s.field<lisp_imm>(2, 16)).raw == false_v && (s.field<lisp_imm>(1, 16)).raw == true_v) || ((s.field<lisp_imm>(2, 16)).raw == true_v && (s.field<lisp_imm>(1, 16)).raw == false_v)) {
          // ⋯$2 Const(·$3·)·· ⇛ [$3 == False && $6 == True || $3 == True && $6 == False]  If($3 == True to_bool lnot)($2)
          // Reduction
          s.field<split_tsa>(1, 0).release();
          s.field<split_tsa>(2, 0).release();
          replace(s.stack[-4].e, w.wr<>(((s.field<lisp_imm>(2, 16)).raw == true_v ? to_bool : lnot), s.stack[-3].e));
          s.reduce_any(-1);
        } else {
          // ⋯$2 Const($5 $3 $4) Const($8 $6 $7)· ⇛  if($2 MkConst($3 $4 $5) MkConst($6 $7 $8))
          // Reduction
          expr* stmt_0 = s.wrap_pre_fx(s.mk_const(s.field<lisp_imm>(1, 16), s.field<expr*>(1, 24)), s.field<split_tsa>(1, 0));
          expr* stmt_1 = s.wrap_pre_fx(s.mk_const(s.field<lisp_imm>(2, 16), s.field<expr*>(2, 24)), s.field<split_tsa>(2, 0));
          replace(s.stack[-4].e, w.wr<>(et::b_if, s.stack[-3].e, stmt_1, stmt_0));
          s.reduce_any(-1);
        }
      } break;
      case 74:
        // ⋯Prog($1) Const:$3($2··)· ⇛ { $2 +=! $1; Copy(-1) }
        // Reduction
        (s.field<split_tsa>(1, 0)).inplace_concat_fx((s.field<split_tsa>(2, 0)));
        // Reduction
        s.copy_reduce(-1, 1);
      break;
      case 75:
        // ⋯Const:$3($2··) Prog($1)· ⇛ { $2 +=$ $1.AsPreFx(); Copy(-1) }
        // Reduction
        (s.field<split_tsa>(2, 0)).inplace_concat_value(s.field<split_tsa>(1, 0).to_pre_fx());
        // Reduction
        s.copy_reduce(-1, 2);
      break;
      case 76:
        // ⋯IAddConst:$3($2···) Prog($1)· ⇛ { $2 +=! $1; Copy(-1) }
        // Reduction
        (s.field<split_tsa>(2, 0)).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        // Reduction
        s.copy_reduce(-1, 2);
      break;
      case 77:
        // ⋯MergeVecs:$1· ⇛ Copy(-1)
        // Reduction
        s.copy_reduce(-1, 1, 32, 15);
      break;
      case 78:
        // MergeVecs $1 ⇛ { Collect += $1; Pop 1 }
        // Reduction
        (s.field<term_stack_allocation>(1, 16)).push((s.stack->e));
        // Reduction
        s.pop(1);
      break;
      case 79:
        // MergedVecs·MergeVecs($1 $2)· ⇛ { Collect += $2; Fx +=! $1; Pop 3 }
        // Reduction
        (s.field<term_stack_allocation>(3, 16)) += (s.field<term_stack_allocation>(1, 16));
        // Reduction
        (s.field<split_tsa>(3, 0)).inplace_concat_fx((s.field<split_tsa>(1, 0)));
        // Reduction
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
