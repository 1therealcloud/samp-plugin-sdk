/*
 * samp-plugin-sdk example
 *
 * This file is intentionally more complete than a minimal plugin. It shows the
 * main pieces of the legacy SA-MP plugin API exposed by this SDK:
 *
 *   - plugin entry points:
 *       Supports, Load, Unload, AmxLoad, AmxUnload, ProcessTick
 *
 *   - server data passed to Load():
 *       logprintf
 *       AMX function table
 *       CallPublic for filterscripts and the gamemode
 *       legacy SA-MP internal slots (presence check only)
 *
 *   - Pawn native registration and parameter handling
 *
 *   - AMX strings:
 *       amx_GetCStringEx
 *       amx_SetCString
 *
 *   - calling Pawn publics:
 *       amx_FindPublic
 *       amx_Push
 *       amx_PushString
 *       amx_PushArray
 *       amx_Exec
 *       amx_Release
 *
 *   - AMX memory:
 *       amx_GetAddr
 *       amx_Allot
 *       amx_PushAddress
 *
 *   - AMX metadata/introspection:
 *       amx_Flags
 *       amx_MemInfo
 *       amx_NameLength
 *       amx_NumNatives / amx_GetNative
 *       amx_NumPublics / amx_GetPublic
 *       amx_NumPubVars / amx_GetPubVar
 *       amx_NumTags / amx_GetTag
 *       amx_FindNative
 *       amx_FindPubVar
 *
 *   - per-AMX userdata:
 *       amx_SetUserData
 *       amx_GetUserData
 *
 *   - redirecting a registered native:
 *       amx_Redirect
 *
 *   - raw AMX table helpers from amx2.h:
 *       amx2_NumEntries
 *       amx2_GetEntry
 *       amx2_GetEntryName
 *
 * The SDK also forwards the lower-level AMX runtime API (amx_Callback,
 * amx_Cleanup, amx_Clone, amx_Init, amx_InitJIT, amx_SetCallback,
 * amx_SetDebugHook, the Align helpers and the UTF8 helpers). Those functions
 * control or implement the AMX runtime itself, so a normal SA-MP plugin should
 * not call them just to demonstrate that they exist.
 *
 * amx2.h also provides C++ convenience helpers (amx_GetCppString and
 * amx_SetCppString). This example is written in C, so those are not used here.
 */

#include <stdint.h>
#include <stdlib.h>

#include <amx/amx.h>
#include <amx/amx2.h>
#include <plugincommon.h>

/*
 * The server supplies these callbacks through ppData in Load().
 *
 * CallPublic callbacks take only a public name. They are useful for notifying
 * scripts from plugin code when no parameters are required.
 */
typedef void (*logprintf_t)(const char *format, ...);
typedef int (*callpublic_t)(char *name);

static logprintf_t logprintf;
static callpublic_t callPublicFilterScripts;
static callpublic_t callPublicGameMode;

static unsigned int tickCount;

/*
 * Each AMX instance (gamemode/filterscript) can store up to AMX_USERNUM tagged
 * pointers. We use one slot to attach a tiny piece of per-script state.
 *
 * The numeric tag is arbitrary; it only has to be stable and unique inside the
 * plugin.
 */
#define EXAMPLE_USERDATA_TAG 0x45584d50L /* "EXMP" */

typedef struct ExampleAmxState
{
    unsigned int nativeCalls;
    AMX_NATIVE originalNative;
    int redirected;
} ExampleAmxState;

static int Example_HasParams(const cell *params, int count)
{
    if (params == NULL || count < 0)
        return 0;

    return params[0] >= (cell)(count * (int)sizeof(cell));
}

static ExampleAmxState *Example_GetState(AMX *amx)
{
    void *ptr = NULL;

    if (amx == NULL)
        return NULL;

    if (amx_GetUserData(amx, EXAMPLE_USERDATA_TAG, &ptr) != AMX_ERR_NONE)
        return NULL;

    return (ExampleAmxState *)ptr;
}

static void Example_CountNativeCall(AMX *amx)
{
    ExampleAmxState *state = Example_GetState(amx);

    if (state != NULL)
        ++state->nativeCalls;
}

