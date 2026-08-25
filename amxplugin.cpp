//----------------------------------------------------------
//
//   SA-MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2009 SA-MP Team
//
//----------------------------------------------------------
//
// This provides an interface to call AMX library functions
// through the function table supplied by the SA-MP server.
//
// Assign pAMXFunctions from PLUGIN_DATA_AMX_EXPORTS in Load().
//

//----------------------------------------------------------

#include "amx/amx.h"
#include "plugincommon.h"

//----------------------------------------------------------

void *pAMXFunctions;

//----------------------------------------------------------

namespace
{
	template <typename Function>
	Function GetAMXFunction(PLUGIN_AMX_EXPORT index)
	{
		return static_cast<Function *>(pAMXFunctions)[index];
	}
}

#define AMX_FORWARD(return_type, name, parameters, arguments) \
	return_type AMXAPI amx_##name parameters                    \
	{                                                           \
		typedef return_type (AMXAPI *Function) parameters;        \
		return GetAMXFunction<Function>(PLUGIN_AMX_EXPORT_##name) arguments; \
	}

AMX_FORWARD(uint16_t *, Align16, (uint16_t *v), (v))
AMX_FORWARD(uint32_t *, Align32, (uint32_t *v), (v))
#if defined _I64_MAX || defined HAVE_I64
AMX_FORWARD(uint64_t *, Align64, (uint64_t *v), (v))
#endif
AMX_FORWARD(int, Allot, (AMX *amx, int cells, cell *amx_addr, cell **phys_addr), (amx, cells, amx_addr, phys_addr))
AMX_FORWARD(int, Callback, (AMX *amx, cell index, cell *result, cell *params), (amx, index, result, params))
AMX_FORWARD(int, Cleanup, (AMX *amx), (amx))
AMX_FORWARD(int, Clone, (AMX *amxClone, AMX *amxSource, void *data), (amxClone, amxSource, data))
AMX_FORWARD(int, Exec, (AMX *amx, cell *retval, int index), (amx, retval, index))
AMX_FORWARD(int, FindNative, (AMX *amx, const char *name, int *index), (amx, name, index))
AMX_FORWARD(int, FindPublic, (AMX *amx, const char *funcname, int *index), (amx, funcname, index))
AMX_FORWARD(int, FindPubVar, (AMX *amx, const char *varname, cell *amx_addr), (amx, varname, amx_addr))
AMX_FORWARD(int, FindTagId, (AMX *amx, cell tag_id, char *tagname), (amx, tag_id, tagname))
AMX_FORWARD(int, Flags, (AMX *amx, uint16_t *flags), (amx, flags))
AMX_FORWARD(int, GetAddr, (AMX *amx, cell amx_addr, cell **phys_addr), (amx, amx_addr, phys_addr))
AMX_FORWARD(int, GetNative, (AMX *amx, int index, char *funcname), (amx, index, funcname))
AMX_FORWARD(int, GetPublic, (AMX *amx, int index, char *funcname), (amx, index, funcname))
AMX_FORWARD(int, GetPubVar, (AMX *amx, int index, char *varname, cell *amx_addr), (amx, index, varname, amx_addr))
AMX_FORWARD(int, GetString, (char *dest, const cell *source, int use_wchar, size_t size), (dest, source, use_wchar, size))
AMX_FORWARD(int, GetTag, (AMX *amx, int index, char *tagname, cell *tag_id), (amx, index, tagname, tag_id))
AMX_FORWARD(int, GetUserData, (AMX *amx, long tag, void **ptr), (amx, tag, ptr))
AMX_FORWARD(int, Init, (AMX *amx, void *program), (amx, program))
AMX_FORWARD(int, InitJIT, (AMX *amx, void *reloc_table, void *native_code), (amx, reloc_table, native_code))
AMX_FORWARD(int, MemInfo, (AMX *amx, long *codesize, long *datasize, long *stackheap), (amx, codesize, datasize, stackheap))
AMX_FORWARD(int, NameLength, (AMX *amx, int *length), (amx, length))
AMX_FORWARD(AMX_NATIVE_INFO *, NativeInfo, (const char *name, AMX_NATIVE func), (name, func))
AMX_FORWARD(int, NumNatives, (AMX *amx, int *number), (amx, number))
AMX_FORWARD(int, NumPublics, (AMX *amx, int *number), (amx, number))
AMX_FORWARD(int, NumPubVars, (AMX *amx, int *number), (amx, number))
AMX_FORWARD(int, NumTags, (AMX *amx, int *number), (amx, number))
AMX_FORWARD(int, Push, (AMX *amx, cell value), (amx, value))
AMX_FORWARD(int, PushArray, (AMX *amx, cell *amx_addr, cell **phys_addr, const cell array[], int numcells), (amx, amx_addr, phys_addr, array, numcells))
AMX_FORWARD(int, PushString, (AMX *amx, cell *amx_addr, cell **phys_addr, const char *string, int pack, int use_wchar), (amx, amx_addr, phys_addr, string, pack, use_wchar))
AMX_FORWARD(int, RaiseError, (AMX *amx, int error), (amx, error))
AMX_FORWARD(int, Register, (AMX *amx, const AMX_NATIVE_INFO *nativelist, int number), (amx, nativelist, number))
AMX_FORWARD(int, Release, (AMX *amx, cell amx_addr), (amx, amx_addr))
AMX_FORWARD(int, SetCallback, (AMX *amx, AMX_CALLBACK callback), (amx, callback))
AMX_FORWARD(int, SetDebugHook, (AMX *amx, AMX_DEBUG debug), (amx, debug))
AMX_FORWARD(int, SetString, (cell *dest, const char *source, int pack, int use_wchar, size_t size), (dest, source, pack, use_wchar, size))
AMX_FORWARD(int, SetUserData, (AMX *amx, long tag, void *ptr), (amx, tag, ptr))
AMX_FORWARD(int, StrLen, (const cell *cstring, int *length), (cstring, length))
AMX_FORWARD(int, UTF8Check, (const char *string, int *length), (string, length))
AMX_FORWARD(int, UTF8Get, (const char *string, const char **endptr, cell *value), (string, endptr, value))
AMX_FORWARD(int, UTF8Len, (const cell *cstr, int *length), (cstr, length))
AMX_FORWARD(int, UTF8Put, (char *string, char **endptr, int maxchars, cell value), (string, endptr, maxchars, value))

#undef AMX_FORWARD

//----------------------------------------------------------
// EOF
