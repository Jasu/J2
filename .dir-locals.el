((c++-mode . ((eval . (config/set-projectile-relative-include-paths "src"
                                                                    "ext/termpaint"
                                                                    "ext/double-conversion/double-conversion"
                                                                    "subprojects/zydis/include"
                                                                    "subprojects/zydis/dependencies/zycore/include"
                                                                    "build/subprojects/zydis/__CMake_build"
                                                                    "build/subprojects/zydis/__CMake_build/zycore"
                                                                    "subprojects/rapidyaml/src"
                                                                    "subprojects/rapidyaml"
                                                                    "subprojects/rapidyaml/ext/c4core/src"
                                                                    "build/generated"
                                                                    "subprojects/rapidyaml/ext/c4core/ext"
                                                                    "/usr/include/freetype2"))
              (config/src-roots . (("src" . "j")))
              (flycheck-clang-warnings . ("all"
                                          "extra"
                                          "extra-semi"
                                          "unreachable-code"
                                          "unused-label"
                                          "no-pragma-once-outside-header"
                                          "no-potentially-evaluated-expression"
                                          "no-c99-designator"))
              (flycheck-idle-change-delay . 3.0)
              (flycheck-relevant-error-other-file-minimum-level . warning))))
