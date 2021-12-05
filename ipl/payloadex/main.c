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

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x88601318)()
#define IcacheClear() ((int (*)(void)) 0x88600D84)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x8860A2D4)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88603F18)(str)

#define FAT_MOUNT_ADDR 0x88603394
#define FAT_OPEN_ADDR 0x886033A4
#define FAT_READ_ADDR 0x8860340C
#define FAT_CLOSE_ADDR 0x8860342C

#define RECOVERY_ERROR1 0x886033EC
#define RECOVERY_ERROR2 0x886033FC

#define MODULE_PATCH 0x88602AF8

#define LOAD_CORE 0x88602910

#define PSP_CONFIG_HASH 0x88602E68

#define NAND_ENCRYPTION 0x886009C4

#elif IPL_02G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886013E0)()
#define IcacheClear() ((int (*)(void)) 0x88600E4C)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x8860A3A8)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88603FEC)(str)

#define FAT_MOUNT_ADDR 0x88603468
#define FAT_OPEN_ADDR 0x88603478
#define FAT_READ_ADDR 0x886034E0
#define FAT_CLOSE_ADDR 0x88603500

#define RECOVERY_ERROR1 0x886034C0
#define RECOVERY_ERROR2 0x886034D0

#define MODULE_PATCH 0x88602BB4

#define LOAD_CORE 0x886029D8

#define PSP_CONFIG_HASH 0x88602F3C

#define NAND_ENCRYPTION 0x88600A54

#elif IPL_03G

#define Ipl_Payload(a0, a1, a2, a3, t0, t1, t2) ((int (*)(u32, u32, u32, u32, u32, u32, u32)) 0x88600000)(a0, a1, a2, a3, t0, t1, t2)
#define DcacheClear() ((int (*)(void)) 0x886013E0)()
#define IcacheClear() ((int (*)(void)) 0x88600E4C)()
#define sceBootModuleLoad(str) ((int (*)(char *)) 0x8860A3D4)(str)
#define sceBootLfatOpen(str) ((int (*)(char *)) 0x88604018)(str)

#define FAT_MOUNT_ADDR 0x88603494
#define FAT_OPEN_ADDR 0x886034A4
#define FAT_READ_ADDR 0x8860350C
#define FAT_CLOSE_ADDR 0x8860352C

#define RECOVERY_ERROR1 0x886034EC
#define RECOVERY_ERROR2 0x886034FC

#define MODULE_PATCH 0x88602BCC

#define LOAD_CORE 0x886029D8

#define PSP_CONFIG_HASH 0x88602F68

#define NAND_ENCRYPTION 0x88600A54

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
	strcpy(g_filename, "/TM/DC9");
	strcat(g_filename, g_file);
	int ret = MsFatOpen(g_filename);
	if (ret < 0)
	{
		strcpy(g_filename, "/TM/DC9/retail");
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

int PatchLoadCore(void *a0, void *a1, void *a2, int (* module_start)(void *, void *, void *))
{
#ifdef DEBUG
	printf("PatchLoadCore\n");
#endif

	u32 text_addr = ((u32)module_start) - 0xC74;

#ifdef MSIPL
	// disable unsign check
	_sw(0x1021, text_addr + 0x691C);
	_sw(0x1021, text_addr + 0x694C);
	_sw(0x1021, text_addr + 0x69E4);
#endif

	ClearCaches();
	
	return module_start(a0, a1, a2);
}

int entry(u32 a0, u32 a1, u32 dispsw_low32, u32 a3, u32 dispsw_high32, u32 t1, u32 t2)
{
#ifdef DEBUG
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

		// Clear Systemctrl vars
		memset((void *)0x88fb0000, 0, 0x100);

		ClearCaches();
#ifndef MSIPL
	}
#endif

#ifdef DEBUG
		dispsw_high32 |= 0x1000000;
#endif

	return Ipl_Payload(a0, a1, dispsw_low32, a3, dispsw_high32, t1, t2);	
}

