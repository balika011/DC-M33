#include <pspsdk.h>
#include <string.h>
#include "syscon.h"
#ifdef MSIPL
#include "fat.h"
#endif

#ifdef DEBUG
#include "printf.h"
#include "uart.h"
#endif

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

#ifdef IPL_01G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(void *, void *, void *, void *, void *, void *, void *)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x88600880)() // OK
#define IcacheClear() ((int (*)(void)) 0x8860012C)() // OK
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x8860574C)(str) // OK
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x8860B6C0)(str) // OK

#define FAT_MOUNT_ADDR 0x886027B0 // OK
#define FAT_OPEN_ADDR 0x886027C4 // OK
#define FAT_READ_ADDR 0x88602834 // OK
#define FAT_CLOSE_ADDR 0x88602860 // OK

#define MODULE_PATCH 0x886070F8 // OK

#define RECOVERY_ERROR1 0x88602810 // OK
#define RECOVERY_ERROR2 0x88602828 // OK
	
#define LOAD_CORE 0x88605640 // OK

#define PSP_CONFIG_HASH 0x88607390 // OK

#define NAND_ENCRYPTION 0x88601620 // OK

#define KIRK11_SIGANTURE_CHECK 0x88601040 // OK

#elif IPL_02G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(void *, void *, void *, void *, void *, void *, void *)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886007C0)()
#define IcacheClear() ((int (*)(void)) 0x8860022C)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x88604FFC)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88607BE0)(str)

#define FAT_MOUNT_ADDR 0x8860200C
#define FAT_OPEN_ADDR 0x88602020
#define FAT_READ_ADDR 0x88602090
#define FAT_CLOSE_ADDR 0x886020BC

#define MODULE_PATCH 0x88606A90

#define RECOVERY_ERROR1 0x8860206C
#define RECOVERY_ERROR2 0x88602084
	
#define LOAD_CORE 0x88604EF0

#define PSP_CONFIG_HASH 0x88606D38

#define NAND_ENCRYPTION 0x886013CC

#define KIRK11_SIGANTURE_CHECK 0

#elif IPL_03G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(void *, void *, void *, void *, void *, void *, void *)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886007C0)()
#define IcacheClear() ((int (*)(void)) 0x8860022C)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x88604FFC)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88607BE0)(str)

#define FAT_MOUNT_ADDR 0x8860200C
#define FAT_OPEN_ADDR 0x88602020
#define FAT_READ_ADDR 0x88602090
#define FAT_CLOSE_ADDR 0x886020BC

#define MODULE_PATCH 0x88606AA0

#define RECOVERY_ERROR1 0x8860206C
#define RECOVERY_ERROR2 0x88602084
	
#define LOAD_CORE 0x88604EF0

#define PSP_CONFIG_HASH 0x88606D38

#define NAND_ENCRYPTION 0x886013CC

#define KIRK11_SIGANTURE_CHECK 0

#else

#error Define IPL

#endif

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

#ifdef MSIPL
char g_filename[128];
#endif
char g_file[64];
int sceBootLfatOpenPatched(char *file)
{
#ifdef DEBUG
	printf("sceBootLfatOpenPatched(%s)\n", file);
#endif
	
	// Copy to other buffer to avoid changing the string
	memcpy(g_file, file, 64);
	
	if (memcmp(g_file+4, "pspbtcnf", 8) == 0)
	{
		switch(_lw(0x88FB00C0))
		{
			case 1: memcpy(&g_file[strlen(g_file) - 4], "_m33.bin", 9); break;
			case 2: memcpy(&g_file[strlen(g_file) - 4], "_np.bin", 8); break;
			default: memcpy(&g_file[strlen(g_file) - 4], "_umd.bin", 9);
		}
	}

#ifdef MSIPL
	strcpy(g_filename, "/TM/DC10");
	strcat(g_filename, g_file);
	int ret = MsFatOpen(g_filename);
	if (ret < 0)
	{
		if (_lw(0x88FB00CC) == 2)
			strcpy(g_filename, "/TM/DC10/testingtool");
		else
			strcpy(g_filename, "/TM/DC10/retail");
		strcat(g_filename, g_file);
		ret = MsFatOpen(g_filename);
	}
	return ret;
#else
	return sceBootLfatOpen(g_file);
#endif
}

#ifdef MSIPL
char tmctrl[] = "/tmctrl.prx";

int sceBootModuleLoadPatched(char *file)
{
	int len = sceBootModuleLoad(file); // strlen?
	if (len >= 0)
	{
		if (len > 14)
		{
			for (int i = 0; i < len - 14; i++)
			{
				if (strcasecmp(&file[i], "/kd/lfatfs.prx") == 0)
				{
					memcpy(&file[i], tmctrl, sizeof(tmctrl));
					break;
				}
			}
		}
	}
	
	return len;
}
#endif

