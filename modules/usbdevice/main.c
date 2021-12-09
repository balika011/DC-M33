#include <pspsdk.h>
#include <pspkernel.h>
#include <pspiofilemgr_kernel.h>
#include <pspumd.h>
#include <systemctrl.h>

#include <stdio.h>
#include <string.h>

#include "pspusbdevice.h"

PSP_MODULE_INFO("pspUsbDev_Driver", 0x1006, 1, 0);

SceIoDeviceTable *lflash_driver;
SceIoDeviceTable *umd_driver;
SceIoDeviceTable *msstor_driver;
int device_unit;
int read_only = 0;
u32 device_sizes[5];
int umd_vfat_size = 0x3CF800; 
int umd_vfat_ptr;
int umd_file_size; 
int umd_file_clusters;
int umd_last_sector;

SceUID umd_fpl;
u8  *umd_sector;

u8 unassigned_devices[4];

int (* Orig_df_open)(SceIoIob *iob, char *file, int flags, SceMode mode);
int (* Orig_df_close)(SceIoIob *iob);
int (* Orig_df_read)(SceIoIob *iob, char *data, int len);
int (* Orig_df_write)(SceIoIob *iob, const char *data, int len);
SceOff(* Orig_df_lseek)(SceIoIob *iob, SceOff ofs, int whence);
int (* Orig_df_ioctl)(SceIoIob *iob, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);
int (* Orig_df_devctl)(SceIoIob *iob, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen);


static int Lflash_IoOpen(SceIoIob *iob, char *file, int flags, SceMode mode)
{
	switch (device_unit)
	{
		case 0:
			file = "0,0";
		break;

		case 1:
			file = "0,1";
		break;

		case 2:
			file = "0,2";
		break;

		case 3:
			file = "0,3";
		break;

		default:
			return 0x800200d2;
	}

	return lflash_driver->dt_func->df_open(iob, file, flags, mode);
}

static int Common_IoWrite(SceIoIob *iob, const char *data, int len)
{
	if (read_only)
		return -1;
	else
		return lflash_driver->dt_func->df_write(iob, data, len);
}

