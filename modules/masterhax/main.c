#include <pspkernel.h>
#include <pspctrl.h>
#include <psputility.h>

#include <stdio.h>
#include <string.h>

#include "exploit.h"

PSP_MODULE_INFO("MasterHax", 0, 1, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(3 << 10);

#if 1
u8 hax[0x1000];

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

void KClearCaches(void)
{
	/* Clear the Icache */
	asm("\
	.word 0x40088000; .word 0x24091000; .word 0x7D081240;\
	.word 0x01094804; .word 0x4080E000; .word 0x4080E800;\
	.word 0x00004021; .word 0xBD010000; .word 0xBD030000;\
	.word 0x25080040; .word 0x1509FFFC; .word 0x00000000;\
	"::);
	
	/* Clear the dcache */
	asm("\
	.word 0x40088000; .word 0x24090800; .word 0x7D081180;\
	.word 0x01094804; .word 0x00004021; .word 0xBD140000;\
	.word 0xBD140000; .word 0x25080040; .word 0x1509FFFC;\
	.word 0x00000000; .word 0x0000000F; .word 0x00000000;\
	"::);
}


void UtilsForKernel_39ffb756_hook()
{
	memcpy(0xBFC00000, hax, sizeof(hax));

	KClearCaches();

	_sw(0x32F6, 0xBC10004C);
}

void hook_rebootex()
{
	MAKE_JUMP(0x88000000 + 0x000015A8, (u32) UtilsForKernel_39ffb756_hook | 0x80000000);
	_sw(0, 0x88000000 + 0x000015A8 + 4);

	KClearCaches();
}
#endif

int main(int argc, char *argv[])
{
	pspDebugScreenInit();
	pspDebugScreenSetTextColor(0x0054D035);
	pspDebugScreenPrintf("Reading hax.bin\n");

#if 0
	SceUID fd = sceIoOpen("hax.bin", PSP_O_RDONLY, 0);
	u32 ptr = 0xBFC00000;
	if (fd >= 0)
	{
		u8 buffer[0x100];
		while (sceIoRead(fd, buffer, sizeof(buffer)))
		{
			kmemcpy(ptr, buffer, sizeof(buffer));
			ptr += 0x100;
		}

		pspDebugScreenPrintf("Ra1n!\n");
		
		w32(0x32F6, 0xBC10004C);
	}

	pspDebugScreenPrintf("Failed to open hax.bin\nPress X to exit.");
		
	while (1)
	{
		SceCtrlData pad_data;
		sceCtrlPeekBufferPositive(&pad_data, 1);
		if (pad_data.Buttons & PSP_CTRL_CROSS)
		{
			break;
		}
	}
	
#endif

#if 1
	SceUID fd = sceIoOpen("hax.bin", PSP_O_RDONLY, 0);
	if (fd >= 0)
	{
		sceIoRead(fd, hax, sizeof(hax));
	}
	
	kexec(hook_rebootex);
#endif
	sceKernelExitGame();
	
	return 0;
}
