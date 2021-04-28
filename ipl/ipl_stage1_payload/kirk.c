#include <psptypes.h>
#include <string.h>
#include "sysreg.h"

#define PHYS_TO_HW(x) (((u32) x) & 0x1FFFFFFF)

struct KIRK
{
	vu32 Signature;
	vu32 Version;
	vu32 Error;
	vu32 StartProcessing;
	vu32 Command;
	vu32 Result;
	vu32 field_18;
	vu32 Pattern;
	vu32 ASyncPattern;
	vu32 ASyncPatternEnd;
	vu32 PatternEnd;
	vu32 SourceAddr;
	vu32 DestAddr;
};

#define KIRK_MODE_CMD1 1
#define KIRK_MODE_CMD2 2
#define KIRK_MODE_CMD3 3
#define KIRK_MODE_ENCRYPT_CBC 4
#define KIRK_MODE_DECRYPT_CBC 5
struct KIRK_AES128CBC_HEADER
{
	u32 mode;
	u32 unk_4;
	u32 unk_8;
	u32 keyseed;
	u32 data_size;
};

struct KIRK *g_KIRK = (void *) 0xBDE00000;

void KirkReset()
{
	SysregResetKirkEnable();
	SysregBusclkKirkEnable();
	SysregBusclkKirkDisable();
	SysregResetKirkDisable();
	SysregBusclkKirkEnable();
}

void KirkCmd15()
{
	g_KIRK->Command = 15;
	g_KIRK->SourceAddr = PHYS_TO_HW(0xBFC00C00);
	g_KIRK->DestAddr = PHYS_TO_HW(0xBFC00C00);
	g_KIRK->StartProcessing = 1;
	asm("sync"::);
	while ((g_KIRK->StartProcessing & 1) != 0);
	while (!g_KIRK->Pattern);
	g_KIRK->PatternEnd = g_KIRK->Pattern & g_KIRK->ASyncPattern;
	asm("sync"::);
}

int kirkDecryptAes(u8 *out, u8 *data, u32 size, u8 key_idx)
{
	struct KIRK_AES128CBC_HEADER *header = (void *) 0xBFC00C00;
	memset(header, 0, 0x40);
	header->mode = KIRK_MODE_DECRYPT_CBC;
	header->keyseed = key_idx;
	header->data_size = size;
	memcpy(&header[1], data, size);
	asm("sync"::);
	g_KIRK->Command = 7;
	g_KIRK->SourceAddr = PHYS_TO_HW(header);
	g_KIRK->DestAddr = PHYS_TO_HW(header);
	g_KIRK->StartProcessing = 1;
	asm("sync"::);
	while ((g_KIRK->StartProcessing & 1) != 0);
	while (!g_KIRK->Pattern);
	int res = g_KIRK->Result;
	g_KIRK->PatternEnd = g_KIRK->Pattern & g_KIRK->ASyncPattern;
	asm("sync"::);
	
	memcpy(out, header, size);

	memset(header, 0, 0x400);

	if (res)
		return -1;

	return 0;
}
