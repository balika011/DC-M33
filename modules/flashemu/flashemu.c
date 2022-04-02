#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspinit.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>
#include <systemctrl.h>

#include <stdio.h>
#include <string.h>

#include "flashemu.h"

#define DC_PATH "/TM/DC10"

SceUID assign_thread = -1;

void WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	sceIoWrite(fd, buf, size);
	sceIoClose(fd);
}

SceIoDeviceEntry *getMSEntry()
{
	SceIoDeviceEntry *ms0 = 0;
	while (1)
	{
		ms0 = sctrlHENFindEntry("ms0:");
		if (ms0)
			break;
		sceKernelDelayThread(20000);
	}
	return ms0;
}

static int FlashEmu_df_init(SceIoDeviceEntry* de)
{
	// Kprintf("flashemu: IoInit.\n");
	
	// plenty, because iofilemgr can only have 64 open files a time
	de->d_private = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, 0x4000, 1, "");
	
	return 0;
}

static int FlashEmu_df_exit(SceIoDeviceEntry* de)
{
	// Kprintf("flashemu: IoExit.\n");
	
	sceKernelDeleteHeap(de->d_private);

	return 0;
}

typedef struct FlashEmuPrivate
{
	int virtual;
	union
	{
		struct
		{
			u8 *buffer;
			int offset;
			u32 size;
		} virtualinfo;
		void *ms_private;
	} uni;
} FlashEmuPrivate;

extern u8 cptbl;
extern u32 size_cptbl;

static int FlashEmu_df_open(SceIoIob *iob, char *file, int flags, SceMode mode)
{
	// Kprintf("flashemu: FlashEmu_df_Open(%x, %s, %x, %x)\n", iob, file, flags, mode);
	
	if (strcmp("/codepage/cptbl.dat", file) == 0)
	{
		FlashEmuPrivate *fep = sceKernelAllocHeapMemory(iob->i_de->d_private, sizeof(FlashEmuPrivate));		
		fep->virtual = 1;
		fep->uni.virtualinfo.buffer = &cptbl;
		fep->uni.virtualinfo.offset = 0;
		fep->uni.virtualinfo.size = size_cptbl;
		iob->i_private = fep;

		return 0;
	}
	
	SceIoDeviceEntry *ms0 = getMSEntry();

	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = 0;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;

	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, file);

	int ret = ms0->d_dp->dt_func->df_open(&fake_iob, fake_file, flags, mode);
	if (ret < 0)
	{
		if (sctrlHENIsTestingTool())
			strcpy(fake_file, DC_PATH "/testingtool");
		else
			strcpy(fake_file, DC_PATH "/retail");
		strcat(fake_file, file);
		ret = ms0->d_dp->dt_func->df_open(&fake_iob, fake_file, flags, mode);
	}
	
	FlashEmuPrivate *fep = sceKernelAllocHeapMemory(iob->i_de->d_private, sizeof(FlashEmuPrivate));	
	fep->virtual = 0;
	fep->uni.ms_private = fake_iob.i_private;
		
	iob->i_private = fep;
	
	// Kprintf("Open: %s %x\n", fake_file, ret);

	return ret;
}

static int FlashEmu_df_close(SceIoIob *iob)
{
	// Kprintf("flashemu: FlashEmu_df_Close(%x)\n", iob);
	
	FlashEmuPrivate *fep = iob->i_private;
	if (fep->virtual)
	{
		sceKernelFreeHeapMemory(iob->i_de->d_private, fep);
		return 0;
	}

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = fep->uni.ms_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	int ret = ms0->d_dp->dt_func->df_close(&fake_iob);
	
	iob->i_private = 0;

	sceKernelFreeHeapMemory(iob->i_de->d_private, fep);

	return ret;
}

