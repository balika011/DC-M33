#include <pspsdk.h>
#include <string.h>
#include "syscon.h"
#include "ms.h"

#ifdef DEBUG
#include "printf.h"
#include "uart.h"
#endif

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

void DcacheClear();
void IcacheClear();

void ClearCaches()
{
	DcacheClear();
	IcacheClear();
}

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
	// SYSCON SPI enable
	REG32(0xbc100058) |= 0x02;
	REG32(0xbc10007c) |= 0xc8;
	
	asm("sync"::);
	
	pspSyscon_init();
	pspSysconCrlMsPower(1);
	
#ifdef DEBUG
	pspSysconCrlHpPower(1);
	
	uart_init();
	
	_putchar('m');
	_putchar('s');
	_putchar('i');
	_putchar('p');
	_putchar('l');
	_putchar('l');
	_putchar('d');
	_putchar('r');
	_putchar('\n');
#endif

	pspMsInit();
	
	for (int i = 0; i < 26; i++)
		pspMsReadSector(0x18 + i, (void *)(0x40c0000 + i * 0x200));

	ClearCaches();

	return ((int (*)()) 0x40c0000)();
}

