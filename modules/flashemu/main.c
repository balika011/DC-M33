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

void SystemControlPatch()
{
	const u8 *rbtx;
	switch(sceKernelGetModel())
	{
		case 0: rbtx = &rebootex; break;
		case 1: rbtx = &rebootex_02g; break;
		case 2: rbtx = &rebootex_03g; break;
	}
	
	sctrlHENSetRebootexOverride(rbtx);
}

STMOD_HANDLER previous = NULL;

int OnModuleStart(SceModule2 *module)
{
	if (strcmp(module->modname, "sceLflashFatfmt") == 0)
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

