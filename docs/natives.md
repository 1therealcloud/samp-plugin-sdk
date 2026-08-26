# Pawn natives

A Pawn native is a C or C++ function callable from a Pawn script.

Example Pawn declaration:

```pawn
native Example_Add(a, b);
```

Matching C function:

```c
static cell AMX_NATIVE_CALL Example_Add(AMX *amx, cell *params)
{
    (void)amx;
    return params[1] + params[2];
}
```

## Native function signature

Every native uses:

```c
cell AMX_NATIVE_CALL NativeName(AMX *amx, cell *params);
```

Arguments:

- `amx` — the script instance that called the native;
- `params` — argument data supplied by Pawn.

The return value is a Pawn `cell`.

## Parameter layout

`params[0]` is the number of parameter **bytes**, not the number of arguments.

Actual arguments begin at `params[1]`.

For:

```pawn
native Example_Add(a, b);
```

the layout is:

```text
params[0] = 2 * sizeof(cell)
params[1] = a
params[2] = b
```

A simple argument-count check:

```c
static int HasParams(const cell *params, int count)
{
    return params != NULL
        && params[0] >= (cell)(count * (int)sizeof(cell));
}
```

Then:

```c
static cell AMX_NATIVE_CALL Example_Add(AMX *amx, cell *params)
{
    (void)amx;

    if (!HasParams(params, 2))
        return 0;

    return params[1] + params[2];
}
```

## Registering natives

Create a null-terminated `AMX_NATIVE_INFO` table:

```c
static AMX_NATIVE_INFO natives[] =
{
    { "Example_Add", Example_Add },
    { "Example_Print", Example_Print },
    { NULL, NULL }
};
```

Register it from `AmxLoad()`:

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    return amx_Register(amx, natives, -1);
}
```

Advertise native support:

```c
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}
```

And when using the CMake helper:

```cmake
samp_add_plugin(my-plugin
    AMX_NATIVES
    SOURCES
        plugin.c
)
```

## Reading Pawn strings

The simplest C helper is `amx_GetCStringEx()` from `amx2.h`.

Pawn:

```pawn
native Example_Print(const text[]);
```

C:

```c
#include <stdlib.h>
#include <amx/amx2.h>

static cell AMX_NATIVE_CALL Example_Print(AMX *amx, cell *params)
{
    char *text = NULL;

    if (amx_GetCStringEx(amx, params[1], &text) != AMX_ERR_NONE)
        return 0;

    logprintf("%s", text);

    free(text);
    return 1;
}
```

`amx_GetCStringEx()` allocates the returned string with `malloc()`.

The caller must free it.

### C++ strings

When compiling C++, `amx2.h` also provides:

```cpp
std::string amx_GetCppString(AMX *amx, cell param);
int amx_SetCppString(
    AMX *amx,
    cell param,
    const std::string &str,
    size_t maxlen
);
```

Example:

```cpp
std::string text = amx_GetCppString(amx, params[1]);
```

## Writing Pawn strings

Use:

```c
int amx_SetCString(
    AMX *amx,
    cell param,
    const char *str,
    int len
);
```

Pawn:

```pawn
native Example_GetName(output[], size);
```

C:

```c
static cell AMX_NATIVE_CALL Example_GetName(AMX *amx, cell *params)
{
    static const char name[] = "example";

    if (params[2] <= 0)
        return 0;

    return amx_SetCString(
        amx,
        params[1],
        name,
        (int)params[2]
    ) == AMX_ERR_NONE;
}
```

## Accessing Pawn arrays and references

Pawn arrays and by-reference arguments are AMX addresses.

Convert an AMX address into a physical C pointer with `amx_GetAddr()`:

```c
cell *values = NULL;

if (amx_GetAddr(amx, params[1], &values) != AMX_ERR_NONE)
    return 0;

values[0] = 100;
values[1] = 200;
```

For a Pawn declaration such as:

```pawn
native Example_Fill(values[], count);
```

always validate `count` before accessing the buffer.

The AMX API does not know the logical Pawn array length for you.

## Raising an AMX error

A native can report an AMX runtime error with:

```c
amx_RaiseError(amx, AMX_ERR_NATIVE);
```

Example:

```c
static cell AMX_NATIVE_CALL Example_Fail(AMX *amx, cell *params)
{
    (void)params;

    amx_RaiseError(amx, AMX_ERR_NATIVE);
    return 0;
}
```

Common errors include:

```text
AMX_ERR_NONE
AMX_ERR_NATIVE
AMX_ERR_MEMORY
AMX_ERR_MEMACCESS
AMX_ERR_NOTFOUND
AMX_ERR_PARAMS
AMX_ERR_GENERAL
```

Prefer returning a useful Pawn value for ordinary validation failures and
reserve `amx_RaiseError()` for conditions that should be reported as an AMX
runtime error.

## Per-AMX state

A server can have multiple AMX instances.

Do not store state that belongs to one script in a single global variable.

The AMX structure provides user-data slots:

```c
amx_SetUserData
amx_GetUserData
```

Example:

```c
#define MY_STATE_TAG 0x4d595354L

typedef struct MyState
{
    unsigned int calls;
} MyState;

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    MyState *state = calloc(1, sizeof(*state));

    if (state == NULL)
        return AMX_ERR_MEMORY;

    if (amx_SetUserData(amx, MY_STATE_TAG, state) != AMX_ERR_NONE)
    {
        free(state);
        return AMX_ERR_USERDATA;
    }

    return amx_Register(amx, natives, -1);
}
```

Read it from a native:

```c
static MyState *GetState(AMX *amx)
{
    void *ptr = NULL;

    if (amx_GetUserData(amx, MY_STATE_TAG, &ptr) != AMX_ERR_NONE)
        return NULL;

    return (MyState *)ptr;
}
```

Release it in `AmxUnload()`:

```c
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    MyState *state = GetState(amx);

    if (state != NULL)
    {
        amx_SetUserData(amx, MY_STATE_TAG, NULL);
        free(state);
    }

    return AMX_ERR_NONE;
}
```

## Redirecting a native

`amx_Redirect()` can replace the address of a native already linked into an AMX
instance.

```c
AMX_NATIVE original = NULL;

amx_Redirect(
    amx,
    "SomeNative",
    (ucell)(uintptr_t)ReplacementNative,
    &original
);
```

Restore it later:

```c
if (original != NULL)
{
    amx_Redirect(
        amx,
        "SomeNative",
        (ucell)(uintptr_t)original,
        NULL
    );
}
```

This is a low-level feature. Redirects are per-AMX and should be restored before
the relevant AMX is destroyed.

## Native implementation checklist

A native should generally:

1. validate its parameter count;
2. validate lengths and counts before accessing buffers;
3. convert Pawn addresses with `amx_GetAddr()`;
4. free strings returned by `amx_GetCStringEx()`;
5. return useful values to Pawn;
6. avoid blocking the server thread;
7. keep state per `AMX` when the state belongs to a script.

