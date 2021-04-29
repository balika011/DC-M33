#include <pspsdk.h>
#include <string.h>
#include "syscon.h"
#include "fat.h"

#ifdef DEBUG
#include "printf.h"
#include "uart.h"
#endif

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

#ifdef IPL_01G

int (* Ipl_Payload)(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2) = (void *)0x88600000;
int (* DcacheClear)(void) = (void *)0x886007C0;
int (* IcacheClear)(void) = (void *)0x8860022C;
int (* sceBootModuleLoad)(char *) = (void *)0x88604F34;

#define FAT_MOUNT_ADDR 0x88601F44
#define FAT_OPEN_ADDR 0x88601F58
#define FAT_READ_ADDR 0x88601FC8
#define FAT_CLOSE_ADDR 0x88601FF4

#define CHECK_PSP_CONFIG 0x88604F68

#define MODULE_PATCH 0x886069D0

#define REMOVE_BY_DEBUG 0x88603018
#define KPRINTF_ADDR 0x88612738

#define INIT_ERROR 0x88601F50
#define RECOVERY_ERROR1 0x88601FA4
#define RECOVERY_ERROR2 0x88601FBC
	
#define LOAD_CORE 0x88604E28

#define PSP_CONFIG_HASH 0x88606C68

#define NAND_ENCRYPTION 0x8860133C

#elif IPL_02G

int (* Ipl_Payload)(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2) = (void *)0x88600000;
int (* DcacheClear)(void) = (void *)0x886007C0;
int (* IcacheClear)(void) = (void *)0x8860022C;
int (* sceBootModuleLoad)(char *) = (void *)0x88604FFC;

#define FAT_MOUNT_ADDR 0x8860200C
#define FAT_OPEN_ADDR 0x88602020
#define FAT_READ_ADDR 0x88602090
#define FAT_CLOSE_ADDR 0x886020BC

#define CHECK_PSP_CONFIG 0x88605030

#define MODULE_PATCH 0x88606A90

#define REMOVE_BY_DEBUG 0x886030E0
#define KPRINTF_ADDR 0x88612828

#define INIT_ERROR 0x88602018
#define RECOVERY_ERROR1 0x8860206C
#define RECOVERY_ERROR2 0x88602084
	
#define LOAD_CORE 0x88604EF0

#define PSP_CONFIG_HASH 0x88606D38

#define NAND_ENCRYPTION 0x886013CC

#elif IPL_03G

int (* Ipl_Payload)(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2) = (void *)0x88600000;
int (* DcacheClear)(void) = (void *)0x886007C0;
int (* IcacheClear)(void) = (void *)0x8860022C;
int (* sceBootModuleLoad)(char *) = (void *)0x88604FFC;

#define FAT_MOUNT_ADDR 0x8860200C
#define FAT_OPEN_ADDR 0x88602020
#define FAT_READ_ADDR 0x88602090
#define FAT_CLOSE_ADDR 0x886020BC

#define CHECK_PSP_CONFIG 0x88605030

#define MODULE_PATCH 0x88606AA0

#define REMOVE_BY_DEBUG 0x886030E0
#define KPRINTF_ADDR 0x88612828

#define INIT_ERROR 0x88602018
#define RECOVERY_ERROR1 0x8860206C
#define RECOVERY_ERROR2 0x88602084
	
#define LOAD_CORE 0x88604EF0

#define PSP_CONFIG_HASH 0x88606D38

#define NAND_ENCRYPTION 0x886013CC

#else

#error Define IPL

#endif

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

char g_filename[128];
int BuildPath(char *path)
{
	strcpy(g_filename, "/TM/DC8");
	strcat(g_filename, path);
	return MsFatOpen(g_filename);
}

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
			case 1: memcpy(&g_file[strlen(g_file) - 4], "_oe.bin", 8); break; // MODE_OE_LEGACY
			case 2: memcpy(&g_file[strlen(g_file) - 4], "_m33.bin", 9); break; // MODE_MARCH33
			default: memcpy(&g_file[strlen(g_file) - 4], "_umd.bin", 9);
		}
	}

	return BuildPath(g_file);	
}

char tmctrl[] = "/tmctrl500.prx";

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

int PatchLoadCore(void *a0, void *a1, void *a2, int (* module_start)(void *, void *, void *))
{
#ifdef DEBUG
	printf("PatchLoadCore\n");
#endif

	u32 text_addr = ((u32)module_start) - 0xC74;

	// disable unsign check
	_sw(0x1021, text_addr + 0x691C);
	_sw(0x1021, text_addr + 0x694C);
	_sw(0x1021, text_addr + 0x69E4);

	ClearCaches();
	
	return module_start(a0, a1, a2);
}

#ifdef DEBUG
int kprintf(uint32_t unk, char *fmt, va_list va)
{
	vprintf(fmt, va);

	return 0;
}
#endif

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
	// GPIO enable ?
	REG32(0xbc100058) |= 0x02;

	// GPIO enable
	REG32(0xbc10007c) |= 0xc8;
	asm("sync"::);
	
	pspSyscon_init();
	pspSysconCrlMsPower(1);
	
#ifdef DEBUG
	pspSysconCrlHpPower(1);
	
	uart_init();

	printf("rebootex starting...\n");
#endif

	// Patch sceBootLfatOpen, scebootLfatRead and sceBooLfatClose calls
	MAKE_CALL(FAT_MOUNT_ADDR, MsFatMount);
	MAKE_CALL(FAT_OPEN_ADDR, sceBootLfatOpenPatched);
	MAKE_CALL(FAT_READ_ADDR, MsFatRead);
	MAKE_CALL(FAT_CLOSE_ADDR, MsFatClose);

	// Patch sceKernelCheckPspConfig to enable plain config 
	// sw $a0, 0($sp) -> sw $a1, 0($sp) 
	// addiu v1, zero, $ffff -> addi	$v1, $a1, 0x0000
	// return -1 -> return a1 (size)
	_sw(0xafa50000, CHECK_PSP_CONFIG);	
	_sw(0x20a30000, CHECK_PSP_CONFIG + 4);

	// patch init error
	_sw(0, INIT_ERROR);
	// Two patches during file read to avoid possible fake recovery file error
	_sw(0, RECOVERY_ERROR1);
	_sw(0, RECOVERY_ERROR2);
	
	MAKE_CALL(MODULE_PATCH, sceBootModuleLoadPatched);
	
#ifdef DEBUG
	// patch point : removeByDebugSecion 
	// Dummy a function to make it return 1 
	_sw(0x03e00008, REMOVE_BY_DEBUG);
	_sw(0x24020001, REMOVE_BY_DEBUG + 4);

	_sw((uint32_t) kprintf, KPRINTF_ADDR);
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
	_sw(0x03e00008, NAND_ENCRYPTION);
	_sw(0x1021, NAND_ENCRYPTION + 4);

	/*
	rtm_module_after = _lw(0x88FB00D0);
	rtm_buf = _lw(0x88FB00D4);
	rtm_size = _lw(0x88FB00D8);
	rtm_flags = _lw(0x88FB00DC);
	inject_rtm = 0;
	in_rtm = 0;
	*/

	ClearCaches();

	return Ipl_Payload(a0, a1, a2, a3, t0, t1, t2);	
}

