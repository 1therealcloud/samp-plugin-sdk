//----------------------------------------------------------
//
//   SA-MP Multiplayer Modification For GTA:SA
//   Copyright 2014 SA-MP Team, Dan, maddinat0r
//
//----------------------------------------------------------

#include <cassert>
#include <cstdlib>
#include <cstring>

//----------------------------------------------------------

#include "amx/amx2.h"

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

#if defined __cplusplus

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

#endif // __cplusplus

//----------------------------------------------------------
// EOF