/*
 * Basic native: demonstrates ordinary Pawn parameters and return values.
 *
 * Pawn:
 *     native Example_Add(a, b);
 */
static cell AMX_NATIVE_CALL Example_Add(AMX *amx, cell *params)
{
    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 2))
        return 0;

    return params[1] + params[2];
}

/*
 * Read a Pawn string with the C helper from amx2.h.
 *
 * amx_GetCStringEx allocates a normal C string. The caller owns the returned
 * buffer and must free() it.
 *
 * Pawn:
 *     native Example_Print(const text[]);
 */
static cell AMX_NATIVE_CALL Example_Print(AMX *amx, cell *params)
{
    char *text = NULL;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1))
        return 0;

    if (amx_GetCStringEx(amx, params[1], &text) != AMX_ERR_NONE)
        return 0;

    logprintf("[samp-plugin-example] Pawn says: %s", text);

    free(text);
    return 1;
}

/*
 * Write to a Pawn output buffer.
 *
 * Pawn:
 *     native Example_WriteText(output[], size);
 */
static cell AMX_NATIVE_CALL Example_WriteText(AMX *amx, cell *params)
{
    static const char text[] = "written by samp-plugin-sdk";

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 2) || params[2] <= 0)
        return 0;

    return amx_SetCString(amx, params[1], text, (int)params[2]) == AMX_ERR_NONE;
}

/*
 * Find and execute a public in the AMX instance that called this native.
 *
 * This also demonstrates the AMX stack. Parameters are pushed in reverse
 * order, exactly like arguments on a normal call:
 *
 *     public Example_OnPluginCall(value, const text[])
 *
 * We push the string first, then the integer.
 *
 * amx_PushString allocates temporary AMX heap memory, so amx_Release is called
 * afterwards.
 *
 * Pawn:
 *     native Example_CallPublic(const name[], value, const text[]);
 */
static cell AMX_NATIVE_CALL Example_CallPublic(AMX *amx, cell *params)
{
    char *name = NULL;
    char *text = NULL;
    cell stringAddress = 0;
    cell *physicalString = NULL;
    cell retval = 0;
    int publicIndex;
    int error;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 3))
        return 0;

    error = amx_GetCStringEx(amx, params[1], &name);
    if (error != AMX_ERR_NONE)
        return 0;

    error = amx_GetCStringEx(amx, params[3], &text);
    if (error != AMX_ERR_NONE)
    {
        free(name);
        return 0;
    }

    error = amx_FindPublic(amx, name, &publicIndex);
    if (error != AMX_ERR_NONE)
        goto cleanup;

    /*
     * Example_OnPluginCall(value, text[])
     *
     * Push the last parameter first.
     */
    error = amx_PushString(
        amx,
        &stringAddress,
        &physicalString,
        text,
        0,
        0
    );
    if (error != AMX_ERR_NONE)
        goto cleanup;

    error = amx_Push(amx, params[2]);
    if (error == AMX_ERR_NONE)
        error = amx_Exec(amx, &retval, publicIndex);

    amx_Release(amx, stringAddress);

cleanup:
    free(text);
    free(name);

    return error == AMX_ERR_NONE ? retval : 0;
}

/*
 * Copy a Pawn array to temporary AMX heap memory and pass the copy to a public.
 *
 * Pawn public expected by this example:
 *     public Example_OnPluginArray(count, values[])
 *
 * Pawn native:
 *     native Example_CallPublicArray(
 *         const name[],
 *         const values[],
 *         count
 *     );
 */
static cell AMX_NATIVE_CALL Example_CallPublicArray(AMX *amx, cell *params)
{
    char *name = NULL;
    cell *source = NULL;
    cell arrayAddress = 0;
    cell *physicalArray = NULL;
    cell retval = 0;
    int publicIndex;
    int error;
    int count;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 3) || params[3] < 0)
        return 0;

    count = (int)params[3];

    error = amx_GetCStringEx(amx, params[1], &name);
    if (error != AMX_ERR_NONE)
        return 0;

    error = amx_GetAddr(amx, params[2], &source);
    if (error != AMX_ERR_NONE)
        goto cleanup;

    error = amx_FindPublic(amx, name, &publicIndex);
    if (error != AMX_ERR_NONE)
        goto cleanup;

    /*
     * Example_OnPluginArray(count, values[])
     *
     * Push values[] first because it is the last argument.
     */
    error = amx_PushArray(
        amx,
        &arrayAddress,
        &physicalArray,
        source,
        count
    );
    if (error != AMX_ERR_NONE)
        goto cleanup;

    error = amx_Push(amx, (cell)count);
    if (error == AMX_ERR_NONE)
        error = amx_Exec(amx, &retval, publicIndex);

    amx_Release(amx, arrayAddress);

