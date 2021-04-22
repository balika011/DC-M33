#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspinit.h>
#include <pspsysmem_kernel.h>
#include <pspsysevent.h>

#include <stdio.h>
#include <string.h>

#include "flashemu.h"

char path[260];
SceUID flashemu_sema = -1;
SceUID assign_thread = -1;
char need_wait = 1;

struct emu_file
{
	char allocated;
	SceUID fd;
	char closed;
	char path[192];
	SceMode mode;
	int flags;
	SceOff offset;
} emu_files[32];

#define Lock()		sceKernelWaitSema(flashemu_sema, 1, NULL)
#define UnLock()	sceKernelSignalSema(flashemu_sema, 1)

void WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	sceIoWrite(fd, buf, size);
	sceIoClose(fd);
}

void BuildPath(const char *file)
{
	strcpy(path, "ms0:/TM/DC8");
	strcat(path, file);
}

void WaitMS()
{
	if (need_wait)
	{
		while (1)
		{
			SceUID fd = sceIoDopen("ms0:/TM/DC8/");
			if (fd >= 0)
			{
				sceIoDclose(fd);
				need_wait = 0;
				break;
			}

			sceKernelDelayThread(20000);
		}
	}
}

int SlotGetFree()
{
	for (int i = 0; i < sizeof(emu_files) / sizeof(emu_files[0]); i++)
	{
		if (!emu_files[i].allocated)
			return i;
	}
	
	return -1;
}

int SlotOpen(int slot, char *path, int flags, SceMode mode)
{
	WaitMS();
	
	SceUID fd;
	while (1)
	{
		if (flags == 0xD0D0)
			fd = sceIoDopen(path);
		else
			fd = sceIoOpen(path, flags, mode);
			
		if (fd != 0x80010018)
			break;
		
		for (int i = 0; i < sizeof(emu_files) / sizeof(emu_files[0]); i++)
		{
			if(emu_files[i].allocated && !emu_files[i].closed && emu_files[i].flags == PSP_O_RDONLY)
			{
				SceUID sfd = emu_files[i].fd;
				emu_files[i].offset = sceIoLseek(sfd, 0, PSP_SEEK_CUR);
				sceIoClose(sfd);
				emu_files[i].closed = 1;
				break;
			}
		}
	}
	
	if (fd > 0)
	{
		emu_files[slot].closed = 0;
		emu_files[slot].flags = flags;
		emu_files[slot].mode = mode;
		emu_files[slot].allocated = 1;
		emu_files[slot].fd = fd;
		if(emu_files[slot].path != path)
			strncpy(emu_files[slot].path, path, sizeof(emu_files[slot].path));
	}
	
	return fd;
}

SceUID SlotGetFd(int slot)
{
	if (emu_files[slot].allocated)
	{
		if (emu_files[slot].closed)
		{
			SceUID fd = SlotOpen(slot, emu_files[slot].path, emu_files[slot].flags, emu_files[slot].mode);
			if (fd < 0)
				return fd;

			sceIoLseek(fd, emu_files[slot].offset, PSP_SEEK_SET);
			emu_files[slot].fd = fd;
			emu_files[slot].closed = 0;
		}
		return emu_files[slot].fd;
	}
	
	return -1;
}

void SlotClear(int slot)
{
	emu_files[slot].allocated = 0;
}

int SlotCloseOneReadOnly()
{
	Lock();
	for (int i = 0; i < sizeof(emu_files) / sizeof(emu_files[0]); i++)
	{
		if (emu_files[i].allocated && !emu_files[i].closed && emu_files[i].flags == PSP_O_RDONLY)
		{
			SceUID fd = emu_files[i].fd;
			emu_files[i].offset = sceIoLseek(fd, 0, PSP_SEEK_CUR);
			emu_files[i].closed = 1;
			sceIoClose(fd);
			UnLock();
			return 0;
		}
	}
	
	UnLock();
	return 0x80010018;
}

