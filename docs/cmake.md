# CMake integration

`samp-plugin-sdk` provides:

```text
samp-plugin-sdk
samp::plugin-sdk
```

and the convenience function:

```cmake
samp_add_plugin()
```

The recommended integration is to add the SDK as a subdirectory and use
`samp_add_plugin()` for the actual plugin module.

## Add the SDK

For example, with the SDK stored at:

```text
third-party/samp-plugin-sdk/
```

use:

```cmake
add_subdirectory(third-party/samp-plugin-sdk)
```

The public target is:

```cmake
samp::plugin-sdk
```

For an ordinary target:

```cmake
target_link_libraries(my-target
    PRIVATE
        samp::plugin-sdk
)
```

The SDK automatically exposes its include root, so code can use:

```c
#include <plugincommon.h>
#include <amx/amx.h>
#include <amx/amx2.h>
```

## `samp_add_plugin`

The easiest way to create a plugin is:

```cmake
samp_add_plugin(my-plugin
    SOURCES
        src/plugin.c
)
```

For a plugin that registers Pawn natives:

```cmake
samp_add_plugin(my-plugin
    AMX_NATIVES
    SOURCES
        src/plugin.c
        src/natives.c
)
```

For a plugin that also uses `ProcessTick`:

```cmake
samp_add_plugin(my-plugin
    AMX_NATIVES
    PROCESS_TICK
    SOURCES
        src/plugin.c
        src/natives.c
)
```

## Options

### `AMX_NATIVES`

```cmake
AMX_NATIVES
```

Declares that the plugin implements:

```text
AmxLoad
AmxUnload
```

and causes those entry points to be exported on Windows.

The plugin should also advertise:

```c
SUPPORTS_AMX_NATIVES
```

from `Supports()`.

### `PROCESS_TICK`

```cmake
PROCESS_TICK
```

Declares that the plugin implements:

```text
ProcessTick
```

and causes it to be exported on Windows.

The plugin should also advertise:

```c
SUPPORTS_PROCESS_TICK
```

from `Supports()`.

## Automatic plugin properties

`samp_add_plugin()` creates a CMake `MODULE` library and automatically:

- links `samp::plugin-sdk`;
- removes the normal `lib` prefix;
- hides non-exported symbols;
- generates the required Windows module-definition file.

A target such as:

```cmake
samp_add_plugin(example
    AMX_NATIVES
    PROCESS_TICK
    SOURCES plugin.c
)
```

produces a plugin named approximately:

```text
example.dll
```

on Windows and:

```text
example.so
```

on Linux.

## Windows exports

The legacy Windows SA-MP ABI uses `__stdcall`.

SA-MP loads entry points by exact undecorated names, so exporting functions
with ordinary `__declspec(dllexport)` is not sufficient for a portable x86
build.

`samp_add_plugin()` generates a `.def` file automatically.

Base exports:

```text
Supports
Load
Unload
```

With `AMX_NATIVES`:

```text
AmxLoad
AmxUnload
```

With `PROCESS_TICK`:

```text
ProcessTick
```

No manually maintained `plugin.def` is required when using
`samp_add_plugin()`.

## Building the SDK directly

The example plugin is disabled by default.

Configure only the SDK:

```bash
cmake -S . -B build
```

Enable the example explicitly:

```bash
cmake -S . -B build \
    -DSAMP_PLUGIN_SDK_BUILD_EXAMPLE=ON
```

For platform-specific x86 commands, see [build.md](build.md).

## CMake presets

The repository provides presets for common builds:

```text
linux-x86
mingw-x86
msvc-x86
```

Example:

```bash
cmake --preset mingw-x86
cmake --build --preset mingw-x86
```

See [build.md](build.md) for compiler and dependency details.

## Installing the SDK

When installation support is enabled, the SDK installs:

- the static library;
- `plugincommon.h`;
- `amx/amx.h`;
- `amx/amx2.h`;
- CMake package files;
- `SampPlugin.cmake`.

Then another project can use:

```cmake
find_package(samp-plugin-sdk CONFIG REQUIRED)

target_link_libraries(my-target
    PRIVATE
        samp::plugin-sdk
)
```

The installed package also exposes `samp_add_plugin()`.

## Recommended plugin project

A small plugin can use:

```text
my-plugin/
├── CMakeLists.txt
├── src/
│   ├── plugin.c
│   └── natives.c
└── third-party/
    └── samp-plugin-sdk/
```

`CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.21)

project(my-plugin LANGUAGES C)

add_subdirectory(third-party/samp-plugin-sdk)

samp_add_plugin(my-plugin
    AMX_NATIVES
    PROCESS_TICK
    SOURCES
        src/plugin.c
        src/natives.c
)
```

That is enough for all SDK include paths, the static SDK implementation, and
the platform-specific plugin exports.

