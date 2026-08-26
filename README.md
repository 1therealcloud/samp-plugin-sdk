# samp-plugin-sdk

A cleaned-up and modernized version of the legacy SA-MP Plugin SDK, while preserving the original x86 plugin ABI and AMX compatibility.

## Changes from the original SDK

- Rewritten the SDK core from C++ to C.
- Removed obsolete compiler, platform, and runtime compatibility code.
- Cleaned up and modernized the AMX headers and helper functions.
- Added support for modern MSVC, GCC, Clang, and MinGW-w64 toolchains.
- Added compile-time checks for the legacy 32-bit SA-MP / AMX ABI.
- Added modern CMake support for building and integrating the SDK.
- Added automatic Windows plugin export generation.
- Added a documented example plugin.
- Added separate documentation for initialization, callbacks, natives, AMX, CMake, and building.

## Documentation

See [docs/README.md](docs/README.md) for the full documentation.

## License

See [LICENSE](LICENSE).

