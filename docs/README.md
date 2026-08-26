# samp-plugin-sdk documentation

This directory documents the legacy SA-MP plugin API exposed by
`samp-plugin-sdk`.

The SDK targets the **32-bit SA-MP plugin ABI** and can be used by both legacy
SA-MP servers and compatible legacy-plugin loaders such as open.mp.

## Documentation

- [Building](build.md) — build the SDK on Windows and Linux with MSVC, GCC,
  Clang, or MinGW-w64.
- [Initialization](initialize.md) — create a plugin, initialize the SDK, access
  the server data table, and shut down cleanly.
- [Plugin callbacks](callbacks.md) — `Supports`, `Load`, `Unload`, `AmxLoad`,
  `AmxUnload`, and `ProcessTick`.
- [Pawn natives](natives.md) — register native functions, read parameters,
  handle strings and arrays, and keep per-AMX state.
- [AMX API](amx.md) — execute Pawn publics, access AMX memory, inspect scripts,
  and use the additional helpers from `amx2.h`.
- [CMake integration](cmake.md) — use `samp::plugin-sdk` and
  `samp_add_plugin()` from another project.

## Headers

Most plugins only need:

```c
#include <amx/amx.h>
#include <plugincommon.h>
```

Include `amx2.h` when using the additional SDK helpers:

```c
#include <amx/amx2.h>
```

`amx2.h` includes `amx.h`, so code using the extended helpers can simply use:

```c
#include <amx/amx2.h>
#include <plugincommon.h>
```

## Platform requirements

The SDK intentionally implements the original 32-bit SA-MP plugin ABI.

Supported targets:

- Windows x86
- Linux x86

Supported compilers:

- MSVC
- GCC
- Clang

MinGW-w64 GCC can be used to build Windows x86 plugins from either Windows or
Linux.

See [build.md](build.md) for complete build instructions.

