# Initialization

A legacy SA-MP plugin is a dynamic library containing a small set of entry
points that the server loads by name.

The minimum useful plugin implements:

```c
Supports
Load
Unload
```

Plugins that register Pawn natives also implement:

```c
AmxLoad
AmxUnload
```

Plugins that need per-server-tick processing additionally implement:

```c
ProcessTick
```

The CMake helper provided by this SDK exports the correct entry points
automatically. See [cmake.md](cmake.md).

## Headers

A normal C plugin starts with:

```c
#include <amx/amx.h>
#include <plugincommon.h>
```

Use the extended helpers when needed:

```c
#include <amx/amx2.h>
#include <plugincommon.h>
```

## `Supports`

`Supports()` describes which parts of the legacy plugin API the plugin uses.

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION;
}
```

A plugin that registers Pawn natives:

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION
        | SUPPORTS_AMX_NATIVES;
}
```

A plugin that also implements `ProcessTick()`:

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION
        | SUPPORTS_AMX_NATIVES
        | SUPPORTS_PROCESS_TICK;
}
```

Available flags:

| Flag | Meaning |
| --- | --- |
| `SUPPORTS_VERSION` | Required legacy plugin ABI version (`0x0200`) |
| `SUPPORTS_VERSION_MASK` | Mask for extracting the ABI version |
| `SUPPORTS_AMX_NATIVES` | Plugin implements `AmxLoad` / `AmxUnload` |
| `SUPPORTS_PROCESS_TICK` | Plugin implements `ProcessTick` |

## `Load`

`Load()` is called once when the server loads the plugin.

```c
PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData);
```

The server passes a legacy data table through `ppData`.

The most important rule is:

> Set `pAMXFunctions` before calling any forwarded `amx_*` function.

Example:

```c
typedef void (*logprintf_t)(const char *format, ...);

static logprintf_t logprintf;

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL)
        return 0;

    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    if (logprintf == NULL || pAMXFunctions == NULL)
        return 0;

    logprintf("plugin loaded");
    return 1;
}
```

Returning zero tells the server that plugin initialization failed.

## Server data table

The public legacy entries are:

| Entry | Value | Description |
| --- | ---: | --- |
| `PLUGIN_DATA_LOGPRINTF` | `0x00` | Server logging function |
| `PLUGIN_DATA_AMX_EXPORTS` | `0x10` | AMX function table used by the SDK |
| `PLUGIN_DATA_CALLPUBLIC_FS` | `0x11` | Call a no-argument public in filterscripts |
| `PLUGIN_DATA_CALLPUBLIC_GM` | `0x12` | Call a no-argument public in the gamemode |

The call-public callbacks have the legacy shape:

```c
typedef int (*callpublic_t)(char *name);
```

Example:

```c
static callpublic_t callPublicGameMode;

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL)
        return 0;

    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    callPublicGameMode =
        (callpublic_t)ppData[PLUGIN_DATA_CALLPUBLIC_GM];

    return pAMXFunctions != NULL;
}
```

Later:

```c
if (callPublicGameMode != NULL)
    callPublicGameMode("OnMyPluginEvent");
```

These callbacks only pass the public name. For parameters and return values,
use the AMX API described in [amx.md](amx.md).

## Legacy SA-MP internal entries

The SDK keeps the historical entries:

```c
PLUGIN_DATA_NETGAME
PLUGIN_DATA_RAKSERVER
PLUGIN_DATA_LOADFSCRIPT
PLUGIN_DATA_CONSOLE
PLUGIN_DATA_UNLOADFSCRIPT
```

These are **not a stable public API**.

They expose implementation details of the original SA-MP server and require
external private type definitions to use safely. Compatible servers are not
required to implement them; open.mp leaves these slots null.

New code should not depend on them.

If old code must inspect one, always check for null first:

```c
void *rakserver = ppData[PLUGIN_DATA_RAKSERVER];

if (rakserver != NULL)
{
    /* SA-MP-specific legacy code */
}
```

## `Unload`

`Unload()` is called when the plugin is unloaded.

```c
PLUGIN_EXPORT void PLUGIN_CALL Unload(void)
{
    if (logprintf != NULL)
        logprintf("plugin unloaded");
}
```

Release plugin-global resources here.

Per-script resources should normally be released in `AmxUnload()` instead.

## Complete minimal plugin

```c
#include <amx/amx.h>
#include <plugincommon.h>

typedef void (*logprintf_t)(const char *format, ...);

static logprintf_t logprintf;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION;
}

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL)
        return 0;

    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    if (logprintf == NULL || pAMXFunctions == NULL)
        return 0;

    logprintf("example plugin loaded");
    return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload(void)
{
    if (logprintf != NULL)
        logprintf("example plugin unloaded");
}
```

For a plugin that exposes Pawn natives, continue with
[natives.md](natives.md).

