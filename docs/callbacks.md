# Plugin callbacks

The legacy SA-MP plugin interface consists of exported functions called by the
server during the plugin and AMX lifecycle.

The complete callback surface is:

```c
Supports
Load
Unload
AmxLoad
AmxUnload
ProcessTick
```

## Lifecycle

A typical lifecycle is:

```text
server starts
    |
    +-- Supports()
    |
    +-- Load(ppData)
    |
    +-- AmxLoad(gamemode)
    +-- AmxLoad(filterscript)
    +-- ...
    |
    +-- ProcessTick()
    +-- ProcessTick()
    +-- ...
    |
    +-- AmxUnload(filterscript)
    +-- AmxUnload(gamemode)
    |
    +-- Unload()
```

Scripts can also be loaded or unloaded while the server is running, so
`AmxLoad()` and `AmxUnload()` may be called multiple times.

Do not assume there is only one `AMX` instance.

## `Supports`

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void);
```

Returns the legacy plugin version combined with feature flags.

Example:

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION
        | SUPPORTS_AMX_NATIVES
        | SUPPORTS_PROCESS_TICK;
}
```

Only advertise a feature when the corresponding callback is actually
implemented and exported.

## `Load`

```c
PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData);
```

Called once when the plugin is loaded.

Use it to:

- initialize `pAMXFunctions`;
- obtain `logprintf`;
- obtain the call-public callbacks if needed;
- initialize plugin-global state.

Return non-zero on success and zero on failure.

Example:

```c
PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL)
        return 0;

    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    return pAMXFunctions != NULL;
}
```

See [initialize.md](initialize.md) for the complete data table.

## `Unload`

```c
PLUGIN_EXPORT void PLUGIN_CALL Unload(void);
```

Called before the plugin library is unloaded.

Use it for plugin-global cleanup.

Do not free per-AMX state here if it can be cleaned up correctly from
`AmxUnload()`.

## `AmxLoad`

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx);
```

Called for every gamemode or filterscript AMX loaded by the server.

The common use is registering native functions:

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    return amx_Register(amx, natives, -1);
}
```

`amx_Register()` returns an AMX error code, so it can be returned directly.

This callback is also a good place to allocate per-script state with
`amx_SetUserData()`.

See [natives.md](natives.md).

## `AmxUnload`

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx);
```

Called before an AMX is destroyed.

A plugin with no per-AMX state can simply return:

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    (void)amx;
    return AMX_ERR_NONE;
}
```

If the plugin stored userdata on the AMX, release it here.

Example:

```c
#define MY_TAG 0x4d595441L

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    void *ptr = NULL;

    if (amx_GetUserData(amx, MY_TAG, &ptr) == AMX_ERR_NONE)
    {
        free(ptr);
        amx_SetUserData(amx, MY_TAG, NULL);
    }

    return AMX_ERR_NONE;
}
```

## `ProcessTick`

```c
PLUGIN_EXPORT void PLUGIN_CALL ProcessTick(void);
```

Called once per server processing tick when
`SUPPORTS_PROCESS_TICK` is advertised.

Example:

```c
static unsigned int ticks;

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick(void)
{
    ++ticks;
}
```

Keep `ProcessTick()` short. Long blocking operations delay the server loop.

For expensive work, process small queued pieces per tick or use a worker
thread and synchronize the result back to the server thread.

## Calling Pawn callbacks

There are two ways to call Pawn publics.

### Server call-public callbacks

`Load()` exposes:

```c
PLUGIN_DATA_CALLPUBLIC_FS
PLUGIN_DATA_CALLPUBLIC_GM
```

They call a public by name with no parameters:

```c
typedef int (*callpublic_t)(char *name);

static callpublic_t callPublicGameMode;

callPublicGameMode("OnPluginEvent");
```

### AMX API

For a specific `AMX` instance, use:

```c
amx_FindPublic
amx_Push
amx_PushString
amx_PushArray
amx_Exec
```

This supports parameters and return values.

Example:

```c
int index;
cell retval = 0;

if (amx_FindPublic(amx, "OnPluginValue", &index) == AMX_ERR_NONE)
{
    amx_Push(amx, 123);
    amx_Exec(amx, &retval, index);
}
```

More examples are available in [amx.md](amx.md).

## Windows exports

Windows x86 uses `__stdcall`, which normally decorates function names.

SA-MP looks up exact undecorated names such as:

```text
Supports
Load
Unload
AmxLoad
AmxUnload
ProcessTick
```

For this reason `PLUGIN_EXPORT` does not use `__declspec(dllexport)`.

When using `samp_add_plugin()`, the SDK's CMake integration generates the
module definition file automatically.

See [cmake.md](cmake.md).

