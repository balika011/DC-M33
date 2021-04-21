#include <pspsdk.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <systemctrl.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>

#include <stdio.h>
#include <string.h>

#include "flashemu.h"

PSP_MODULE_INFO("TimeMachine_Control", 0x1007, 1, 0);
PSP_MAIN_THREAD_ATTR(0);

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000
#define SC_OPCODE	0x0000000C
#define JR_RA		0x03e00008

#define NOP	0x00000000

#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) & 0x3FFFFFFF) >> 2), a); 
#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x3FFFFFFF) >> 2), a); 
#define MAKE_SYSCALL(a, n) _sw(SC_OPCODE | (n << 6), a);
#define JUMP_TARGET(x) ((x & 0x3FFFFFFF) << 2)
#define REDIRECT_FUNCTION(a, f) _sw(J_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a);  _sw(NOP, a+4);
#define MAKE_DUMMY_FUNCTION0(a) _sw(0x03e00008, a); _sw(0x00001021, a+4);
#define MAKE_DUMMY_FUNCTION1(a) _sw(0x03e00008, a); _sw(0x24020001, a+4);

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();
}

extern u8 rebootex;
extern u8 rebootex_02g;
extern u8 rebootex_03g;

int sceKernelGzipDecompressPatched(u8 *dest, u32 destSize, const u8 *src, int unknown)
{
	switch(sceKernelGetModel())
	{
		case 0: src = &rebootex; break;
		case 1: src = &rebootex_02g; break;
		case 2: src = &rebootex_03g; break;
	}
	
	return sceKernelGzipDecompress(dest, destSize, src, 0);
}

void SystemControlPatch()
{
	u32 text_addr = ((SceModule2 *)sceKernelFindModuleByName("SystemControl"))->text_addr;
	
	u32 addr = text_addr + 0x27B8;
	
	if (sceKernelGetModel())
		addr = text_addr + 0x2A68;
	
	MAKE_CALL(addr, sceKernelGzipDecompressPatched);

	ClearCaches();
}

int sceMSFAT_IoOpenPatched(int type, void (*cb)(void *), void *arg)
{
	int res;
	do
	{
		res = sceKernelExtendKernelStack(type, cb, arg);
		if (res != 0x80010018)
			break;
		res = sceKernelExtendKernelStack(0x4000, (void (*)(void*)) SlotCloseOneReadOnly, 0);
	}
	while (res >= 0);

	return res;
}

int sceMSFAT_IoDopenPatched(int type, void (*cb)(void *), void *arg)
{
	int res;
	do
	{
		res = sceKernelExtendKernelStack(type, cb, arg);
		if (res != 0x80010018)
			break;
		res = sceKernelExtendKernelStack(0x4000, (void (*)(void*)) SlotCloseOneReadOnly, 0);
	}
	while (res >= 0);

	return res;
}

int sceMSFAT_IoDevctlPatched(int type, void (*cb)(void *), void *arg)
{
	int res;
	do
	{
		res = sceKernelExtendKernelStack(type, cb, arg);
		if (res != 0x80010018)
			break;
		res = sceKernelExtendKernelStack(0x4000, (void (*)(void*)) SlotCloseOneReadOnly, 0);
	}
	while (res >= 0);

	return res;
}

STMOD_HANDLER previous = NULL;

int OnModuleStart(SceModule2 *module)
{
	if (strcmp(module->modname, "sceMediaSync") == 0)
	{
		u32 text_addr = ((SceModule2 *)sceKernelFindModuleByName("sceMSFAT_Driver"))->text_addr;
		MAKE_CALL(text_addr + 0x48D4, sceMSFAT_IoOpenPatched);
		MAKE_CALL(text_addr + 0x5338, sceMSFAT_IoDopenPatched);
		MAKE_CALL(text_addr + 0x5B90, sceMSFAT_IoDevctlPatched);
		ClearCaches();
	}
	else if (strcmp(module->modname, "sceLflashFatfmt") == 0)
	{
		u32 StartFatfmt = sctrlHENFindFunction("sceLflashFatfmt", "LflashFatfmt", 0xB7A424A4);// sceLflashFatfmtStartFatfmt
		if (StartFatfmt)
		{
			MAKE_DUMMY_FUNCTION0(StartFatfmt);
			ClearCaches();
		}
	}

	if (previous)
		return previous(module);

	return 0;
}

int module_start(SceSize args, void *argp)
{
	SystemControlPatch();
	InstallFlashEmu();
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);
	ClearCaches();
	
	return 0;
}

int module_reboot_before()
{
	PreareRebootFlashEmu();
	return 0;
}