static int FlashEmu_IoInit(PspIoDrvArg* arg)
{
	//Kprintf("IoInit.\n");
	flashemu_sema = sceKernelCreateSema("FlashSema", 0, 1, 1, NULL);
	memset(emu_files, 0, sizeof(emu_files));
	
	return 0;
}

static int FlashEmu_IoExit(PspIoDrvArg* arg)
{
	//Kprintf("IoExit.\n");

	return 0;
}

static int open_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	const char *file = (const char *)argv[1];
	int flags = argv[2];
	SceMode mode = (SceMode)argv[3];

	Lock();
	BuildPath(file);
	
	int slot = SlotGetFree();
	if (slot < 0)
	{
		UnLock();
		return 0x80010018;
	}

	int ret = SlotOpen(slot, path, flags, mode);
	if (ret < 0)
	{
		//Kprintf("Error 0x%08X in IoOpen, file %s\n", fd, path);
		UnLock();
		return ret;
	}

	
	arg->arg = (void *) slot;
	UnLock();

	return 0;
}

static int FlashEmu_IoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	int argv[4];

	argv[0] = (int)arg;
	argv[1] = (int)file;
	argv[2] = (int)flags;
	argv[3] = (int)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)open_main, (void *)argv);	
}

static int close_main(PspIoDrvFileArg *arg)
{
	int slot = (int)arg->arg;
	SceUID fd = SlotGetFd(slot);
	if (fd < 0)
		return fd;

	int res = sceIoClose(fd);
	if (res < 0)
		return res;

	SlotClear(slot);

	return 0;
}

static int FlashEmu_IoClose(PspIoDrvFileArg *arg)
{
	Lock();
	int res = sceKernelExtendKernelStack(0x4000, (void *)close_main, (void *)arg);
	UnLock();

	return res;
}

static int read_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	char *data = (char *)argv[1];
	int len = argv[2];

	Lock();
	
	SceUID fd = SlotGetFd((int)arg->arg);
	if (fd < 0)
	{
		//Kprintf("Error 0x%08X in IoRead\n", fd);
		UnLock();

		return fd;
	}
	
	int res = sceIoRead(fd, data, len);
	
	UnLock();

	return res;
}

static int FlashEmu_IoRead(PspIoDrvFileArg *arg, char *data, int len)
{
	int argv[3];

	argv[0] = (int)arg;
	argv[1] = (int)data;
	argv[2] = (int)len;
	
	return sceKernelExtendKernelStack(0x4000, (void *)read_main, (void *)argv);
}

static int write_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	const char *data = (const char *)argv[1];
	int len = argv[2];

	Lock();
	
	SceUID fd = SlotGetFd((int) arg->arg);
	if (fd < 0)
	{
		//Kprintf("Error 0x%08X in IoWrite\n", fd);
		UnLock();
		return fd;
	}
	
	int res = 0;
	if (data || len)
		res = sceIoWrite(fd, data, len);

	UnLock();

	return res;
}

static int FlashEmu_IoWrite(PspIoDrvFileArg *arg, const char *data, int len)
{
	int argv[3];

	argv[0] = (int)arg;
	argv[1] = (int)data;
	argv[2] = (int)len;
	
	return sceKernelExtendKernelStack(0x4000, (void *)write_main, (void *)argv);	
}

static int lseek_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	u32 ofs = (u32)argv[1];
	int whence = argv[2];

	Lock();
	
	SceUID fd = SlotGetFd((int)arg->arg);
	if (fd < 0)
	{
		//Kprintf("Error 0x%08X in IoLseek\n", fd);
		UnLock();
		return fd;
	}
	
	int res = sceIoLseek(fd, ofs, whence);
	
	UnLock();

	return res;
}

static SceOff FlashEmu_IoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence)
{
	int argv[3];
	
	argv[0] = (int)arg;
	argv[1] = (int)ofs;
	argv[2] = (int)whence;

	return (SceOff)sceKernelExtendKernelStack(0x4000, (void *)lseek_main, (void *)argv);
}