static u8 data_5803[96] = 
{
	0x02, 0x00, 0x08, 0x00, 0x08, 0x00, 0x07, 0x9F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x21, 0x21, 0x00, 0x00, 0x20, 0x01, 0x08, 0x00, 0x02, 0x00, 0x02, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int Common_IoIoctl(SceIoIob *iob, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	if (cmd == 0x02125008)
	{
		u32 *x = (u32 *)outdata;
		if (!read_only)
			*x = 1; // Enable writing 
		else
			*x = 0;
		return 0;
	}
	else if (cmd == 0x02125803)
	{
		memcpy(outdata, data_5803, 96);
		return 0;
	}
	else if (cmd == 0x02125006)
	{
		*(u32 *)outdata = device_sizes[device_unit];	
		return 0;
	}
	else if (cmd == 0x0211D032)
	{
		return Common_IoWrite(iob, outdata, outlen);
	}	
	else if (cmd == 0x0201D033)
	{
		return 0;
	}

	return 0;
}

static int Common_IoDevctl(SceIoIob *iob, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	if (cmd == 0x02125801)
	{
		u8 *data8 = (u8 *)outdata;

		data8[0] = 1;
		data8[1] = 0;
		data8[2] = 0,
		data8[3] = 1;
		data8[4] = 0;
				
		return 0;
	}

	return 0x80010086;
}

static int UmdVfat_Open(SceIoIob *iob, char *file, int flags, SceMode mode)
{
	umd_vfat_ptr = 0;
	mode = PSP_O_RDONLY;
	umd_last_sector = -1;
	
	return umd_driver->dt_func->df_open(iob, file, flags, mode);
}

u8 boot_sector[0x40] = 
{
	0xEB, 0x00, 0x00, 0x4D, 0x41, 0x52, 0x43, 0x48, 0x20, 0x33, 0x33, 0x00, 0x02, 0x40, 0x08, 0x00, 
	0x02, 0x00, 0x02, 0x00, 0x00, 0xF8, 0xF4, 0x00, 0x3F, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xF8, 0x3C, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x4F, 0x20, 0x4E, 0x41, 
	0x4D, 0x45, 0x20, 0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x31, 0x36, 0x20, 0x20, 0x20, 0x00, 0x00
};

u8 file_entry[0x40] = 
{
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x08, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x47, 0x0F, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x55, 0x4D, 0x44, 0x39, 0x36, 0x36, 0x30, 0x20, 0x49, 0x53, 0x4F, 0x21, 0x00, 0x00, 0x31, 0x1C, 
	0x82, 0x36, 0x82, 0x36, 0x00, 0x00, 0x31, 0x1C, 0x82, 0x36, 0x02, 0x00, 0x33, 0x33, 0x33, 0x33
};

static int ReadAllocTableSector(u32 fatsec, u16 *fat)
{
	int i;
	u32 cluster;

	if (fatsec == 0)
	{
		fat[0] = 0xFFF8;
		fat[1] = 0xFFFF;

		i = 2;
		cluster = 0;
	}
	else
	{
		i = 0;	
		cluster = (0x100*fatsec)-2;
	}	

	if (cluster >= umd_file_clusters)
		return 0;

	for (; i < 0x100; i++, cluster++)
	{
		if (cluster == (umd_file_clusters-1))
		{
			fat[i] = 0xFFFF;
			break;
		}

		fat[i] = (cluster+3);
	}

	return 0;
}

static int ReadUmdSector(SceIoIob *iob, u32 sector)
{
	if (sector == umd_last_sector)
	{
		return 0;
	}

	if (umd_last_sector != (sector-1))
	{
		if (umd_driver->dt_func->df_lseek(iob, sector, PSP_SEEK_SET) < 0)
		{
			return -1;
		}
	}

	int res = umd_driver->dt_func->df_read(iob, (void *)umd_sector, 1);
	if (res >= 0)
	{
		umd_last_sector = sector;
	}

	return res;
}

static int ReadUmdVfatSector(SceIoIob *iob, u32 sector, u8 *buf)
{
	if (sector == 0)
	{
		memcpy(buf, boot_sector, 0x40);
		buf[0x1FE] = 0x55;
		buf[0x1FF] = 0xAA;
	}
	else if (sector >= 8 && sector < 0xFC)
	{
		return ReadAllocTableSector(sector-8, (u16 *)buf);
	}
	else if (sector >= 0xFC && sector < 0x1F0)
	{	
		return ReadAllocTableSector(sector-0xFC, (u16 *)buf);
	}
	else if (sector == 0x1F0)
	{
		memcpy(buf, file_entry, 0x40);
	}
	
	return 0;
}

static int UmdVfat_Read(SceIoIob *iob, char *data, int len)
{
	int i, read = 0;
	int rem;
	
	if (data == NULL || len == 0 || (len & 0x1FF))
	{
		return 0x80010016;
	}

	memset(data, 0, len);

	len /= 0x200;
	rem = len;

	if (umd_vfat_ptr >= 0x210)
	{
		int s = umd_vfat_ptr-0x210;
		int lba = s / 4;

		if (s < umd_file_size)
		{
			int x = (s & 3);
			int move = 1;

			if (s + rem >= umd_vfat_size)
			{
				rem = umd_vfat_size-s;
			}

			if (x && rem >= 0)
			{
				ReadUmdSector(iob, lba);

				int m = 4-x;

				if (m > rem)
					m = rem;

				memcpy(data, umd_sector+(x*0x200), m*0x200);
				data += (m*0x200);
				lba++;
				rem -= m;
				move = 0;
				s += m;
			}
			
			if (rem >= 0)
			{
				if (move)
				{
					umd_driver->dt_func->df_lseek(iob, lba, PSP_SEEK_SET);
				}

				int nsectors = rem / 4;

				if (nsectors > 0)
				{
					if (umd_driver->dt_func->df_read(iob, data, nsectors) < 0)
						return -1;

					data += (nsectors*0x800);
					lba += nsectors;
					rem -= (nsectors * 4);
					s += (nsectors * 4);
				}
				
				if (rem > 0)
				{
					ReadUmdSector(iob, lba);
					memcpy(data, umd_sector, rem*0x200);
				}
			}

			umd_vfat_ptr += len;
			return (len*0x200);
		}		
	}

	for (i = 0; i < len; i++)
	{
		if (ReadUmdVfatSector(iob, umd_vfat_ptr, (u8 *)data) != 0)
		{
			break;
		}

		umd_vfat_ptr++;
		data += 0x200;
		read += 0x200;
	}

	return read;
}

SceOff UmdVfat_IoLseek(SceIoIob *iob, SceOff ofs, int whence)
{
	int ofs32 = (int)ofs;
	
	if (ofs32 & 0x1FF)
	{
		return 0x80010016;
	}

	int s_ofs = (ofs32/0x200);
	
	if (whence == PSP_SEEK_SET)
	{
		umd_vfat_ptr = s_ofs;
	}
	else if (whence == PSP_SEEK_CUR)
	{
		umd_vfat_ptr += s_ofs;
	}
	else if (whence == PSP_SEEK_END)
	{
		umd_vfat_ptr = umd_vfat_size+s_ofs;
	}
	else
	{
		return 0x80010016;
	}
	
	return (umd_vfat_ptr*0x200);
}

int pspUsbDeviceSetDevice(u32 unit, int ronly, int unassign_mask)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;
	
	if (unit > PSP_USBDEVICE_UMD9660)
	{
		pspSdkSetK1(k1);
		return 0x80010016;
	}

	device_unit = unit;
	memset(unassigned_devices, 0, sizeof(unassigned_devices));
	umd_sector = NULL;

	if (unit == PSP_USBDEVICE_UMD9660)
	{
		u8 *pvd;
		
		if (!sceUmdCheckMedium())
		{
			if (sceUmdWaitDriveStatCB(UMD_WAITFORDISC, 2*1000*1000) < 0)
			{
				res = 0x80210003;
				goto RETURN;
			}
		}

		sceUmdActivate(1, "disc0:"); 
		sceUmdWaitDriveStat(UMD_WAITFORINIT);
		
		res = sceIoDevctl("umd0:", 0x01f20002, NULL, 0, &umd_file_size, 4);
		if (res < 0)
		{
			goto RETURN;
		}

		if (sceIoDevctl("umd0:", 0x01e28035, NULL, 0, &pvd, 4) >= 0)
		{
			if (memcmp(pvd+1, "CD001", 5) == 0)
			{
				u32 p = 0;
				char *ptr = strchr((char *)(pvd+0x373), '|');		
				
				if (ptr)
					p = (u32)((u32)ptr - (u32)(pvd+0x373));

				if (p > 11)
					p = 11;	
				
				memcpy(boot_sector+0x2B, pvd+0x373, p);
				memcpy(file_entry, pvd+0x373, p);
			}
		}
		
		umd_file_size *= 4;		
		umd_file_clusters = (umd_file_size / 0x40);

		if (umd_file_size & 0x3F)
		{
			umd_file_clusters++;
		}

		umd_fpl = sceKernelCreateFpl("UmdSector", PSP_MEMORY_PARTITION_KERNEL, 0, 0x800, 1, NULL);
		if (umd_fpl < 0)
		{
			res = umd_fpl;
			goto RETURN;
		}

		sceKernelAllocateFpl(umd_fpl, (void *)&umd_sector, NULL);
		
		read_only = 1;
		device_sizes[unit] = umd_vfat_size;
		*(u32 *)&file_entry[0x3C] = umd_file_size*0x200;
	}
	else
	{		
		read_only = ronly;
	
		if (unassign_mask & UNASSIGN_MASK_FLASH0)
		{
			res = sceIoUnassign("flash0:");
			if (res >= 0)
			{
				unassigned_devices[0] = 1;
			}
			else if (res != SCE_KERNEL_ERROR_NODEV)
			{
				goto RETURN;
			}
		}

		if (unassign_mask & UNASSIGN_MASK_FLASH1)
		{		
			res = sceIoUnassign("flash1:");
			if (res >= 0)
			{
				unassigned_devices[1] = 1;
			}
			else if (res != SCE_KERNEL_ERROR_NODEV)
			{
				goto RETURN;
			}
		}

		if (unassign_mask & UNASSIGN_MASK_FLASH2)
		{
			res = sceIoUnassign("flash2:");
			if (res >= 0)
			{
				unassigned_devices[2] = 1;
			}
			else if (res != SCE_KERNEL_ERROR_NODEV)
			{
				goto RETURN;
			}
		}
	
		if (unassign_mask & UNASSIGN_MASK_FLASH3)
		{
			res = sceIoUnassign("flash3:");
			if (res >= 0)
			{
				unassigned_devices[3] = 1;
			}
			else if (res != SCE_KERNEL_ERROR_NODEV)
			{
				goto RETURN;
			}
		}
	}

	int intr = sceKernelCpuSuspendIntr();

	if (unit == PSP_USBDEVICE_UMD9660)
	{
		msstor_driver->dt_func->df_open = UmdVfat_Open;
		msstor_driver->dt_func->df_close = umd_driver->dt_func->df_close;
		msstor_driver->dt_func->df_read = UmdVfat_Read;
		msstor_driver->dt_func->df_lseek = UmdVfat_IoLseek;
	}
	else
	{
		msstor_driver->dt_func->df_open = Lflash_IoOpen;
		msstor_driver->dt_func->df_close = lflash_driver->dt_func->df_close;
		msstor_driver->dt_func->df_read = lflash_driver->dt_func->df_read;
		msstor_driver->dt_func->df_lseek = lflash_driver->dt_func->df_lseek;
	}

	msstor_driver->dt_func->df_write =  Common_IoWrite;	
	msstor_driver->dt_func->df_ioctl =  Common_IoIoctl;
	msstor_driver->dt_func->df_devctl = Common_IoDevctl;

	sceKernelCpuResumeIntr(intr);

RETURN:
	
	pspSdkSetK1(k1);
	return res;
}

