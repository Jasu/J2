# Architecture difference notes

Notes for porting to different platforms in the future

## Conditional branches

### AMD64 vs. Aarch64

#### Flag registers:

|          | AMD64          | Aarch64        |
| Sign     | `S` (1 == neg) | `N` (1 == neg) |
| Zero     | `Z`            | `Z`            |
| Carry    | `C`            | `C`            |
| Overflow | `O`            | `V`            |
| Parity   | `P`            | N/A            |

#### Condition codes

|                 | AMD64 mnemonic   | Aarch64 mnemonic | Description                                                |
| `Z`             | `EQ`, `Z`        | `EQ`             | Zero or equal                                              |
| `!Z`            | `NE`, `NZ`       | `NE`             | Non-zero or not equal                                      |
| `C`             | `C`, `B`         | `CS`, `HS`       | Carry or below         (Note inverted mnemonic in Aarch64) |
| `!C`            | `NC`, `NB`, `AE` | `CC`, `LO`       | Not carry or not below (Note inverted mnemonic in Aarch64) |
| `!C and !Z`     | `A`, `NBE`       | N/A              |                                                            |
| `C or Z`        | `BE`, `NA`       | N/A              |                                                            |
| `S`             | `S`              | `MI`             |                                                            |
| `!S`            | `NS`             | `PL`             |                                                            |
| `O`             | `O`              | `VS`             |                                                            |
| `!O`            | `NO`             | `VC`             |                                                            |
| `C and !Z`      | ``               | `HI`             |                                                            |
| `!C or Z`       | ``               | `LS`             |                                                            |
| `S == O`        | `GE`, `NL`       | `GE`             |                                                            |
| `S != O`        | `NGE`, `L`       | `LT`             |                                                            |
| `!Z and S == O` | `NLE`, `G`       | `GT`             |                                                            |
| `Z or S != O`   | `LE`, `NG`       | `LE`             |                                                            |

## Addressing modes

|              | AMD64                                          | Aarch64                                                                             |
| RIP-relative | Up to 64-bit                                   | +-1 MB, 32 bits with two instructions. DWORD-aligned.                               |
| Indexed      | Any reg by any reg, scale up to 8, 32-bit disp | Base + 64-bit scaled index, or base + 9/12-bit disp, or pre/post-indexed 9-bit disp |
