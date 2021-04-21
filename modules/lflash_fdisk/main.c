#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <psploadcore.h>
#include <pspiofilemgr_kernel.h>
#include <pspsyscon.h>
#include <pspnand_driver.h>
#include <pspidstorage.h>
#include <pspdisplay_kernel.h>
#include <systemctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


PSP_MODULE_INFO("pspLflashFdisk", 0x1007, 1, 0);

typedef struct PartitionStruct
{
	int partsize;
	int _4;	
} PartitionStruct;

typedef struct LflashParam
{
	u16 version;    // 0x00
    u8 location[7]; // 0x02
    u8 pad;
    u16 vendorNum;  // 0x0A
    u16 productNum; // 0x0C
	u16 pad2;
    int type;   // 0x10
    int blocks; // 0x14
    int bsize;  // 0x18
    int bits;   // 0x1C
    int flags;  // 0x20    
    char vendor[0x10];  // 0x24
    char product[0x10]; // 0x34
} LflashParam;

char device[64];
LflashParam lflash_param;
u8 buffer[0x1000];


PartitionStruct partitions[16];

int debug, lba;
u32 numpart, maxsectors, numheads, numsectors, totalsectors;

void InitVars()
{
	debug = 0;
	lba = 0;
	
	memset(device, 0, sizeof(device));
	memset(&lflash_param, 0, sizeof(lflash_param));
	memset(buffer, 0, sizeof(buffer));
	memset(partitions, 0, sizeof(partitions));

	numpart = 0;
	maxsectors = 0;
	numheads = 0;
	numsectors = 0;
	totalsectors = 0;
}

int AddPartition(int size)
{	
	if (numpart >= 16)
	{
		// Kprintf("Too many partitions.\n");
		return -1;
	}

	partitions[numpart].partsize = size;
	numpart++;
	
	return 0;
}

int write_sector(SceUID fd, u8 *buf, int block)
{
    SceOff offset = block << lflash_param.bits;
    SceOff pos = sceIoLseek(fd, offset, PSP_SEEK_SET);
    if (pos != offset)
    {
        //Kprintf("-> seek error (blkno=%d) (r=%d)\n", block, pos);
        return -1;
    }

    int blkSize = lflash_param.bsize;
    int res = sceIoWrite(fd, buf, blkSize);
    if (res != blkSize)
    {
        //Kprintf("-> write error (blkno=%d) (r=%d)\n", blknum, bytes_written);
        return -1;
    }

    return 1;
}

int SetCHS(u8 *chs, int unk1)
{
	int res = 0;
	int h, cs1, cs2;
	int y;

	cs2 = (unk1 / numsectors) / numheads;
	h = (unk1 / numsectors) % numheads;
	y = (unk1 % numsectors) & 0x3F;
	
	if (cs2 >= maxsectors)
	{
		cs2 = maxsectors-1;
		res = 1;
	}
	
	cs1 = ((cs2 & 0x300) >> 2) | y;

	chs[0] = h;
	chs[1] = cs1;
	chs[2] = cs2;

	return res;
}

int create_partition_record(u8 *partRec, int unk1, u32 partLen, int isExtPart, int unk4) // create primary partition??
{    
    int res1 = SetCHS(partRec+1, unk1); // CHS of first sector of partition
    int res2 = SetCHS(partRec+5, unk1+partLen-1); // CHS of last sector of partition
	u32 sectorsbefore;  
	int partType;
	
	if (isExtPart) // is an extended partition
    {
        if ((res1 + res2 == 0) && (lba == 0))
            partType = 0x5; // Extended Partition
        else
            partType = 0xF; // LBA-mapped Extended partition
    }
    else 
    {
        if ((res1 + res2 == 0) && (lba == 0)) // CHS addressing
        {
            if (partLen >= 0x800000)
                partType = 0xB; // FAT32
            else if (partLen >= 0x10000)
                partType = 0x6; // FAT16 (over 32M)
            else
                partType = 0x1; // FAT12
        }
        else // LBA addressing
        {
            if (partLen >= 0x800000)
                partType = 0xC; // LBA-mapped FAT32
            else
                partType = 0xE; // LBA-mapped FAT16
        }
    }

	sectorsbefore = unk1-unk4;

    // partition type
    partRec[0x4] = partType;

    // LBA of the first sector in the partition
    partRec[0x8] = sectorsbefore;
    partRec[0x9] = sectorsbefore >> 8;
    partRec[0xA] = sectorsbefore >> 16;
    partRec[0xB] = sectorsbefore >> 24;

    // Partition length (in sectors)
    partRec[0xC] = partLen;
    partRec[0xD] = partLen >>  8;
    partRec[0xE] = partLen >> 16;
    partRec[0xF] = partLen >> 24;    
    
    return 0;
}