static int FlashEmu_IoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int res;
	
	//Kprintf("Ioctl cmd 0x%08X\n", cmd);
	
	Lock();

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
			if(arg->fs_num != 3 || sceKernelGetModel() == 0)
				res = 0x80010016;
			break;

		default:
			WriteFile("fatms0:/unk_ioctl.bin", &cmd, sizeof(unsigned int));
			while(1);
	}

	UnLock();
	return res;
}

static int remove_main(int *argv)
{
	const char *name = (const char *)argv[1];

	Lock();
	BuildPath(name);

	WaitMS();
	int res = sceIoRemove(path);
	
	UnLock();
	return res;
}

static int FlashEmu_IoRemove(PspIoDrvFileArg *arg, const char *name)
{
	int argv[2];

	argv[0] = (int)arg;
	argv[1] = (int)name;
	
	return sceKernelExtendKernelStack(0x4000, (void *)remove_main, (void *)argv);	
}

static int mkdir_main(int *argv)
{
	const char *name = (const char *)argv[1];
	SceMode mode = (SceMode)argv[2];

	Lock();
	BuildPath(name);

	WaitMS();
	int res = sceIoMkdir(path, mode);

	UnLock();
	return res;
}

static int FlashEmu_IoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode)
{
	int argv[3];

	argv[0] = (int)arg;
	argv[1] = (int)name;
	argv[2] = mode;

	return sceKernelExtendKernelStack(0x4000, (void *)mkdir_main, (void *)argv);
}

static int rmdir_main(int *argv)
{
	const char *name = (const char *)argv[1];
	
	Lock();
	BuildPath(name);

	WaitMS();
	int res = sceIoRmdir(path);

	UnLock();
	return res;
}

static int FlashEmu_IoRmdir(PspIoDrvFileArg *arg, const char *name)
{
	int argv[2];

	argv[0] = (int)arg;
	argv[1] = (int)name;	

	return sceKernelExtendKernelStack(0x4000, (void *)rmdir_main, (void *)argv);
}

static int dopen_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	const char *dirname = (const char *)argv[1];
	
	Lock();
	BuildPath(dirname);
	
	int slot = SlotGetFree();
	if (slot < 0)
	{
		UnLock();
		return 0x80010018;
	}

	int ret = SlotOpen(slot, path, 0xD0D0, 0);
	if (ret < 0)
	{
		//Kprintf("Error 0x%08X in IoOpen, file %s\n", fd, patemu_files[slot].fdh);
		UnLock();
		return ret;
	}

	
	arg->arg = (void *) slot;
	UnLock();
	return 0;
}

static int FlashEmu_IoDopen(PspIoDrvFileArg *arg, const char *dirname)
{
	int argv[2];

	argv[0] = (int)arg;
	argv[1] = (int)dirname;	

	return sceKernelExtendKernelStack(0x4000, (void *)dopen_main, (void *)argv);
}

static int dclose_main(PspIoDrvFileArg *arg)
{
	int slot = (int) arg->arg;

	Lock();
	
	SceUID fd = SlotGetFd(slot);
	if (fd < 0)
	{
		//Kprintf("Error 0x%08X in IoDclose, file %s\n", fd, path);
		UnLock();
		return fd;
	}
	
	int res = sceIoDclose(fd);
	SlotClear(slot);

	UnLock();
	return res;
}

static int FlashEmu_IoDclose(PspIoDrvFileArg *arg)
{
	return sceKernelExtendKernelStack(0x4000, (void *)dclose_main, (void *)arg);
}

static int dread_main(int *argv)
{
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)argv[0];
	SceIoDirent *dir = (SceIoDirent *)argv[1];
	
	Lock();
	
	SceUID fd = SlotGetFd((int) arg->arg);
	if (fd < 0)
	{
		//Kprintf("Error 0x%08X in IoDread, file %s\n", fd, path);
		UnLock();
		return fd;
	}
	
	int res = sceIoDread(fd, dir);
	
	UnLock();

	return res;
}

