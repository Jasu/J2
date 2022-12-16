# J framework

A C++ framework only its creator could love.

## External dependencies

### Runtime dependencies

#### [fontconfig](https://www.freedesktop.org/wiki/Software/fontconfig/) (MIT-licensed)

For finding font files.

#### [FreeType](https://www.freetype.org/) (FTL/GPL-licensed)

For rasterizing fonts. The FTL license is a BSD-style license with a credit clause.

#### [double-conversion](https://github.com/google/double-conversion) (BSD-licensed)

For reading and writing floating-point numbers.

#### [HarfBuzz](https://harfbuzz.github.io/) (MIT-licensed)

For shaping runs of text.

#### [libuv](https://libuv.org) (MIT-licensed)

Event loop.

#### [Termpaint](https://github.com/termpaint/termpaint) (Boost-licensed)

Terminal I/O library.

#### [libunwind](https://www.nongnu.org/libunwind/) (MIT-licensed)

Stack unwinding to print backtraces, and mechanism to register JITted functions
for stack unwinding.

#### [rapidyaml](https://github.com/biojppm/rapidyaml) (MIT-licensed)

YAML parsing.

#### [SPIRV headers](https://github.com/KhronosGroup/SPIRV-Headers) (MIT-licensed)

For generating SPIR-V introspection/disassembly code.

#### [Unicode Database](http://www.unicode.org/versions/Unicode13.0.0/) (Unicode® Copyright and Terms of Use)

For text segmentation.

#### [Vulkan](https://www.khronos.org/vulkan/) (*Apache 2.0-licensed)

For rendering. The license depends on the actual implementation used.

#### [Zydis](https://zydis.re/) (MIT-licensed)

X86-64 disassembler, for printing disassembly of JITted code.

#### [xcb](https://xcb.freedesktop.org/) (MIT-licensed)

For interacting with X11. `xcb-xinput`, `xcb-xkb`, and `xcb-errors` are required.

#### [xkbcommon](https://xkbcommon.org/) (MIT/X11-derivative-licensed)

For interpreting key codes. `xkbcommon-x11` is also required.

### Build-time dependencies

#### A C/C++-compiler, linker, and a standard library

Currently, only Clang is supported.

#### [AsmDB](https://github.com/asmjit/asmdb) (Unlicense-licensed)

For generating assembler data.

#### [jq](https://stedolan.github.io/jq/) (MIT-licensed)

For generating various files during build.

#### [Meson](https://mesonbuild.com/) (Apache 2.0-licensed)

The build system.

#### [Ninja](https://ninja-build.org/) (Apache 2.0-licensed)

The backend of the build system.

#### [Node.JS](https://nodejs.org/en/) (MIT-licensed)

For generating assembler data.

#### [Python 3](https://www.python.org/) (PSF license agreement)

For generating various files during build.

#### [shaderc](https://github.com/google/shaderc) (Apache 2.0-licensed)

For compiling GLSL shaders.

#### [re2c](https://re2c.org/) (Public domain)

For generating parsers.

### Test dependencies (optional)

The following dependencies are only required for compiling tests and benchmarks.

#### [doctest](https://github.com/onqtam/doctest) (MIT-licensed)

Faster version of Catch 2.

#### [NASM](https://nasm.us/) (BSD-licensed)

Netwide Assembler, for generating test fixtures for the assembler.