int PatchMemlmd(u32 memlmdStub)
{
#ifdef DEBUG
	printf("PatchMemlmd\n");
#endif

	u32 stubs = *(u32 *)(memlmdStub + 0x18);
	u32 jmp = *(u32 *) stubs;
	u32 memlmd_2ae425d2 = (jmp & ~J_OPCODE) << 2 | 0x80000000;

	u32 text_addr = memlmd_2ae425d2 - 0x13F0;
	
	// disable ecdsa signature check
	_sw(0, text_addr + 0xDBC);
	_sw(0, text_addr + 0xDBD);

	ClearCaches();

	return 0;
}

int PatchLoadCore(void *a0, void *a1, void *a2, int (* module_start)(void *, void *, void *))
{
#ifdef DEBUG
	printf("PatchLoadCore\n");
#endif

	u32 text_addr = ((u32)module_start) - 0xAF8;

#ifdef MSIPL
	// disable unsign check
	_sw(0x1021, text_addr + 0x5994);
	_sw(0x1021, text_addr + 0x59C4);
	_sw(0x1021, text_addr + 0x5A5C);
#endif

	_sw(0x02602021, text_addr + 0x2BD0);
	MAKE_CALL(text_addr + 0x2CA4, PatchMemlmd);

	ClearCaches();
	
	return module_start(a0, a1, a2);
}

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
#ifdef MSIPL
	// GPIO enable ?
	REG32(0xbc100058) |= 0x02;

	// GPIO enable
	REG32(0xbc10007c) |= 0xc8;
	asm("sync"::);
	
	sceSysconInit();
	sceSysconCtrlMsPower(1);
#endif
	
#ifdef DEBUG
	sceSysconCtrlHRPower(1);
	
	uart_init();

	printf("rebootex starting...\n");
#endif

	// Patch sceBootLfatOpen, scebootLfatRead and sceBooLfatClose calls
#ifdef MSIPL
	MAKE_CALL(FAT_MOUNT_ADDR, MsFatMount);
#endif
	MAKE_CALL(FAT_OPEN_ADDR, sceBootLfatOpenPatched);
#ifdef MSIPL
	MAKE_CALL(FAT_READ_ADDR, MsFatRead);
	MAKE_CALL(FAT_CLOSE_ADDR, MsFatClose);
#endif

	// Two patches during file read to avoid possible fake recovery file error
	_sw(0, RECOVERY_ERROR1);
	_sw(0, RECOVERY_ERROR2);
	
#ifdef MSIPL
	MAKE_CALL(MODULE_PATCH, sceBootModuleLoadPatched);
#endif

	// Patch the call to LoadCore module_start 
	// 88602910: jr $t7 -> mov $a3, $t7 // a3 = LoadCore module_start 
	// 88602914: mov $sp, $s4 -> j PatchLoadCore
	// 88602918: nop -> mov $sp, $s4
	_sw(0x00113821, LOAD_CORE);
	MAKE_JUMP(LOAD_CORE + 4, PatchLoadCore);
	_sw(0x02a0e821, LOAD_CORE + 8);
	
	// Remove check of return code from the check of the hash in pspbtcnf.bin
	_sw(0, PSP_CONFIG_HASH);

	// Patch nand encryption
#ifdef MSIPL
	_sw(0x03e00008, NAND_ENCRYPTION);
	_sw(0x1021, NAND_ENCRYPTION + 4);
#endif

	// Patch ecdsa signature check
	_sw(0, KIRK11_SIGANTURE_CHECK);
	_sw(0, KIRK11_SIGANTURE_CHECK + 4);

	/*
	rtm_module_after = _lw(0x88FB00D0);
	rtm_buf = _lw(0x88FB00D4);
	rtm_size = _lw(0x88FB00D8);
	rtm_flags = _lw(0x88FB00DC);
	inject_rtm = 0;
	in_rtm = 0;
	*/

#ifdef DEBUG
	_sw(0, 0x886029E4);
	_sw(0, 0x886035FC);
	_sw(0, 0x886036D4);
	_sw(0, 0x886029B4);

	((u32 *)a0)[16] = 0;
	((u32 *)a0)[17] = 0;
	
	// enable bootloader Kprintf
	((u32 *)a0)[17] |= 0x1000000;
	// enable uart4 Krpintf (add uart4.prx after init.prx in pspbtcnf)
	((u32 *)a0)[17] |= 0x2000000;
	// iofilemgr debug stuff
	((u32 *)a0)[17] |= 0x8000000;

	printf("Kick it!\n");	
#endif

	ClearCaches();

	return Ipl_Payload(a0, a1, a2, a3, t0, t1, t2);	
}