static int FlashEmu_IoDread(PspIoDrvFileArg *arg, SceIoDirent *dir)
{
	int argv[2];

	argv[0] = (int)arg;
	argv[1] = (int)dir;
	
	return sceKernelExtendKernelStack(0x4000, (void *)dread_main, (void *)argv);
}

static int getstat_main(int *argv)
{
	const char *file = (const char *)argv[1];
	SceIoStat *stat = (SceIoStat *)argv[2];
	
	Lock();
	BuildPath(file);

	WaitMS();
	int res = sceIoGetstat(path, stat);

	UnLock();
	return res;
}

static int FlashEmu_IoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat)
{
	int argv[3];

	argv[0] = (int)arg;
	argv[1] = (int)file;
	argv[2] = (int)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)getstat_main, (void *)argv);
}

static int chstat_main(int *argv)
{
	const char *file = (const char *)argv[1];
	SceIoStat *stat = (SceIoStat *)argv[2];
	int bits = (int)argv[3];
	
	Lock();
	BuildPath(file);

	WaitMS();
	int res = sceIoChstat(path, stat, bits);

	UnLock();
	return res;
}

static int FlashEmu_IoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits)
{
	int argv[4];

	argv[0] = (int)arg;
	argv[1] = (int)file;
	argv[2] = (int)stat;
	argv[3] = (int)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)chstat_main, (void *)argv);
}

static int rename_main(int *argv)
{
	const char *oldname = (const char *)argv[1];
	const char *newname = (const char *)argv[2];	
	
	Lock();
	BuildPath(oldname);
	
	WaitMS();
	int res = sceIoRename(oldname, newname);

	UnLock();
	return res;
}

static int FlashEmu_IoRename(PspIoDrvFileArg *arg, const char *oldname, const char *newname)
{
	int argv[3];

	argv[0] = (int)arg;
	argv[1] = (int)oldname;
	argv[2] = (int)newname;

	return sceKernelExtendKernelStack(0x4000, (void *)rename_main, (void *)argv);
}

static int chdir_main(int *argv)
{
	const char *dir = (const char *)argv[1];
	
	Lock();
	BuildPath(dir);

	WaitMS();
	int res = sceIoChdir(path);

	UnLock();
	return res;
}

static int FlashEmu_IoChdir(PspIoDrvFileArg *arg, const char *dir)
{
	int argv[2];

	argv[0] = (int)arg;
	argv[1] = (int)dir;	

	return sceKernelExtendKernelStack(0x4000, (void *)chdir_main, (void *)argv);
}

static int FlashEmu_IoMount(PspIoDrvFileArg *arg)
{
	//Kprintf("IoMount: %s %s %08X %08X %08X \n", asgn_name, dev_name, wr_mode, unk, unk2);
	return 0;
}

static int FlashEmu_IoUmount(PspIoDrvFileArg *arg)
{
	//Kprintf("IoUmount, fs_num %d\n", arg->fs_num);
	return 0;
}

static int FlashEmu_IoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int res = 0;

	switch (cmd)
	{
		case 0x00005802:
			res = 0;
			break;
			
		case 0x00208813:
			res = 0;
			if (sceKernelGetModel() == 0 || arg->fs_num != 3)
				return 0x80010016;

		default:
			WriteFile("fatms0:/unk_devctl.bin", &cmd, sizeof(unsigned int));
			while (1);
	}

	return res;
}

int FlashEmu_IoUnk21(PspIoDrvFileArg *arg)
{
	return 0x80010086;
}