int write_partition_table(SceUID fd, int blknum, int numblocks, int numSectF1)
{
    memset(buffer, 0, sizeof(buffer));

    if (blknum != 0)
    {
        create_partition_record(buffer+0x1BE, blknum+numsectors, numblocks-numsectors, 0, blknum);

        if (numSectF1) // flash1 ??
            create_partition_record(buffer+0x1CE, blknum+numblocks, numSectF1, 1, totalsectors);
    }
    else
    {
        create_partition_record(buffer+0x1BE, totalsectors, numblocks, 1, 0);
    }

    buffer[0x1FE] = 0x55;
    buffer[0x1FF] = 0xAA;

    int res = write_sector(fd, buffer, blknum);

    return((res < 0) ? -1 : 0);
}

int DoFdisk(SceUID fd)
{
	int i, p;
	int numblocks;
	
	if (numheads != 0 && numsectors != 0)
	{
		totalsectors = numheads*numsectors; // totalsectors
		maxsectors = (lflash_param.blocks+totalsectors-1) / totalsectors; // maxsectors

		if (maxsectors > 1024)
			maxsectors = 1024;
	}
	else
	{
		numsectors = 32;
		maxsectors = 1024;
		totalsectors = (lflash_param.blocks+maxsectors-1 / maxsectors);
		numheads = (totalsectors+numsectors-1) / numsectors;

		if (numheads > 256)
		{
			numheads = 256;
		}
		else if (numheads == 1)
		{
			numsectors = numheads;

			while (numsectors < totalsectors)
			{
				numsectors *= 2;

				if (numsectors >= 32)
					break;
			}
		}

		totalsectors = numheads*numsectors;
	}

	for (i = 0, numblocks = 0; i < numpart; i++)
	{
		partitions[i]._4 = partitions[i].partsize * totalsectors;

		if (partitions[i]._4 < numsectors)
		{
			partitions[i]._4 = lflash_param.blocks-totalsectors-numblocks;
			partitions[i]._4 = (partitions[i]._4 / totalsectors) * totalsectors;
		}

		numblocks += partitions[i]._4;
	}

	if ((lflash_param.blocks-totalsectors) < numblocks)
	{
		// Kprintf("-> too many blocks for the drive (%d < %d)\n");
		return -1;
	}

	if (write_partition_table(fd, 0, numblocks, 0) < 0)
	{
		Kprintf("write_partition_table failed.\n");
		return -2;
	}

	for (i = 0, p = totalsectors; i < numpart; i++)
	{
		if (write_partition_table(fd, p, partitions[i]._4, (i < (numpart-1)) ? partitions[i+1]._4 : 0 ) < 0)
		{
			Kprintf("write_partition_table %d failed.\n", i);
			return -3;
		}

		p += partitions[i]._4;
	}
	
	return 0;
}

// fdisk -H 2 -S 32 768 128 32 0 0 0 lflash0:0

void Usage()
{

}

int pspLflashFdiskStartFdisk(int argc, char *argv[])
{
	int i, res;
	SceUID fd;

	Kprintf("Lflash fdisk 0.1\n");
	
	InitVars();
	
	if (argc <= 1)
	{
		Usage();
		return -1;
	}	

	for (i = 1; i < argc-1; i++)
	{	
		if (argv[i][0] == '-')
		{
			/*if (strcmp(argv[i], "-debug") == 0)
			{
				debug = 1;			
			}
			else if (strcmp(argv[i], "-lba") == 0)
			{
				lba = 1;
			}
			else */if (strcmp(argv[i], "-H") == 0)
			{
				i++;
				
				if (i == (argc-1))
				{
					Usage();
					return -1;
				}

				numheads = strtol(argv[i], NULL, 0);
				if (numheads <= 0 || numheads >= 256)
				{
					Usage();
					return -1;
				}
			}
			else if (strcmp(argv[i], "-S") == 0)
			{
				i++;

				if (i == (argc-1))
				{
					Usage();
					return -1;
				}

				numsectors = strtol(argv[i], NULL, 0);
				if (numsectors <= 0 || numsectors >= 63)
				{
					Usage();
					return -1;
				}
			}
			else
			{
				Usage();
				return -1;
			}
		}
		else if (look_ctype_table(argv[i][0]) & 4)
		{
			res = AddPartition(strtol(argv[i], NULL, 0));
			if (res < 0)
				return res;
		}
	}

	strcpy(device, argv[i]);
	strcat(device, ",single");

	fd = sceIoOpen(device, PSP_O_RDWR | PSP_O_EXCL, 0);
	if (fd < 0)
		return fd;

	res = sceIoIoctl(fd, 0x3d001, NULL, 0, &lflash_param, sizeof(lflash_param));
	if (res < 0)
	{
		sceIoClose(fd);
		return res;
	}

	res = DoFdisk(fd);
	sceIoClose(fd);
	
	return res;
}

int module_start(SceSize args, void *argp)
{
	return 0;
}