cleanup:
    free(name);
    return error == AMX_ERR_NONE ? retval : 0;
}

/*
 * Allocate raw cells on the AMX heap and pass their address to a public.
 *
 * This demonstrates amx_Allot + amx_PushAddress. amx_PushAddress converts a
 * physical cell pointer back to an AMX address and pushes it for us.
 *
 * Pawn public expected by this example:
 *     public Example_OnPluginBuffer(values[])
 *
 * Pawn:
 *     native Example_CallPublicBuffer(const name[]);
 */
static cell AMX_NATIVE_CALL Example_CallPublicBuffer(AMX *amx, cell *params)
{
    char *name = NULL;
    cell amxAddress = 0;
    cell *physicalAddress = NULL;
    cell retval = 0;
    int publicIndex;
    int error;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1))
        return 0;

    error = amx_GetCStringEx(amx, params[1], &name);
    if (error != AMX_ERR_NONE)
        return 0;

    error = amx_FindPublic(amx, name, &publicIndex);
    if (error != AMX_ERR_NONE)
        goto cleanup;

    error = amx_Allot(amx, 3, &amxAddress, &physicalAddress);
    if (error != AMX_ERR_NONE)
        goto cleanup;

    physicalAddress[0] = 10;
    physicalAddress[1] = 20;
    physicalAddress[2] = 30;

    error = amx_PushAddress(amx, physicalAddress);
    if (error == AMX_ERR_NONE)
        error = amx_Exec(amx, &retval, publicIndex);

    amx_Release(amx, amxAddress);

cleanup:
    free(name);
    return error == AMX_ERR_NONE ? retval : 0;
}

/*
 * Read a public Pawn variable by name.
 *
 * amx_FindPubVar returns the AMX address. amx_GetAddr converts that address to
 * a physical pointer that the plugin can read/write directly.
 *
 * Pawn:
 *     native Example_ReadPublicVar(const name[]);
 */
static cell AMX_NATIVE_CALL Example_ReadPublicVar(AMX *amx, cell *params)
{
    char *name = NULL;
    cell amxAddress;
    cell *physicalAddress = NULL;
    cell value = 0;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1))
        return 0;

    if (amx_GetCStringEx(amx, params[1], &name) != AMX_ERR_NONE)
        return 0;

    if (amx_FindPubVar(amx, name, &amxAddress) == AMX_ERR_NONE &&
        amx_GetAddr(amx, amxAddress, &physicalAddress) == AMX_ERR_NONE)
    {
        value = *physicalAddress;
    }

    free(name);
    return value;
}

/*
 * Demonstrate AMX metadata functions.
 *
 * This prints information about the current gamemode/filterscript and also
 * exercises the high-level table enumeration functions.
 *
 * Pawn:
 *     native Example_DumpAmxInfo();
 */
