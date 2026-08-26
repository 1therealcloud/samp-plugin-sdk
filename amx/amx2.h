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

static inline int amx2_UseNameTable(const AMX_HEADER *hdr)
{
	return hdr->defsize == (int16_t)sizeof(AMX_FUNCSTUBNT);
}

static inline unsigned amx2_NumEntries(const AMX_HEADER *hdr, int32_t first, int32_t next)
{
	return (unsigned)(((uint32_t)next - (uint32_t)first) / (uint16_t)hdr->defsize);
}

static inline AMX_FUNCSTUB *amx2_GetEntry(AMX_HEADER *hdr, int32_t table, unsigned index)
{
	return (AMX_FUNCSTUB *)((unsigned char *)hdr
		+ (uint32_t)table
		+ (size_t)index * (uint16_t)hdr->defsize);
}

static inline char *amx2_GetEntryName(AMX_HEADER *hdr, AMX_FUNCSTUB *entry)
{
	if (amx2_UseNameTable(hdr))
	{
		AMX_FUNCSTUBNT *entryNT = (AMX_FUNCSTUBNT *)entry;
		return (char *)((unsigned char *)hdr + entryNT->nameofs);
	}

	return entry->name;
}

/* Legacy helper names kept for source compatibility. */
#define USENAMETABLE(hdr) \
	amx2_UseNameTable((hdr))

#define NUMENTRIES(hdr,field,nextfield) \
	amx2_NumEntries((hdr), (hdr)->field, (hdr)->nextfield)

#define GETENTRY(hdr,table,index) \
	amx2_GetEntry((hdr), (hdr)->table, (unsigned)(index))

#define GETENTRYNAME(hdr,entry) \
	amx2_GetEntryName((hdr), (AMX_FUNCSTUB *)(entry))

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
