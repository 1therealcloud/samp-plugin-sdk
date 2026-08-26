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

#include <cassert>
#include <cstdlib>
#include <cstring>

//----------------------------------------------------------

#include "amx/amx2.h"
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

int AMXAPI amx_PushAddress(AMX *amx, cell *address)
{
	assert(amx != NULL);
	assert(amx->base != NULL);

	AMX_HEADER *hdr = (AMX_HEADER *)amx->base;
	assert(hdr->magic == AMX_MAGIC);

	unsigned char *data = amx->data != NULL
		? amx->data
		: amx->base + (int)hdr->dat;

	cell xaddr = (cell)((unsigned char *)address - data);
	if ((ucell)xaddr >= (ucell)amx->stp)
		return AMX_ERR_MEMACCESS;

	return amx_Push(amx, xaddr);
}

void AMXAPI amx_Redirect(AMX *amx, char *from, ucell to, AMX_NATIVE *store)
{
	AMX_HEADER *hdr = (AMX_HEADER *)amx->base;
	for (int idx = 0, num = NUMENTRIES(hdr, natives, libraries); idx != num; ++idx)
	{
		AMX_FUNCSTUB *func = GETENTRY(hdr, natives, idx);
		if (strcmp(from, GETENTRYNAME(hdr, func)) != 0)
			continue;

		if (store != NULL)
			*store = (AMX_NATIVE)func->address;

		func->address = to;
		return;
	}
}

int AMXAPI amx_GetCStringEx(AMX *amx, cell param, char **dest)
{
	if (dest == NULL)
		return AMX_ERR_PARAMS;

	*dest = NULL;

	cell *ptr = NULL;
	int error = amx_GetAddr(amx, param, &ptr);
	if (error != AMX_ERR_NONE)
		return error;

	int length = 0;
	error = amx_StrLen(ptr, &length);
	if (error != AMX_ERR_NONE)
		return error;

	char *string = (char *)malloc((size_t)length + 1);
	if (string == NULL)
		return AMX_ERR_MEMORY;

	error = amx_GetString(string, ptr, 0, (size_t)length + 1);
	if (error != AMX_ERR_NONE)
	{
		free(string);
		return error;
	}

	*dest = string;
	return AMX_ERR_NONE;
}

int AMXAPI amx_SetCString(AMX *amx, cell param, const char *str, int len)
{
	cell *dest;
	int error = amx_GetAddr(amx, param, &dest);
	if (error != AMX_ERR_NONE)
		return error;

	return amx_SetString(dest, str, 0, 0, len);
}

int AMXAPI amx_GetCString(AMX *amx, cell param, char *&dest)
{
	int error = amx_GetCStringEx(amx, param, &dest);
	if (error != AMX_ERR_NONE)
		return 0;

	int length = 0;
	cell *ptr = NULL;
	if (amx_GetAddr(amx, param, &ptr) != AMX_ERR_NONE ||
		amx_StrLen(ptr, &length) != AMX_ERR_NONE)
	{
		free(dest);
		dest = NULL;
		return 0;
	}

	return length;
}

std::string AMXAPI amx_GetCppString(AMX *amx, cell param)
{
	cell *addr = NULL;
	if (amx_GetAddr(amx, param, &addr) != AMX_ERR_NONE)
		return std::string();

	int length = 0;
	if (amx_StrLen(addr, &length) != AMX_ERR_NONE || length <= 0)
		return std::string();

	std::string string((size_t)length + 1, '\0');
	if (amx_GetString(&string[0], addr, 0, string.size()) != AMX_ERR_NONE)
		return std::string();

	string.resize((size_t)length);
	return string;
}

int AMXAPI amx_SetCppString(AMX *amx, cell param, const std::string &str, size_t maxlen)
{
	cell *dest = NULL;
	int error = amx_GetAddr(amx, param, &dest);
	if (error != AMX_ERR_NONE)
		return error;

	return amx_SetString(dest, str.c_str(), 0, 0, maxlen);
}

//----------------------------------------------------------
// EOF
