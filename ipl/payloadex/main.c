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

#if IPL_01G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2) // OK
#define DcacheClear() ((int (*)(void)) 0x88601604)() // OK
#define IcacheClear() ((int (*)(void)) 0x88600EB0)() // OK
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x88609EC0)(str) // OK
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x8860759C)(str) // OK

#define FAT_MOUNT_ADDR 0x88603FC0 // OK
#define FAT_OPEN_ADDR 0x88603FD0 // OK
#define FAT_READ_ADDR 0x88604038 // OK
#define FAT_CLOSE_ADDR 0x88604058 // OK

#define RECOVERY_ERROR1 0x88604018 // OK
#define RECOVERY_ERROR2 0x88604028 // OK

#define MODULE_PATCH 0x886036AC // OK

#define LOAD_CORE 0x886034B4 // OK

#define PSP_CONFIG_HASH 0x88603A6C // OK

#define NAND_ENCRYPTION 0x88600B48 // OK

#define KIRK11_SIGANTURE_CHECK 0x88600568 // OK

#elif IPL_02G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886016CC)()
#define IcacheClear() ((int (*)(void)) 0x88600F78)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x88609F80)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88607664)(str)

#define FAT_MOUNT_ADDR 0x88604088
#define FAT_OPEN_ADDR 0x88604098
#define FAT_READ_ADDR 0x88604100
#define FAT_CLOSE_ADDR 0x88604120

#define RECOVERY_ERROR1 0x886040E0
#define RECOVERY_ERROR2 0x886040F0

#define MODULE_PATCH 0x88603790

#define LOAD_CORE 0x8860357C

#define PSP_CONFIG_HASH 0x88603B34

#define NAND_ENCRYPTION 0x88600BD8

#define KIRK11_SIGANTURE_CHECK 0x8860058C

#elif IPL_03G || IPL_04G || IPL_05G || IPL_07G || IPL_09G || IPL_11G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886016CC)()
#define IcacheClear() ((int (*)(void)) 0x88600F78)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x88609F80)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88607660)(str)

#define FAT_MOUNT_ADDR 0x88604084
#define FAT_OPEN_ADDR 0x88604094
#define FAT_READ_ADDR 0x886040FC
#define FAT_CLOSE_ADDR 0x8860411C

#define RECOVERY_ERROR1 0x886040DC
#define RECOVERY_ERROR2 0x886040EC

#define MODULE_PATCH 0x88603794

#define LOAD_CORE 0x8860357C

#define PSP_CONFIG_HASH 0x88603B30

#define NAND_ENCRYPTION 0x88600BD8

#define KIRK11_SIGANTURE_CHECK 0x8860058C

#else

#error Define IPL

#endif

int recovery = 0;
#ifdef MSIPL
int msboot = 0;
#endif

#ifdef MSIPL
char g_filename[128];
#endif
char g_file[64];
int sceBootLfatOpenPatched(char *file)
{
#ifdef DEBUG
	printf("sceBootLfatOpenPatched(%s)", file);
#endif

	// Copy to other buffer to avoid changing the string
	memcpy(g_file, file, 64);

	if (memcmp(g_file+4, "pspbtcnf", 8) == 0)
	{		
		if (recovery)
			memcpy(&g_file[strlen(g_file) - 4], "_recovery.bin", 17);
#ifdef MSIPL
		else if (!msboot)
			memcpy(&g_file[strlen(g_file) - 4], "_dc.bin", 8);
#endif
		else
			memcpy(&g_file[strlen(g_file) - 4], "_umd.bin", 9);
	}
	
#ifdef DEBUG
	printf("-> %s\n", g_file);
#endif

#ifdef MSIPL
	strcpy(g_filename, "/TM/DC10");
	strcat(g_filename, g_file);
	int ret = MsFatOpen(g_filename);
	if (ret < 0)
	{
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
	int len = sceBootModuleLoad(file);

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

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

int PatchMemlmd(u32 memlmdStub)
{
#ifdef DEBUG
	printf("PatchMemlmd\n");
#endif

	u32 stubs = *(u32 *)(memlmdStub + 0x18);
	u32 jmp = *(u32 *) stubs;
	u32 memlmd_2ae425d2 = (jmp & ~J_OPCODE) << 2 | 0x80000000;

#if IPL_01G
	u32 text_addr = memlmd_2ae425d2 - 0x13F0;

	// disable ecdsa signature check
	_sw(0, text_addr + 0xDBC);
	_sw(0, text_addr + 0xDC0);
#else
	u32 text_addr = memlmd_2ae425d2 - 0x14B0;

	// disable ecdsa signature check
	_sw(0, text_addr + 0xDDC);
	_sw(0, text_addr + 0xDE0);
#endif

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

int entry(u32 a0, u32 a1, u32 dispsw_low32, u32 a3, u32 dispsw_high32, u32 t1, u32 t2)
{
#ifdef DEBUG
	sceSysconCtrlHRPower(1);

	uart_init();

	printf("payloadex starting...\n");
#endif

	recovery = 0;
#ifdef MSIPL
	msboot = 0;
#endif
	u32 ctrl = _lw(0x88fb0000);
#ifndef MSIPL
	if ((ctrl & (SYSCON_CTRL_HOME | SYSCON_CTRL_TRIANGLE | SYSCON_CTRL_ALLOW_UP)) != 0)
	{
#endif
		if((ctrl & SYSCON_CTRL_RTRG) == 0)
		{
			recovery = 1;
		}
#ifdef MSIPL
		else if((ctrl & SYSCON_CTRL_START) == 0)
		{
			msboot = 1;
		}
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
		_sw(0x000f3821, LOAD_CORE);
		MAKE_JUMP(LOAD_CORE + 4, PatchLoadCore);
		_sw(0x0280e821, LOAD_CORE + 8);
		
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
		
#if IPL_04G || IPL_05G || IPL_07G || IPL_09G || IPL_11G
		// TODO: fix masterhax cache issue workaround
		_sw(0x27BDFFF0, 0x88600038); // addiu   $sp, -0x10
		_sw(0xAFBF0000, 0x8860003C); // sw      $ra, 0($sp)
		MAKE_CALL(0x88600040, 0x886016CC);
		_sw(0, 0x88600044);
		MAKE_CALL(0x88600048, 0x88600F78);
		_sw(0, 0x8860004C);
		_sw(0x8FBF0000, 0x88600050); // lw      $ra, 0($sp)
		_sw(0x27BD0010, 0x88600054); // addiu   $sp, 0x10
		MAKE_JUMP(0x88600058, 0x88600070);
		_sw(0, 0x8860005C);
#endif

#ifdef DEBUG
		// puts uart4 -> uart5
#if IPL_03G || IPL_04G || IPL_05G || IPL_07G || IPL_09G || IPL_11G
		_sw(0x3c08be50, 0x88603E60);
		_sw(0x3c01be50, 0x88603E74);
#endif
		
		// IPL DEBUG - [BREAKS BOOTING]
		// dispsw_low32 |= 0x40000000;

		// enable bootn byteswap problémák vloader Kprintf
		dispsw_high32 |= 0x1000000;
		// enable uart4 Krpintf (add uart4.prx after init.prx in pspbtcnf)
		dispsw_high32 |= 0x2000000;
		// iofilemgr debug stuff
		dispsw_high32 |= 0x8000000;
#endif

		// Clear Systemctrl vars
		memset((void *)0x88fb0000, 0, 0x100);

		ClearCaches();
#ifndef MSIPL
	}
#endif
	
	return Ipl_Payload(a0, a1, dispsw_low32, a3, dispsw_high32, t1, t2);	
}

