/****************************************************************************
	PSP IPL NAND Flash Driver
****************************************************************************/
#include <psptypes.h>
#include "nand.h"

// 8MB buffer for data
#define BUFFER_SIZE (8*1024*1024)
unsigned char buf_data[BUFFER_SIZE];

// 1MB for all area of EXTRA data
unsigned char buf_extra[0x10 * 0x20 * 0x800];

/****************************************************************************
NAND_CTRL(0xbd101004):bit assign

R/-B : 0x01
???  : 0x02
-CLE : 0x04
-WE  : 0x08
CE   : 0x10
???  : 0x20
???  : 0x40
WP   : 0x80
-RE  : 0x100 or 0x200
ALE  : 0x200 or 0x100

****************************************************************************/

/* NAND I/F register s */


#define NAND_STS  (*(volatile u32 *)(0xbd101004))
#define NAND_CTRL (*(volatile u32 *)(0xbd101004))
#define NAND_CMD  (*(volatile u32 *)(0xbd101008))
#define NAND_ADR  (*(volatile u32 *)(0xbd10100c))
#define NAND_END  (*(volatile u32 *)(0xbd101014))

#define NAND_AUTO_ADR (*(volatile u32 *)(0xbd101020))
#define NAND_AUTO_CMD (*(volatile u32 *)(0xbd101024))
#define NAND_AUTO_STS (*(volatile u32 *)(0xbd101024))
#define NAND_AUTO_ERR (*(volatile u32 *)(0xbd101028))

#define NAND_MODE (*(volatile u32 *)(0xbd101000))
#define NAND_DAT  (*(volatile u32 *)(0xbd101300))

#define NAND_DATA  ((volatile u32 *)(0xbff00000))
#define NAND_SPARE ((volatile u32 *)(0xbff00900))

/****************************************************************************
	reset CMD
****************************************************************************/
static void pspNandReset(void)
{
	NAND_CMD = 0xff;
	while((NAND_STS & 0x01) == 0);
	NAND_END = 0x01;
}

/****************************************************************************
****************************************************************************/
int pspNandReadSector(int sector, void *data, u32 *spare)
{
	while ((NAND_STS & 0x01) == 0);

	// start command
	NAND_AUTO_ADR = sector << 10;
	NAND_AUTO_CMD = 0x0301;

	while(NAND_AUTO_STS & 1);

	if(NAND_AUTO_ERR)
		return -1;

	for (int i = 0; i < 3; i++)
		spare[i] = NAND_SPARE[i];

	for (int i = 0; i < 128; i++)
		((uint32_t *) data)[i] = NAND_DATA[i];

	return 0;
}

/****************************************************************************
****************************************************************************/
static u16 iplfat_buf[0x200/2];
static u32 spare_buf[3];

int pspNandReadIPLFat(void)
{
	pspNandReset();

	for (int sector = 0x80; spare_buf[1] != 0x6dc64a38; sector+=0x20)
	{
		if(pspNandReadSector(sector, iplfat_buf, spare_buf) < 0)
			return 0x8001018c;
	}
	
	return 0;
}

/****************************************************************************
****************************************************************************/
int pspNandReadIPLBlock(int block, void *data)
{
	int sector = ((iplfat_buf[block >> 2] << 2) | (block & 3)) * 8;
	if (!sector)
		return -1;
	for(int i = 0; i < 8; i++)
	{
		if(pspNandReadSector(sector + i, &((uint8_t *) data)[i * 0x200], spare_buf) < 0)
			return -1;
		if(spare_buf[1] != 0x6dc64a38)
			return -1;
	}
	return 0;
}