int pspUsbDeviceFinishDevice()
{
	int k1 = pspSdkSetK1(0);
	int res = 0;
	
	int intr = sceKernelCpuSuspendIntr();

	msstor_driver->dt_func->df_open = Orig_df_open;
	msstor_driver->dt_func->df_close = Orig_df_close;
	msstor_driver->dt_func->df_read = Orig_df_read;
	msstor_driver->dt_func->df_write = Orig_df_write;
	msstor_driver->dt_func->df_lseek = Orig_df_lseek;
	msstor_driver->dt_func->df_ioctl = Orig_df_ioctl;
	msstor_driver->dt_func->df_devctl = Orig_df_devctl;

	sceKernelCpuResumeIntr(intr);

	if (umd_sector)
	{
		sceKernelFreeFpl(umd_fpl, umd_sector);
		sceKernelDeleteFpl(umd_fpl);
		umd_sector = NULL;
	}

	if (unassigned_devices[0])
	{
		res = sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, NULL, 0);
		if (res < 0)
		{
			goto RETURN;
		}
	}

	if (unassigned_devices[1])
	{
		res = sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);
		if (res < 0)
		{
			goto RETURN;
		}
	}

	if (unassigned_devices[2])
	{
		res = sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);
		if (res < 0)
		{
			goto RETURN;
		}
	}

	if (unassigned_devices[3])
	{
		res = sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, NULL, 0);
		if (res < 0)
		{
			goto RETURN;
		}
	}