static int FlashEmu_df_read(SceIoIob *iob, char *data, int len)
{
	// Kprintf("flashemu: FlashEmu_df_Read(%x, %x, %x)\n", iob, data, len);
	
	FlashEmuPrivate *fep = iob->i_private;
	if (fep->virtual)
	{
		memcpy(data, &fep->uni.virtualinfo.buffer[fep->uni.virtualinfo.offset], len);
		fep->uni.virtualinfo.offset += len;
		return len;
	}

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = fep->uni.ms_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	int ret = ms0->d_dp->dt_func->df_read(&fake_iob, data, len);
	
	fep->uni.ms_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_write(SceIoIob *iob, const char *data, int len)
{
	// Kprintf("flashemu: FlashEmu_df_Write(%x, %x, %x)\n", iob, data, len);
	
	FlashEmuPrivate *fep = iob->i_private;
	if (fep->virtual)
	{
		return 0x80010086;
	}

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = fep->uni.ms_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	int ret = 0;
	
	if (data && len)
		ret = ms0->d_dp->dt_func->df_write(&fake_iob, data, len);
	
	fep->uni.ms_private = fake_iob.i_private;

	return ret;
}

static SceOff FlashEmu_df_lseek(SceIoIob *iob, SceOff ofs, int whence)
{
	// Kprintf("flashemu: FlashEmu_df_Lseek(%x, %x, %x)\n", iob, ofs, whence);

	FlashEmuPrivate *fep = iob->i_private;
	if (fep->virtual)
	{
		switch(whence)
		{
			case PSP_SEEK_SET: fep->uni.virtualinfo.offset = ofs; break;
			case PSP_SEEK_CUR: fep->uni.virtualinfo.offset += ofs; break;
			case PSP_SEEK_END: fep->uni.virtualinfo.offset = fep->uni.virtualinfo.size - ofs; break;
		}
		return fep->uni.virtualinfo.offset;
	}

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = fep->uni.ms_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	int ret = ms0->d_dp->dt_func->df_lseek(&fake_iob, ofs, whence);
	
	fep->uni.ms_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_ioctl(SceIoIob *iob, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int res;
	
	// Kprintf("flashemu: Ioctl cmd 0x%08X\n", cmd);

	switch (cmd)
	{
		case 0x00005001: // vsh_module : system.dreg / system.ireg
		case 0x00008003: // FW1.50
		case 0x0000B804: // ????
		case 0x00208003: // FW2.00 load prx  "/vsh/module/opening_plugin.prx"
		case 0x00208006: // load prx
		case 0x00208007: // after start FW2.50
		case 0x00208081: // FW2.00 load prx "/vsh/module/opening_plugin.prx"
			res = 0;
			break;
			
		case 0x00208082: // FW2.80 "/vsh/module/opening_plugin.prx"
			res = 0x80010016;
			break;
			
		case 0x00208013: // ????
			res = 0;
			if(sceKernelGetModel() == 0 || iob->i_unit != 3)
				res = 0x80010016;
			break;

		default:
			WriteFile("fatms0:/unk_ioctl.bin", &cmd, sizeof(unsigned int));
			while(1);
	}

	return res;
}

static int FlashEmu_df_remove(SceIoIob *iob, const char *file)
{
	// Kprintf("flashemu: FlashEmu_df_Remove(%x, %s)\n", iob, file);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, file);
	
	int ret = ms0->d_dp->dt_func->df_remove(&fake_iob, fake_file);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_mkdir(SceIoIob *iob, const char *dir, SceMode mode)
{
	// Kprintf("flashemu: FlashEmu_df_Mkdir(%x, %s, %x)\n", iob, dir, mode);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, dir);
	
	int ret = ms0->d_dp->dt_func->df_mkdir(&fake_iob, fake_file, mode);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_rmdir(SceIoIob *iob, const char *dir)
{
	// Kprintf("flashemu: FlashEmu_df_Rmdir.\n");

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, dir);
	
	int ret = ms0->d_dp->dt_func->df_rmdir(&fake_iob, fake_file);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_dopen(SceIoIob *iob, const char *dir)
{
	// Kprintf("flashemu: FlashEmu_df_Dopen(%x, %s)\n", iob, dir);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_dir[260];
	strcpy(fake_dir, DC_PATH);
	strcat(fake_dir, dir);
	
	int ret = ms0->d_dp->dt_func->df_dopen(&fake_iob, fake_dir);
	if (ret < 0)
	{
		if (sctrlHENIsTestingTool())
			strcpy(fake_dir, DC_PATH "/testingtool");
		else
			strcpy(fake_dir, DC_PATH "/retail");
		strcat(fake_dir, dir);
		ret = ms0->d_dp->dt_func->df_dopen(&fake_iob, fake_dir);
	}
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_dclose(SceIoIob *iob)
{
	// Kprintf("flashemu: FlashEmu_df_Dclose(%x)\n", iob);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	int ret = ms0->d_dp->dt_func->df_dclose(&fake_iob);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_dread(SceIoIob *iob, SceIoDirent *dir)
{
	// Kprintf("flashemu: FlashEmu_df_Dread(%x, %x)\n", iob, dir);

	if (dir->d_private = 0xffffffff)
		dir->d_private = 0;

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;

	int ret = ms0->d_dp->dt_func->df_dread(&fake_iob, dir);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_getstat(SceIoIob *iob, const char *file, SceIoStat *stat)
{
	// Kprintf("flashemu: FlashEmu_df_Getstat(%x, %s, %x)\n", iob, file, stat);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, file);
	
	int ret = ms0->d_dp->dt_func->df_getstat(&fake_iob, fake_file, stat);
	if (ret < 0)
	{
		if (sctrlHENIsTestingTool())
			strcpy(fake_file, DC_PATH "/testingtool");
		else
			strcpy(fake_file, DC_PATH "/retail");
		strcat(fake_file, file);
		ret = ms0->d_dp->dt_func->df_getstat(&fake_iob, fake_file, stat);
	}
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_chstat(SceIoIob *iob, const char *file, SceIoStat *stat, int bits)
{
	// Kprintf("flashemu: FlashEmu_df_Chstat(%x, %s, %x, %x)\n", iob, file, stat, bits);

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_file[260];
	strcpy(fake_file, DC_PATH);
	strcat(fake_file, file);
	
	int ret = ms0->d_dp->dt_func->df_chstat(&fake_iob, fake_file, stat, bits);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_rename(SceIoIob *iob, const char *oldname, const char *newname)
{
	// Kprintf("flashemu: FlashEmu_df_Rename.\n");

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_oldname[260];
	strcpy(fake_oldname, DC_PATH);
	strcat(fake_oldname, oldname);
	
	char fake_newname[260];
	strcpy(fake_newname, DC_PATH);
	strcat(fake_newname, newname);
	
	int ret = ms0->d_dp->dt_func->df_rename(&fake_iob, fake_oldname, fake_newname);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_chdir(SceIoIob *iob, const char *dir)
{
	// Kprintf("flashemu: FlashEmu_df_Chdir.\n");

	SceIoDeviceEntry *ms0 = getMSEntry();
	
	SceIoIob fake_iob;
	fake_iob.i_flgs = iob->i_flgs;
	fake_iob.i_unit = 0;
	fake_iob.i_de = ms0;
	fake_iob.d_type = iob->d_type;
	fake_iob.i_private = iob->i_private;
	fake_iob.i_cwd = iob->i_cwd;
	fake_iob.i_fpos = iob->i_fpos;
	fake_iob.i_thread = iob->i_thread;
	fake_iob.dummy = iob->dummy;
	
	char fake_dir[260];
	strcpy(fake_dir, DC_PATH);
	strcat(fake_dir, dir);
	
	int ret = ms0->d_dp->dt_func->df_chdir(&fake_iob, fake_dir);
	
	iob->i_private = fake_iob.i_private;

	return ret;
}

static int FlashEmu_df_mount(SceIoIob *iob)
{
	// Kprintf("flashemu: IoMount\n");
	return 0;
}

static int FlashEmu_df_umount(SceIoIob *iob)
{
	// Kprintf("flashemu: IoUmount, i_unit %d\n", iob->i_unit);
	return 0;
}

static int FlashEmu_df_devctl(SceIoIob *iob, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	// Kprintf("flashemu: IoDevctl(%x, %s, %x, %x, %x, %x, %x)\n", iob, devname, cmd, indata, inlen, outdata, outlen);

	int res = 0;

	switch (cmd)
	{
		case 0x00005802:
			res = 0;
			break;
			
		case 0x00208813:
			res = 0;
			if (sceKernelGetModel() == 0 || iob->i_unit != 3)
				return 0x80010016;

		default:
			WriteFile("fatms0:/unk_devctl.bin", &cmd, sizeof(unsigned int));
			while (1);
	}

	return res;
}

int FlashEmu_df_cancel(SceIoIob *iob)
{
	// Kprintf("flashemu: FlashEmu_df_Cancel\n");
	return 0x80010086;
}

static SceIoDeviceFunction flashemu_funcs =
{
	FlashEmu_df_init,
	FlashEmu_df_exit,
	FlashEmu_df_open,
	FlashEmu_df_close,
	FlashEmu_df_read,
	FlashEmu_df_write,
	FlashEmu_df_lseek,
	FlashEmu_df_ioctl,
	FlashEmu_df_remove,
	FlashEmu_df_mkdir,
	FlashEmu_df_rmdir,
	FlashEmu_df_dopen,
	FlashEmu_df_dclose,
	FlashEmu_df_dread,
	FlashEmu_df_getstat,
	FlashEmu_df_chstat,
	FlashEmu_df_rename,
	FlashEmu_df_chdir,
	FlashEmu_df_mount,
	FlashEmu_df_umount,
	FlashEmu_df_devctl,
	FlashEmu_df_cancel
};

static SceIoDeviceTable flashemu =
{
	"flashfat",
	0x1E0010,
	1,
	"FAT over Flash",
	&flashemu_funcs
};

int Fake_Success()
{
	return 0;
}

int Fake_Error()
{
	return 0x80010086;
}

static SceIoDeviceFunction fake_funcs =
{
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Success,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Error,
	(void *) Fake_Success,
	(void *) Fake_Success
};

static SceIoDeviceTable fake =
{
	"lflash",
	4,
	0x200,
	0,
	&fake_funcs
};

int SceLfatfsAssign()
{
	// Kprintf("SceLfatfsAssign()\n");

	getMSEntry();
	sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, NULL, 0);
	sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH)
		sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);

	if (sceKernelGetModel() != 0)
	{
		int ikc = sceKernelInitKeyConfig();
		if (ikc == PSP_INIT_KEYCONFIG_VSH || (ikc == PSP_INIT_KEYCONFIG_APP && sceKernelBootFrom() == PSP_BOOT_FLASH3))
			sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, NULL, 0);
	}

	assign_thread = -1;
	sceKernelExitDeleteThread(0);
	return 0;
}

int InstallFlashEmu()
{
	// Kprintf("InstallFlashEmu()\n");

	sceIoDelDrv("lflash");
	sceIoAddDrv(&fake);
	sceIoDelDrv("flashfat");
	sceIoAddDrv(&flashemu);
	assign_thread = sceKernelCreateThread("SceLfatfsAssign", SceLfatfsAssign, 100, 0x1000, 0x100000, NULL);
	sceKernelStartThread(assign_thread, 0, NULL);
	
	return 0;
}

int UninstallFlashEmu()
{
	// Kprintf("UninstallFlashEmu()\n");

	return 0;
}

void PreareRebootFlashEmu()
{
	sceIoUnassign("flash0:");
	sceIoUnassign("flash1:");
	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH)
		sceIoUnassign("flash2:");
	if (sceKernelGetModel() != 0)
	{
		int ikc = sceKernelInitKeyConfig();
		if (ikc == PSP_INIT_KEYCONFIG_VSH || (ikc == PSP_INIT_KEYCONFIG_APP && sceKernelBootFrom() == PSP_BOOT_FLASH3))
			sceIoUnassign("flash3:");
	}
}

int sceLfatfsWaitReady()
{
	// Kprintf("sceLfatfsWaitReady()\n");

	if (assign_thread < 0)
		return 0;

	return sceKernelWaitThreadEnd(assign_thread, 0);
}

int sceLfatfsStop()
{
	// Kprintf("sceLfatfsStop()\n");
	return 0;
}

int sceLFatFs_driver_51c7f7ae()
{
	// Kprintf("sceLFatFs_driver_51c7f7ae()\n");
	return 0x6060110;
}

int sceLFatFs_driver_f1fba85f()
{
	// Kprintf("sceLFatFs_driver_f1fba85f()\n");
	return 0;
}

int sceLFatFs_driver_bed8d616()
{
	// Kprintf("sceLFatFs_driver_bed8d616()\n");
	return 0;
}

int dword_880F78C = 0;

