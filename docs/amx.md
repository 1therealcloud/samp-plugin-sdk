# AMX API

`samp-plugin-sdk` exposes the legacy SA-MP AMX API through the server-provided
AMX export table.

`Load()` must initialize:

```c
pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
```

before any forwarded `amx_*` function is used.

The SDK exposes all 44 legacy AMX export slots.

## Common operations

Most plugins only need a small subset:

```text
amx_Register
amx_GetAddr
amx_GetString
amx_SetString
amx_StrLen
amx_FindPublic
amx_Exec
amx_Push
amx_PushString
amx_PushArray
amx_Allot
amx_Release
amx_GetUserData
amx_SetUserData
amx_RaiseError
```

`amx2.h` adds convenient helpers such as:

```text
amx_GetCStringEx
amx_SetCString
amx_PushAddress
amx_Redirect
```

## Calling a Pawn public

Find its index:

```c
int index;

if (amx_FindPublic(amx, "OnPluginEvent", &index) != AMX_ERR_NONE)
    return;
```

Then execute it:

```c
cell retval = 0;

if (amx_Exec(amx, &retval, index) == AMX_ERR_NONE)
{
    /* retval contains the Pawn return value */
}
```

## Passing scalar parameters

Push parameters in reverse order.

Pawn:

```pawn
forward OnPluginValue(value);
public OnPluginValue(value)
{
    return value * 2;
}
```

C:

```c
int index;
cell retval = 0;

if (amx_FindPublic(amx, "OnPluginValue", &index) == AMX_ERR_NONE)
{
    amx_Push(amx, 123);
    amx_Exec(amx, &retval, index);
}
```

For:

```pawn
public OnPluginValues(a, b)
```

push `b` first:

```c
amx_Push(amx, b);
amx_Push(amx, a);
amx_Exec(amx, &retval, index);
```

## Passing a string

Use `amx_PushString()`:

```c
cell amxAddress = 0;
cell *physicalAddress = NULL;

int error = amx_PushString(
    amx,
    &amxAddress,
    &physicalAddress,
    "hello from plugin",
    0,
    0
);

if (error == AMX_ERR_NONE)
{
    error = amx_Exec(amx, &retval, index);
    amx_Release(amx, amxAddress);
}
```

`amx_PushString()` allocates temporary AMX heap memory.

Release it with `amx_Release()` after the call.

## Passing an array

Use `amx_PushArray()`:

```c
cell values[] = { 10, 20, 30 };
cell amxAddress = 0;
cell *physicalAddress = NULL;

int error = amx_PushArray(
    amx,
    &amxAddress,
    &physicalAddress,
    values,
    3
);

if (error == AMX_ERR_NONE)
{
    error = amx_Exec(amx, &retval, index);
    amx_Release(amx, amxAddress);
}
```

## Allocating AMX memory

`amx_Allot()` reserves cells on the AMX heap:

```c
cell amxAddress = 0;
cell *physicalAddress = NULL;

if (amx_Allot(amx, 3, &amxAddress, &physicalAddress) == AMX_ERR_NONE)
{
    physicalAddress[0] = 10;
    physicalAddress[1] = 20;
    physicalAddress[2] = 30;

    amx_Release(amx, amxAddress);
}
```

## `amx_PushAddress`

When you already have a physical pointer into AMX memory,
`amx_PushAddress()` converts it back to the matching AMX address and pushes it.

```c
if (amx_PushAddress(amx, physicalAddress) == AMX_ERR_NONE)
    amx_Exec(amx, &retval, index);
```

This is an `amx2.h` helper.

## Public variables

Find a public variable:

```c
cell amxAddress;

if (amx_FindPubVar(amx, "SomeValue", &amxAddress) == AMX_ERR_NONE)
{
    cell *physicalAddress = NULL;

    if (amx_GetAddr(amx, amxAddress, &physicalAddress) == AMX_ERR_NONE)
        *physicalAddress = 123;
}
```

Enumerate public variables with:

```c
amx_NumPubVars
amx_GetPubVar
```

## Script metadata

The API can inspect the currently loaded script.

### Flags

```c
uint16_t flags;

amx_Flags(amx, &flags);
```

### Memory information

```c
long codeSize;
long dataSize;
long stackHeap;

amx_MemInfo(
    amx,
    &codeSize,
    &dataSize,
    &stackHeap
);
```

### Maximum name length

```c
int length;

amx_NameLength(amx, &length);
```

