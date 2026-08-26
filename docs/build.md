# Building

`samp-plugin-sdk` targets the legacy SA-MP plugin ABI and is **x86 (32-bit) only**.

Supported platforms:

- Windows x86
- Linux x86

Supported compilers:

- MSVC
- GCC
- Clang
- MinGW-w64 GCC for Windows cross-compilation

The project requires **CMake 3.21 or newer**.

## Build outputs

The SDK itself is built as a static library:

- MSVC: `samp-plugin-sdk.lib`
- GCC / Clang / MinGW: `libsamp-plugin-sdk.a`

The example plugin is disabled by default.

To enable it, pass:

```text
-DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
```

The example is built as a loadable plugin:

- Windows: `samp-plugin-example.dll`
- Linux: `samp-plugin-example.so`

---

## Windows

### MSVC

Requirements:

- Visual Studio 2022
- Desktop development with C++
- CMake

Using the provided preset:

```bat
cmake --preset msvc-x86
cmake --build --preset msvc-x86
```

To build the example too:

```bat
cmake --preset msvc-x86 -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
cmake --build --preset msvc-x86
```

Without presets:

```bat
cmake -S . -B build/msvc-x86 ^
    -G "Visual Studio 17 2022" ^
    -A Win32

cmake --build build/msvc-x86 --config Release
```

### MinGW-w64 GCC

Requirements:

- 32-bit MinGW-w64 GCC
- `i686-w64-mingw32-gcc` available in `PATH`
- Ninja
- CMake

Using the provided preset:

```bat
cmake --preset mingw-x86
cmake --build --preset mingw-x86
```

To build the example too:

```bat
cmake --preset mingw-x86 -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
cmake --build --preset mingw-x86
```

Without presets:

```bat
cmake -S . -B build/mingw-x86 ^
    -G Ninja ^
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-x86.cmake ^
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/mingw-x86
```

### Clang / ClangCL

The recommended Windows Clang configuration uses the Visual Studio generator with the ClangCL toolset.

Requirements:

- Visual Studio 2022
- Desktop development with C++
- LLVM / Clang tools for Visual Studio

Configure and build:

```bat
cmake -S . -B build/clang-x86 ^
    -G "Visual Studio 17 2022" ^
    -A Win32 ^
    -T ClangCL

cmake --build build/clang-x86 --config Release
```

To build the example too:

```bat
cmake -S . -B build/clang-x86 ^
    -G "Visual Studio 17 2022" ^
    -A Win32 ^
    -T ClangCL ^
    -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON

cmake --build build/clang-x86 --config Release
```

---

## Linux

The Linux build must have 32-bit development libraries installed.

The examples below use Debian/Ubuntu package names.

### GCC

Install the required packages:

```bash
sudo apt install cmake ninja-build gcc-multilib libc6-dev-i386
```

Using the provided preset:

```bash
CC=gcc cmake --preset linux-x86
cmake --build --preset linux-x86
```

To build the example too:

```bash
CC=gcc cmake --preset linux-x86 -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
cmake --build --preset linux-x86
```

Without presets:

```bash
CC=gcc cmake -S . -B build/linux-gcc-x86 \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x86.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/linux-gcc-x86
```

### Clang

Install the required packages:

```bash
sudo apt install cmake ninja-build clang libc6-dev-i386
```

Using the provided preset:

```bash
CC=clang cmake --preset linux-x86
cmake --build --preset linux-x86
```

To build the example too:

```bash
CC=clang cmake --preset linux-x86 -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
cmake --build --preset linux-x86
```

Without presets:

```bash
CC=clang cmake -S . -B build/linux-clang-x86 \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-x86.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/linux-clang-x86
```

> Do not reuse the same configured build directory when switching between GCC and Clang. CMake caches the selected compiler.

### MinGW-w64 GCC — build Windows x86 from Linux

Install the cross-compiler:

```bash
sudo apt install cmake ninja-build gcc-mingw-w64-i686
```

Using the provided preset:

```bash
cmake --preset mingw-x86
cmake --build --preset mingw-x86
```

To build the example too:

```bash
cmake --preset mingw-x86 -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
cmake --build --preset mingw-x86
```

Without presets:

```bash
cmake -S . -B build/mingw-x86 \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/mingw-x86.cmake \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/mingw-x86
```

This produces Windows x86 binaries using `i686-w64-mingw32-gcc`.

---

## Using the SDK from another CMake project

The simplest setup is to add the SDK as a subdirectory:

```cmake
add_subdirectory(third-party/samp-plugin-sdk)

target_link_libraries(my-plugin PRIVATE samp::plugin-sdk)
```

For a SA-MP plugin, the SDK also provides `samp_add_plugin()`:

```cmake
samp_add_plugin(my-plugin
    AMX_NATIVES
    PROCESS_TICK
    SOURCES
        src/plugin.c
        src/natives.c
)
```

`AMX_NATIVES` adds the `AmxLoad` and `AmxUnload` exports.

`PROCESS_TICK` adds the `ProcessTick` export.

On Windows, the required legacy SA-MP exports are generated automatically through a `.def` file. No manually maintained `plugin.def` is required.

---

## Install

The SDK can also be installed as a CMake package:

```bash
cmake --install build/linux-x86 --prefix /path/to/install
```

A consuming project can then use:

```cmake
find_package(samp-plugin-sdk CONFIG REQUIRED)

target_link_libraries(my-plugin PRIVATE samp::plugin-sdk)
```

`samp_add_plugin()` is available from the installed package as well.

