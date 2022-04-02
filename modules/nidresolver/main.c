#include <pspsdk.h>
#include <string.h>

#include "libraries.h"

#define J_OPCODE	0x08000000
#define JAL_OPCODE	0x0C000000
#define NOP	0x00000000
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a)
#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);  _sw(NOP, a+4);

PSP_MODULE_INFO("NidResolver", 0x3007, 2, 1);

static LLE_HANDLER lle_handler = NULL;

void sctrlHENRegisterLLEHandler(LLE_HANDLER handler)
{
	lle_handler = handler;
}

static int (* aLinkLibEntries)(SceLibraryStubTable *imports, SceSize size, int user, int cs);

void ClearCaches()
{
	sceKernelIcacheInvalidateAll();
	sceKernelDcacheWritebackInvalidateAll();	
}

LibraryData *FindLib(const char *lib)
{
	if (!lib)
		return NULL;

	for (int i = 0; i < sizeof(libraries) / sizeof(libraries[0]); i++)
	{
		if (strcmp(lib, libraries[i].name) == 0)
			return &libraries[i];
	}

	return NULL;
}

u32 FindNewNid(LibraryData *lib, u32 nid)
{
	for (int i = 0; i < lib->nnids; i++)
	{
		if (lib->nids[i].oldnid == nid)
			return lib->nids[i].newnid;
	}

#if 0
	int unk = 1;
	
	for (int i = 0; i < lib->nnids; i++)
	{
		if (lib->nids[i].newnid == nid)
		{
			unk = 0;
			break;
		}
	}
	
	if (unk)
		Kprintf("Unknown nid: %x\n", nid);
#endif

	return 0;
}

int aLinkLibEntriesPatched(SceLibraryStubTable *imports, SceSize size)
{
#if 0
	Kprintf("aLinkLibEntriesPatched++\n");
#endif

	u32 stubTab = (u32)imports;

	for (int i = 0; i < size; i += (((SceLibraryStubTable *)(stubTab + i))->len * 4))
	{
		SceLibraryStubTable *import = (SceLibraryStubTable *)(stubTab + i);

#if 0
		Kprintf("Looking up import nids: %s\n", import->libname);
#endif

		if (lle_handler)
		{
			lle_handler(import);
		}
		
		LibraryData *data = FindLib(import->libname);
		if (data)
		{		
			for (int j = 0; j < import->stubcount; j++)
			{
				u32 nnid = FindNewNid(data, import->nidtable[j]);
				if (nnid)
				{
					import->nidtable[j] = nnid;
				}
			}
		}
		
		if (strcmp("LoadCoreForKernel", import->libname) == 0)
		{
			for (int j = 0; j < import->stubcount; j++)
			{
				if (import->nidtable[j] == 0xd8779ac6)
				{
					SceModule2 *mod = sceKernelFindModuleByName("sceLoaderCore");
					REDIRECT_FUNCTION((u32)&((u32 *)import->stubtable)[j*2], mod->text_addr + 0x748C);
					ClearCaches();
					break;
				}
			}
		}
	}

	ClearCaches();

#if 0
	Kprintf("aLinkLibEntriesPatched--\n");
#endif

	return aLinkLibEntries(imports, size, 0, 0);
}

void PatchLoadCore()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceLoaderCore");
	u32 text_addr = mod->text_addr;

	MAKE_CALL(text_addr + 0x111C, aLinkLibEntriesPatched); // OK
	aLinkLibEntries = (void *)(text_addr + 0x3080); // OK
}

int module_start(SceSize args, void *argp)
{
	PatchLoadCore();

	ClearCaches();

	return 0;
}

int module_stop(void)
{
	return 0;
}