### Natives

```c
int count;

amx_NumNatives(amx, &count);

for (int i = 0; i < count; ++i)
{
    char name[128];
    amx_GetNative(amx, i, name);
}
```

Find one by name:

```c
int index;

if (amx_FindNative(amx, "SomeNative", &index) == AMX_ERR_NONE)
{
    /* found */
}
```

### Publics

```c
int count;

amx_NumPublics(amx, &count);

for (int i = 0; i < count; ++i)
{
    char name[128];
    amx_GetPublic(amx, i, name);
}
```

### Public variables

```c
int count;

amx_NumPubVars(amx, &count);
```

Retrieve an entry:

```c
char name[128];
cell address;

amx_GetPubVar(amx, index, name, &address);
```

### Tags

```c
int count;

amx_NumTags(amx, &count);
```

Retrieve a tag:

```c
char name[128];
cell tagId;

amx_GetTag(amx, index, name, &tagId);
```

A tag name can also be found from its id with:

```c
amx_FindTagId(amx, tagId, name);
```

## User data

Each `AMX` has `AMX_USERNUM` user-data slots.

Use:

```c
amx_SetUserData
amx_GetUserData
```

to associate plugin-owned state with a particular script.

See [natives.md](natives.md) for a complete example.

## AMX error reporting

Use:

```c
amx_RaiseError(amx, error);
```

to raise an AMX runtime error.

For example:

```c
amx_RaiseError(amx, AMX_ERR_NATIVE);
```

Many AMX functions already return an `AMX_ERR_*` value. Always propagate or
handle those errors instead of assuming success.

## Raw AMX table helpers

`amx2.h` exposes typed helpers for parsing AMX tables:

```c
amx2_UseNameTable
amx2_NumEntries
amx2_GetEntry
amx2_GetEntryName
```

Example:

```c
AMX_HEADER *header = (AMX_HEADER *)amx->base;

unsigned count = amx2_NumEntries(
    header,
    header->publics,
    header->natives
);

for (unsigned i = 0; i < count; ++i)
{
    AMX_FUNCSTUB *entry =
        amx2_GetEntry(header, header->publics, i);

    const char *name =
        amx2_GetEntryName(header, entry);
}
```

Legacy macro names are also kept for source compatibility:

```text
USENAMETABLE
NUMENTRIES
GETENTRY
GETENTRYNAME
```

New code should prefer the typed `amx2_*` helpers.

## Native redirection

`amx_Redirect()` replaces a linked native entry inside one AMX instance.

```c
AMX_NATIVE original = NULL;

amx_Redirect(
    amx,
    "OriginalNative",
    (ucell)(uintptr_t)ReplacementNative,
    &original
);
```

Restore it with:

```c
amx_Redirect(
    amx,
    "OriginalNative",
    (ucell)(uintptr_t)original,
    NULL
);
```

Use this carefully; it changes the native table of a live script.

## Complete exported AMX API

The SDK forwards these legacy AMX functions:

### Alignment

```text
amx_Align16
amx_Align32
amx_Align64
```

### AMX lifecycle and runtime control

```text
amx_Init
amx_InitJIT
amx_Cleanup
amx_Clone
amx_Callback
amx_SetCallback
amx_SetDebugHook
```

These are low-level runtime functions. Normal server plugins generally operate
on AMX instances already initialized by the server and do not need to create or
reinitialize the VM themselves.

### Execution

```text
amx_Exec
amx_Push
amx_PushArray
amx_PushString
amx_RaiseError
```

### Memory

```text
amx_Allot
amx_GetAddr
amx_Release
amx_MemInfo
```

### Registration and lookup

```text
amx_Register
amx_NativeInfo
amx_FindNative
amx_FindPublic
amx_FindPubVar
amx_FindTagId
```

### Enumeration

```text
amx_NumNatives
amx_NumPublics
amx_NumPubVars
amx_NumTags

amx_GetNative
amx_GetPublic
amx_GetPubVar
amx_GetTag
```

### Strings

```text
amx_GetString
amx_SetString
amx_StrLen

amx_UTF8Check
amx_UTF8Get
amx_UTF8Len
amx_UTF8Put
```

### State

```text
amx_Flags
amx_NameLength
amx_GetUserData
amx_SetUserData
```

The presence of a forwarded function does not imply that it is useful in every
SA-MP plugin. Prefer the high-level operations above unless you specifically
need to manipulate the AMX runtime.