static cell AMX_NATIVE_CALL Example_DumpAmxInfo(AMX *amx, cell *params)
{
    uint16_t flags = 0;
    long codeSize = 0;
    long dataSize = 0;
    long stackHeap = 0;
    int nameLength = 0;
    int nativeCount = 0;
    int publicCount = 0;
    int pubVarCount = 0;
    int tagCount = 0;
    char name[128];
    cell address;
    cell tagId;

    Example_CountNativeCall(amx);

    (void)params;

    amx_Flags(amx, &flags);
    amx_MemInfo(amx, &codeSize, &dataSize, &stackHeap);
    amx_NameLength(amx, &nameLength);
    amx_NumNatives(amx, &nativeCount);
    amx_NumPublics(amx, &publicCount);
    amx_NumPubVars(amx, &pubVarCount);
    amx_NumTags(amx, &tagCount);

    logprintf(
        "[samp-plugin-example] AMX flags=0x%04x code=%ld data=%ld "
        "stack+heap=%ld max-name=%d natives=%d publics=%d pubvars=%d tags=%d",
        (unsigned int)flags,
        codeSize,
        dataSize,
        stackHeap,
        nameLength,
        nativeCount,
        publicCount,
        pubVarCount,
        tagCount
    );

    /*
     * GetNative/GetPublic/GetPubVar/GetTag expose table entries by index.
     * Print the first item from each table when one exists.
     */
    if (nativeCount > 0 &&
        amx_GetNative(amx, 0, name) == AMX_ERR_NONE)
    {
        logprintf("[samp-plugin-example] first native: %s", name);
    }

    if (publicCount > 0 &&
        amx_GetPublic(amx, 0, name) == AMX_ERR_NONE)
    {
        logprintf("[samp-plugin-example] first public: %s", name);
    }

    if (pubVarCount > 0 &&
        amx_GetPubVar(amx, 0, name, &address) == AMX_ERR_NONE)
    {
        logprintf(
            "[samp-plugin-example] first pubvar: %s @ %ld",
            name,
            (long)address
        );
    }

    if (tagCount > 0 &&
        amx_GetTag(amx, 0, name, &tagId) == AMX_ERR_NONE)
    {
        logprintf(
            "[samp-plugin-example] first tag: %s id=%ld",
            name,
            (long)tagId
        );
    }

    return 1;
}

/*
 * Find a registered native by name.
 *
 * Pawn:
 *     native Example_HasNative(const name[]);
 */
static cell AMX_NATIVE_CALL Example_HasNative(AMX *amx, cell *params)
{
    char *name = NULL;
    int index;
    int found;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1))
        return 0;

    if (amx_GetCStringEx(amx, params[1], &name) != AMX_ERR_NONE)
        return 0;

    found = amx_FindNative(amx, name, &index) == AMX_ERR_NONE;

    free(name);
    return found;
}

/*
 * Directly inspect the AMX public table with the helpers from amx2.h.
 *
 * These helpers are mainly useful when implementing low-level AMX tooling.
 * For ordinary code, amx_NumPublics + amx_GetPublic are usually simpler.
 *
 * Pawn:
 *     native Example_DumpRawPublicTable();
 */
static cell AMX_NATIVE_CALL Example_DumpRawPublicTable(AMX *amx, cell *params)
{
    AMX_HEADER *header;
    unsigned int count;
    unsigned int index;

    Example_CountNativeCall(amx);

    (void)params;

    if (amx == NULL || amx->base == NULL)
        return 0;

    header = (AMX_HEADER *)amx->base;
    if (header->magic != AMX_MAGIC || header->defsize <= 0)
        return 0;

    count = amx2_NumEntries(
        header,
        header->publics,
        header->natives
    );

    logprintf(
        "[samp-plugin-example] raw public table contains %u entries",
        count
    );

    for (index = 0; index < count; ++index)
    {
        AMX_FUNCSTUB *entry = amx2_GetEntry(
            header,
            header->publics,
            index
        );

        logprintf(
            "[samp-plugin-example] public[%u] = %s",
            index,
            amx2_GetEntryName(header, entry)
        );
    }

    return (cell)count;
}

/*
 * amx_Redirect changes the address stored in an already-linked native entry.
 *
 * The example native below normally returns 100. After
 * Example_EnableRedirect() it returns 200, because calls are redirected to
 * Example_RedirectedNative.
 */
static cell AMX_NATIVE_CALL Example_OriginalNative(AMX *amx, cell *params)
{
    Example_CountNativeCall(amx);
    (void)params;
    return 100;
}

static cell AMX_NATIVE_CALL Example_RedirectedNative(AMX *amx, cell *params)
{
    Example_CountNativeCall(amx);
    (void)params;
    return 200;
}

