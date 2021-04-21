#ifndef __PSP_NAND_H__
#define __PSP_NAND_H__

int pspNandReadSector(int sector, void *data, u32 *spare);
int pspNandReadIPLFat(void);
int pspNandReadIPLBlock(int block, void *data);

#endif
