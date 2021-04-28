#include <pspsdk.h>
#include <string.h>

#include "sysreg.h"
#include "kirk.h"
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
#define SET_SEED_ADDRESS 0x4001134

#elif IPL_03G

#define RESET_VECTOR_ADDRESS 0x040f0118
#define ROM_HMAC_ADDRESS 0x40f0fe4
#define PRE_STAGE2_ADDR 0x40f02a8

#define STAGE2_PATCH_INJECTION_ADDRESS 0x400035C
#define ENTRY_POINT_ADDRESS 0x0400D534
#define CLEAR_STRACHPAD_ADDRESS 0x4001218
#define SET_SEED_ADDRESS 0x4001168

#else

#error Define IPL

#endif

extern u32 size_payload;
extern u8 payload;
void Dcache();
void Icache();
#define MAIN_BIN() ((void (*)()) 0x4000000)()

void *memset(void *dest, int value, size_t size)
{
	u8 *d = (u8 *) dest;
	
	while (size--)
		*(d++) = value;
	
	return dest;
}

void *memcpy(void *dest, const void *src, size_t size)
{
	u8 *d = (u8 *) dest;
	u8 *s = (u8 *) src;
	
	while (size--)
		*(d++) = *(s++);

	return dest;
}

int unlockSyscon()
{
	KirkReset();
	
	KirkCmd15();
	
	u8 random_key[16];
	u8 random_key_dec_resp_dec[16];
	
	int ret = seed_gen1(random_key, random_key_dec_resp_dec);
	if (ret)
		return ret;
	
	ret = seed_gen2(random_key, random_key_dec_resp_dec);
	if (ret)
		return ret;
	
	KirkReset();
	
	return 0;
}

#ifdef SET_SEED_ADDRESS
u8 seed_xor[] = {
#if IPL_02G
	0xae, 0xfa, 0xba, 0xcf, 0x1c, 0x5f, 0x88, 0x7e, 0xf1, 0x5d, 0xe4, 0xaf, 0xe1, 0xab, 0x51, 0xf6
#elif IPL_03G
	0x5d, 0x88, 0x4e, 0x46, 0xf5, 0x3f, 0x5e, 0xaa, 0x72, 0x48, 0x36, 0x80, 0xe6, 0x8d, 0xc2, 0xbc
#endif
};


int set_seed(u8 *xor_key, u8 *random_key, u8 *random_key_dec_resp_dec)
{
	for (int i = 0; i < sizeof(seed_xor); i++)
		*(u8 *) (0xBFC00210 + i) ^= seed_xor[i];

	return 0;
}
#endif


u8 rom_hmac[] =
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

void sha256hmacPatched(u8 *key, u32 keylen, u8 *data, u32 datalen, u8 *out)
{
	memcpy(out, rom_hmac, sizeof(rom_hmac));
}

void prestage2()
{
	// Copy stage 2 to scratchpad
	memcpy((u8 *) 0x10000, &payload, size_payload);
	
	// Replace call to Dcache with jump to patch2
	_sw(0x8004000, STAGE2_PATCH_INJECTION_ADDRESS); // j 0x00010000
	_sw(0, STAGE2_PATCH_INJECTION_ADDRESS + 4);
	
	// Change payload entrypoint to 0x88fc0000
	_sw(0x3C1988FC, ENTRY_POINT_ADDRESS); // lui t9, 0x88fc

	// Nullify memset to scratch pad
	_sw(0, CLEAR_STRACHPAD_ADDRESS);

#ifdef SET_SEED_ADDRESS
	MAKE_CALL(SET_SEED_ADDRESS, set_seed);
#endif
	
	Dcache();
	Icache();
	
	// Execute main.bin
	MAIN_BIN();
}

u32 GetTachyonVersion()
{
	u32 ver = _lw(0xbc100040);
	
	if (ver & 0xFF000000)
		return (ver >> 8);

	return 0x100000;
}

int main()
{
	pspSyscon_init();

	u32 baryon_version = 0;
	while (pspSysconGetBaryonVersion(&baryon_version) < 0);
	
	while (pspSyscon_driver_Unkonow_7bcc5eae(0) < 0);

	u32 tachyon_version = GetTachyonVersion();

#ifndef MSIPL
#ifdef SET_SEED_ADDRESS
	unlockSyscon();
#endif

	uint32_t keys = -1;
	pspSysconGetCtrl1(&keys);
	if ((keys & (SYSCON_CTRL_LTRG | SYSCON_CTRL_HOME)) == 0)
	{
		pspSysconCrlMsPower(1);
	
		_sw(0x00000000, 0x80010068);

		if (tachyon_version >= 0x600000)
		{
			_sw(0x00000000, 0x80010A8C);
		}
		else
		{
			_sw(0x00000000, 0x8001080C);
		}
	
		Dcache();
		Icache();

		return ((int (*)())0x80010000)();
	}
#endif

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