static cell AMX_NATIVE_CALL Example_EnableRedirect(AMX *amx, cell *params)
{
    ExampleAmxState *state;

    Example_CountNativeCall(amx);
    (void)params;

    state = Example_GetState(amx);
    if (state == NULL)
        return 0;

    if (!state->redirected)
    {
        state->originalNative = NULL;

        amx_Redirect(
            amx,
            "Example_OriginalNative",
            (ucell)(uintptr_t)Example_RedirectedNative,
            &state->originalNative
        );

        if (state->originalNative == NULL)
            return 0;

        state->redirected = 1;
    }

    return 1;
}

static cell AMX_NATIVE_CALL Example_DisableRedirect(AMX *amx, cell *params)
{
    ExampleAmxState *state;

    Example_CountNativeCall(amx);
    (void)params;

    state = Example_GetState(amx);
    if (state == NULL)
        return 0;

    if (state->redirected && state->originalNative != NULL)
    {
        amx_Redirect(
            amx,
            "Example_OriginalNative",
            (ucell)(uintptr_t)state->originalNative,
            NULL
        );

        state->originalNative = NULL;
        state->redirected = 0;
    }

    return 1;
}

/*
 * Call a public in the gamemode or filterscripts through callbacks supplied by
 * the SA-MP/open.mp server itself.
 *
 * These callbacks only pass the public name; there is no parameter stack here.
 * Do not call the same public that invoked the native or you can recurse.
 *
 * Pawn:
 *     native Example_CallGameModePublic(const name[]);
 *     native Example_CallFilterScriptPublic(const name[]);
 */
static cell AMX_NATIVE_CALL Example_CallGameModePublic(AMX *amx, cell *params)
{
    char *name = NULL;
    int result;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1) || callPublicGameMode == NULL)
        return 0;

    if (amx_GetCStringEx(amx, params[1], &name) != AMX_ERR_NONE)
        return 0;

    result = callPublicGameMode(name);

    free(name);
    return (cell)result;
}

static cell AMX_NATIVE_CALL Example_CallFilterScriptPublic(AMX *amx, cell *params)
{
    char *name = NULL;
    int result;

    Example_CountNativeCall(amx);

    if (!Example_HasParams(params, 1) || callPublicFilterScripts == NULL)
        return 0;

    if (amx_GetCStringEx(amx, params[1], &name) != AMX_ERR_NONE)
        return 0;

    result = callPublicFilterScripts(name);

    free(name);
    return (cell)result;
}

/*
 * ProcessTick runs once per server tick because Supports() advertises
 * SUPPORTS_PROCESS_TICK and the CMake target enables the ProcessTick export.
 *
 * Pawn:
 *     native Example_GetTickCount();
 */
static cell AMX_NATIVE_CALL Example_GetTickCount(AMX *amx, cell *params)
{
    Example_CountNativeCall(amx);

    (void)params;
    return (cell)tickCount;
}

/*
 * Demonstrate amx_RaiseError.
 *
 * Calling this native deliberately raises AMX_ERR_NATIVE in the current VM.
 * This is useful when a native cannot continue and wants to report an AMX
 * runtime error instead of only returning a sentinel value.
 *
 * Pawn:
 *     native Example_RaiseNativeError();
 */
static cell AMX_NATIVE_CALL Example_RaiseNativeError(AMX *amx, cell *params)
{
    Example_CountNativeCall(amx);

    (void)params;
    amx_RaiseError(amx, AMX_ERR_NATIVE);
    return 0;
}

static AMX_NATIVE_INFO natives[] =
{
    { "Example_Add",                    Example_Add },
    { "Example_Print",                  Example_Print },
    { "Example_WriteText",              Example_WriteText },
    { "Example_CallPublic",             Example_CallPublic },
    { "Example_CallPublicArray",        Example_CallPublicArray },
    { "Example_CallPublicBuffer",       Example_CallPublicBuffer },
    { "Example_ReadPublicVar",          Example_ReadPublicVar },
    { "Example_DumpAmxInfo",            Example_DumpAmxInfo },
    { "Example_HasNative",              Example_HasNative },
    { "Example_DumpRawPublicTable",     Example_DumpRawPublicTable },
    { "Example_OriginalNative",         Example_OriginalNative },
    { "Example_EnableRedirect",         Example_EnableRedirect },
    { "Example_DisableRedirect",        Example_DisableRedirect },
    { "Example_CallGameModePublic",     Example_CallGameModePublic },
    { "Example_CallFilterScriptPublic", Example_CallFilterScriptPublic },
    { "Example_GetTickCount",           Example_GetTickCount },
    { "Example_RaiseNativeError",       Example_RaiseNativeError },
    { NULL, NULL }
};

