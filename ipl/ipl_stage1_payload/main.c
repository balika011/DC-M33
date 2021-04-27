#include <pspsdk.h>
#include <string.h>
#include "syscon.h"

#ifdef DEBUG
#include "printf.h"
#include "uart.h"
#endif

#define JAL_OPCODE	0x0C000000
#define J_OPCODE	0x08000000

#define MAKE_JUMP(a, f) _sw(J_OPCODE | (((u32)(f) & 0x0ffffffc) >> 2), a); 
#define MAKE_CALL(a, f) _sw(JAL_OPCODE | (((u32)(f) >> 2)  & 0x03ffffff), a); 

#ifdef IPL_01G

#define RESET_VECTOR_ADDRESS 0x040f0118
#define ROM_HMAC_ADDRESS 0x40f0fe4
#define PRE_STAGE2_ADDR 0x40f02a8

#define STAGE2_PATCH_INJECTION_ADDRESS 0x400035C
#define ENTRY_POINT_ADDRESS 0x400C534
#define CLEAR_STRACHPAD_ADDRESS 0x4001100

#elif IPL_02G

#define RESET_VECTOR_ADDRESS 0x040f0118
#define ROM_HMAC_ADDRESS 0x40f0fe4
#define PRE_STAGE2_ADDR 0x40f02a8

#define STAGE2_PATCH_INJECTION_ADDRESS 0x400035C
#define ENTRY_POINT_ADDRESS 0x400D534
#define CLEAR_STRACHPAD_ADDRESS 0x40011E4
#define SET_SEED_ADDRESS 0x4001144
#define SET_SEED_RETURN 0x40011C8

#elif IPL_03G

#define RESET_VECTOR_ADDRESS 0x040f0118
#define ROM_HMAC_ADDRESS 0x40f0fe4
#define PRE_STAGE2_ADDR 0x40f02a8

#define STAGE2_PATCH_INJECTION_ADDRESS 0x400035C
#define ENTRY_POINT_ADDRESS 0x0400D534
#define CLEAR_STRACHPAD_ADDRESS 0x4001218
#define SET_SEED_ADDRESS 0x4001178
#define SET_SEED_RETURN 0x40011FC

#else

#error Define IPL

#endif

extern uint32_t size_payload;
extern uint8_t payload;
void Dcache();
void Icache();
#define MAIN_BIN() ((void (*)()) 0x4000000)()

void *memset(void *dest, int value, size_t size)
{
	uint32_t *d = (uint32_t *) dest;
	
	while (size--)
		*(d++) = value;
	
	return dest;
}

void *memcpy(void *dest, const void *src, size_t size)
{
	uint32_t *d = (uint32_t *) dest;
	uint32_t *s = (uint32_t *) src;
	
	size /= 4;
	
	while (size--)
		*(d++) = *(s++);

	return dest;
}

#ifdef SET_SEED_ADDRESS
uint8_t seed[] = {
#if IPL_02G
	0xAD, 0x43, 0x2A, 0x80, 0x9A, 0xD7, 0x70, 0xA5, 0x87, 0x50, 0x0E, 0x89, 0xE5, 0x5F, 0x05, 0xC6,
	0x56, 0x86, 0x55, 0xB7, 0x8B, 0x60, 0xD4, 0x89, 0xEA, 0x31, 0xD4, 0x83, 0x37, 0xC5, 0x9D, 0x46,
#elif IPL_03G
	0x79, 0xDB, 0x85, 0x6E, 0x7A, 0x37, 0xF7, 0x3A, 0x5D, 0x85, 0x04, 0xC4, 0xC3, 0x96, 0xBC, 0xA3,
	0xFB, 0x03, 0x26, 0x11, 0x28, 0x5D, 0xFD, 0x5A, 0xE9, 0x92, 0xA1, 0xE3, 0xF7, 0xF7, 0x9E, 0xF3,
#endif
};


void set_seed()
{
	memcpy((uint8_t *) 0xBFC00200, seed, sizeof(seed));
	
	((void (*)()) SET_SEED_RETURN)();
}
#endif


uint8_t rom_hmac[] =
{
#ifdef IPL_01G
	0xc6, 0x32, 0xd1, 0xc9, 0xaa, 0x60, 0xbf, 0x39, 0x42, 0xeb, 0x0b, 0x1e, 0xaa, 0xc8, 0x27, 0x25,
	0xc7, 0x68, 0x95, 0xee, 0x01, 0xc0, 0xe9, 0xc1, 0x28, 0x8a, 0x2d, 0xe1, 0x00, 0x00, 0x00, 0x00
#elif IPL_02G
	0x8a, 0x60, 0xdc, 0x0b, 0x8c, 0x37, 0x98, 0xb7, 0xca, 0x5f, 0x68, 0xd7, 0x6c, 0x31, 0xff, 0x2e,
	0xbd, 0x71, 0x0b, 0x36, 0x7b, 0x98, 0xcf, 0x54, 0x9f, 0x19, 0x3e, 0xa3, 0x00, 0x00, 0x00, 0x00
#elif IPL_03G
	0xd0, 0x5e, 0x6b, 0xb6, 0xe7, 0xff, 0x50, 0x60, 0xde, 0x43, 0xd9, 0x05, 0xb3, 0x00, 0x0c, 0x74,
	0x25, 0x2c, 0x16, 0xe3, 0x28, 0x4f, 0xe8, 0x10, 0xe5, 0x61, 0xfb, 0x08, 0x00, 0x00, 0x00, 0x00
#endif
};

void sha256hmacPatched(uint8_t *key, uint32_t keylen, uint8_t *data, uint32_t datalen, uint8_t *out)
{
	memcpy(out, rom_hmac, sizeof(rom_hmac));
}

void prestage2()
{
	// Copy stage 2 to scratchpad
	memcpy((uint8_t *) 0x10000, &payload, size_payload);
	
	// Replace call to Dcache with jump to patch2
	_sw(0x8004000, STAGE2_PATCH_INJECTION_ADDRESS); // j 0x00010000
	_sw(0, STAGE2_PATCH_INJECTION_ADDRESS + 4);
	
	// Change payload entrypoint to 0x88fc0000
	_sw(0x3C1988FC, ENTRY_POINT_ADDRESS); // lui t9, 0x88fc

	// Nullify memset to scratch pad
	_sw(0, CLEAR_STRACHPAD_ADDRESS);

#ifdef SET_SEED_ADDRESS
	MAKE_JUMP(SET_SEED_ADDRESS, set_seed);
	_sw(0, SET_SEED_ADDRESS + 4);
#endif
	
	Dcache();
	Icache();
	
	// Execute main.bin
	MAIN_BIN();
}

uint32_t GetTachyonVersion()
{
	uint32_t ver = _lw(0xbc100040);
	
	if (ver & 0xFF000000)
		return (ver >> 8);

	return 0x100000;
}

int main()
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

	_sw(0x3c08bfc0, RESET_VECTOR_ADDRESS);
	MAKE_CALL(ROM_HMAC_ADDRESS, sha256hmacPatched);
	
	MAKE_JUMP(PRE_STAGE2_ADDR, prestage2);
	_sw(0, PRE_STAGE2_ADDR + 4);
	
	Dcache();
	Icache();
	
	((void (*)())0x40f0000)();
}