RETURN:
	
	pspSdkSetK1(k1);
	return res;
}

int module_start(SceSize args, void *argp)
{
	SceUID fpl, fd;
	u8 *outdata;
	int i;
	char dev[11];	
	
	strcpy(dev, "lflash0:0,0");
	
	lflash_driver = sctrlHENFindDriver("lflash");
	umd_driver = sctrlHENFindDriver("umd");
	msstor_driver = sctrlHENFindDriver("msstor");

	if (!lflash_driver || !msstor_driver)
	{
		return -1;
	}

	fpl = sceKernelCreateFpl("", PSP_MEMORY_PARTITION_KERNEL, 0, 0x200, 1, NULL);

	if (fpl < 0)
	{
		return fpl;
	}

	sceKernelAllocateFpl(fpl, (void *)&outdata, NULL);

	for (i = 0; i < 4; i++)
	{
		dev[10] = '0'+i;
		fd = sceIoOpen(dev, PSP_O_RDONLY, 0);
		if (fd >= 0)
		{
			if (sceIoIoctl(fd, 0x0003d001, NULL, 0, outdata, 0x44) >= 0)
			{
				device_sizes[i] = *(u32 *)&outdata[0x14];				
			}

			sceIoClose(fd);
		}
	}
		
	Orig_df_open = msstor_driver->dt_func->df_open;
	Orig_df_close = msstor_driver->dt_func->df_close;
	Orig_df_read = msstor_driver->dt_func->df_read;
	Orig_df_write = msstor_driver->dt_func->df_write;
	Orig_df_lseek = msstor_driver->dt_func->df_lseek;
	Orig_df_ioctl = msstor_driver->dt_func->df_ioctl;
	Orig_df_devctl = msstor_driver->dt_func->df_devctl;

	sceKernelFreeFpl(fpl, outdata);
	sceKernelDeleteFpl(fpl);
	
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}

