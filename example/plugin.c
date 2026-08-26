#include <amx/amx.h>
#include <plugincommon.h>

static cell AMX_NATIVE_CALL Example_Add(AMX *amx, cell *params)
{
    (void)amx;
    return params[1] + params[2];
}

static AMX_NATIVE_INFO natives[] =
{
    { "Example_Add", Example_Add },
    { NULL, NULL }
};

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports(void)
{
    return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT int PLUGIN_CALL Load(void **ppData)
{
    if (ppData == NULL || ppData[PLUGIN_DATA_AMX_EXPORTS] == NULL)
        return 0;

    pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
    return 1;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload(void)
{
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
    return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
    (void)amx;
    return AMX_ERR_NONE;
}