/*
 * Supports() tells the server which legacy plugin features are used.
 *
 * SUPPORTS_VERSION is always required.
 * SUPPORTS_AMX_NATIVES enables AmxLoad/AmxUnload.
 * SUPPORTS_PROCESS_TICK enables ProcessTick.
 */
PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION
        | SUPPORTS_AMX_NATIVES
        | SUPPORTS_PROCESS_TICK;
}

/*
 * Load() is called once when the plugin is loaded.
 *
 * ppData is the legacy SA-MP plugin data table. The two entries required by
 * this example are logprintf and the AMX export table.
 *
 * pAMXFunctions is owned by the SDK. It MUST be initialized before any amx_*
 * forwarding function is called.
 */
PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL)
        return 0;

    logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];

    callPublicFilterScripts =
        (callpublic_t)ppData[PLUGIN_DATA_CALLPUBLIC_FS];
    callPublicGameMode =
        (callpublic_t)ppData[PLUGIN_DATA_CALLPUBLIC_GM];

    if (logprintf == NULL || pAMXFunctions == NULL)
        return 0;

    logprintf("[samp-plugin-example] loaded");

    /*
     * These five slots expose private SA-MP server internals.
     *
     * They are intentionally NOT called by this example:
     *   - their C/C++ types are not part of the stable plugin ABI;
     *   - they are tied to SA-MP server internals;
     *   - open.mp intentionally leaves them NULL.
     *
     * The presence check below only demonstrates where the slots live.
     */
    logprintf(
        "[samp-plugin-example] SA-MP internals: "
        "netgame=%s rakserver=%s loadfs=%s console=%s unloadfs=%s",
        ppData[PLUGIN_DATA_NETGAME] != NULL ? "yes" : "no",
        ppData[PLUGIN_DATA_RAKSERVER] != NULL ? "yes" : "no",
        ppData[PLUGIN_DATA_LOADFSCRIPT] != NULL ? "yes" : "no",
        ppData[PLUGIN_DATA_CONSOLE] != NULL ? "yes" : "no",
        ppData[PLUGIN_DATA_UNLOADFSCRIPT] != NULL ? "yes" : "no"
    );

    return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload(void)
{
    if (logprintf != NULL)
        logprintf("[samp-plugin-example] unloaded");
}

/*
 * AmxLoad() is called once for every loaded gamemode/filterscript AMX.
 *
 * Register the natives and attach our per-AMX state.
 */
PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    ExampleAmxState *state;
    int error;

    if (amx == NULL)
        return AMX_ERR_PARAMS;

    state = (ExampleAmxState *)calloc(1, sizeof(*state));
    if (state == NULL)
        return AMX_ERR_MEMORY;

    error = amx_SetUserData(
        amx,
        EXAMPLE_USERDATA_TAG,
        state
    );
    if (error != AMX_ERR_NONE)
    {
        free(state);
        return error;
    }

    error = amx_Register(amx, natives, -1);
    if (error != AMX_ERR_NONE)
    {
        amx_SetUserData(amx, EXAMPLE_USERDATA_TAG, NULL);
        free(state);
        return error;
    }

    logprintf("[samp-plugin-example] AMX loaded");
    return AMX_ERR_NONE;
}

/*
 * AmxUnload() is called before an AMX is destroyed.
 *
 * Restore a redirected native if necessary and free the per-AMX userdata.
 */
PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    ExampleAmxState *state = Example_GetState(amx);

    if (state != NULL)
    {
        if (state->redirected && state->originalNative != NULL)
        {
            amx_Redirect(
                amx,
                "Example_OriginalNative",
                (ucell)(uintptr_t)state->originalNative,
                NULL
            );
        }

        logprintf(
            "[samp-plugin-example] AMX unloaded after %u native calls",
            state->nativeCalls
        );

        amx_SetUserData(amx, EXAMPLE_USERDATA_TAG, NULL);
        free(state);
    }

    return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick(void)
{
    ++tickCount;
}

