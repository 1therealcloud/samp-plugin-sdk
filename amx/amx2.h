//----------------------------------------------------------
//
//   SA-MP Multiplayer Modification For GTA:SA
//   Copyright 2014 SA-MP Team, Dan, maddinat0r
//
//----------------------------------------------------------

#pragma once

//----------------------------------------------------------

#if defined __cplusplus
#include <cstdlib>
#include <cstring>
#include <string>
#endif

//----------------------------------------------------------

#include "amx.h"

//----------------------------------------------------------

#define USENAMETABLE(hdr) \
	((hdr)->defsize==sizeof(AMX_FUNCSTUBNT))

#define NUMENTRIES(hdr,field,nextfield) \
	(unsigned)(((hdr)->nextfield - (hdr)->field) / (hdr)->defsize)

#define GETENTRY(hdr,table,index) \
	(AMX_FUNCSTUB *)((unsigned char*)(hdr) + (unsigned)(hdr)->table + (unsigned)index*(hdr)->defsize)

#define GETENTRYNAME(hdr,entry) \
	(USENAMETABLE(hdr) ? \
		(char *)((unsigned char*)(hdr) + (unsigned)((AMX_FUNCSTUBNT*)(entry))->nameofs) : \
		((AMX_FUNCSTUB*)(entry))->name)

//----------------------------------------------------------

#if defined __cplusplus
extern "C" {
#endif

extern int AMXAPI amx_PushAddress(AMX *amx, cell *address);
extern void AMXAPI amx_Redirect(AMX *amx, char *from, ucell to, AMX_NATIVE *store);
extern int AMXAPI amx_GetCStringEx(AMX *amx, cell param, char **dest);
extern int AMXAPI amx_SetCString(AMX *amx, cell param, const char *str, int len);

#if defined __cplusplus
}

inline int AMXAPI amx_GetCString(AMX *amx, cell param, char *&dest)
{
	if (amx_GetCStringEx(amx, param, &dest) != AMX_ERR_NONE)
		return 0;

	return static_cast<int>(std::strlen(dest));
}

inline std::string AMXAPI amx_GetCppString(AMX *amx, cell param)
{
	char *string = NULL;
	if (amx_GetCStringEx(amx, param, &string) != AMX_ERR_NONE)
		return std::string();

	std::string result(string);
	std::free(string);
	return result;
}

inline int AMXAPI amx_SetCppString(AMX *amx, cell param, const std::string &str, size_t maxlen)
{
	cell *dest = NULL;
	int error = amx_GetAddr(amx, param, &dest);
	if (error != AMX_ERR_NONE)
		return error;

	return amx_SetString(dest, str.c_str(), 0, 0, maxlen);
}
#endif

//----------------------------------------------------------
// EOF
