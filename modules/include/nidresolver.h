#ifndef __NID_RESOLVER_H__
#define __NID_RESOLVER_H__

#include <systemctrl_internal.h>

typedef struct
{
	u32 oldnid;
	u32 newnid;
} Nids;

typedef struct
{
	const char *name;
	Nids *nids;
	int  nnids;
} LibraryData;

LibraryData *FindLib(const char *lib);
u32 FindNewNid(LibraryData *lib, u32 nid);
void sctrlHENRegisterLLEHandler(LLE_HANDLER handler);

#endif
