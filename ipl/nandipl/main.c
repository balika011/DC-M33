#include <pspsdk.h>
#include <string.h>
#include "syscon.h"
#include "nand.h"

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

uint32_t GetTachyonVersion()
{
	uint32_t ver = _lw(0xbc100040);
	
	if (ver & 0xFF000000)
		return (ver >> 8);

	return 0x100000;
}

int entry(void *a0, void *a1, void *a2, void *a3, void *t0, void *t1, void *t2)
{
	// SYSCON SPI enable
	REG32(0xbc100058) |= 0x02;
	REG32(0xbc10007c) |= 0xc8;
	asm("sync"::);
	
	pspSyscon_init();

#ifdef DEBUG
	pspSysconCrlHpPower(1);
	
	uart_init();
	
	_putchar('n');
	_putchar('a');
	_putchar('n');
	_putchar('d');
	_putchar('i');
	_putchar('p');
	_putchar('l');
	_putchar('l');
	_putchar('d');
	_putchar('r');
	_putchar('\n');
#endif
	uint32_t baryon_version = 0;
	pspSysconGetBaryonVersion(&baryon_version);
	uint32_t tachyon_version = GetTachyonVersion();
	
	if (tachyon_version >= 0x600000)
		_sw(0x20070910, 0xbfc00ffc);
	else if (tachyon_version >= 0x400000)
		_sw(0x20050104, 0xbfc00ffc);
	else
		_sw(0x20040420, 0xbfc00ffc);

	pspNandReadIPLFat();

	for (int i = 0; i < 3; i++)
		if (pspNandReadIPLBlock(1 + i, (void *) (0x40e0000 + i * 0x1000)) < 0)
			break;
	
	for (int i = 0; i < 0x1D; i++)
		if (pspNandReadIPLBlock(4 + i, (void *) (0x40f0000 + i * 0x1000)) < 0)
			break;

	ClearCaches();
	
	return ((int (*)())0x40e0000)();
}