static PspIoDrvFuncs flashemu_funcs =
{
	FlashEmu_IoInit,
	FlashEmu_IoExit,
	FlashEmu_IoOpen,
	FlashEmu_IoClose,
	FlashEmu_IoRead,
	FlashEmu_IoWrite,
	FlashEmu_IoLseek,
	FlashEmu_IoIoctl,
	FlashEmu_IoRemove,
	FlashEmu_IoMkdir,
	FlashEmu_IoRmdir,
	FlashEmu_IoDopen,
	FlashEmu_IoDclose,
	FlashEmu_IoDread,
	FlashEmu_IoGetstat,
	FlashEmu_IoChstat,
	FlashEmu_IoRename,
	FlashEmu_IoChdir,
	FlashEmu_IoMount,
	FlashEmu_IoUmount,
	FlashEmu_IoDevctl,
	FlashEmu_IoUnk21
};

static PspIoDrv flashemu =
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

static PspIoDrvFuncs fake_funcs =
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

static PspIoDrv fake =
{
	"lflash",
	4,
	0x200,
	0,
	&fake_funcs
};

int SceLfatfsAssign()
{
	WaitMS();
	sceIoAssign("flash0:", "lflash0:0,0", "flashfat0:", IOASSIGN_RDONLY, NULL, 0);
	sceIoAssign("flash1:", "lflash0:0,1", "flashfat1:", IOASSIGN_RDWR, NULL, 0);

	if (sceKernelInitKeyConfig() == 0x100)
		sceIoAssign("flash2:", "lflash0:0,2", "flashfat2:", IOASSIGN_RDWR, NULL, 0);

	if (sceKernelGetModel() == 1)
	{
		int ikc = sceKernelInitKeyConfig();
		if (ikc == 0x100 || (ikc == 0x400 && sceKernelBootFrom() == 0x80))
			sceIoAssign("flash3:", "lflash0:0,3", "flashfat3:", IOASSIGN_RDWR, NULL, 0);
	}

	assign_thread = -1;
	sceKernelExitDeleteThread(0);
	return 0;
}

int SysEventHandlerFunc(int ev_id, char *ev_name, void *param, int *result)
{
	switch (ev_id)
	{
	case 0x4000:
		for (int i = 0; i < sizeof(emu_files) / sizeof(emu_files[0]); i++)
		{
			if (emu_files[i].allocated && !emu_files[i].closed && emu_files[i].flags != 0xD0D0)
			{
				SceUID fd = emu_files[i].fd;
				emu_files[i].offset = sceIoLseek(fd, 0, PSP_SEEK_CUR);
				emu_files[i].closed = 1;
				sceIoClose(fd);
			}
		}
		break;
	case 0x10009:
		need_wait = 1;
		break;
	}
	
	return 0;

}

static PspSysEventHandler sys_event_handler =
{
	sizeof(PspSysEventHandler),
	"",
	0xFFFF00,
	SysEventHandlerFunc,
};

int InstallFlashEmu()
{
	sceIoDelDrv("lflash");
	sceIoAddDrv(&fake);
	sceIoDelDrv("flashfat");
	sceIoAddDrv(&flashemu);
	sceKernelRegisterSysEventHandler(&sys_event_handler);
	assign_thread = sceKernelCreateThread("SceLfatfsAssign", SceLfatfsAssign, 100, 0x1000, 0x100000, NULL);
	sceKernelStartThread(assign_thread, 0, NULL);
	
	return 0;
}

int UninstallFlashEmu()
{
	return 0;
}

void PreareRebootFlashEmu()
{
	SceUInt timeout = 500000;
	sceKernelWaitSema(flashemu_sema, 1, &timeout);
	sceKernelDeleteSema(flashemu_sema);
	sceIoUnassign("flash0:");
	sceIoUnassign("flash1:");
	sceKernelUnregisterSysEventHandler(&sys_event_handler);
}

int sceLfatfsWaitReady()
{
	if (assign_thread < 0)
		return 0;

	return sceKernelWaitThreadEnd(assign_thread, 0);
}

int sceLfatfsStop()
{
	return 0;
}

int sceLFatFs_driver_51c7f7ae()
{
	return 0x5000010;
}

int sceLFatFs_driver_f1fba85f()
{
	return 0;
}

