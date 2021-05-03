#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>
#include <psploadexec_kernel.h>
#include <pspreg.h>
#include <pspctrl.h>
#include <psprtc.h>
#include <pspusb.h>
#include <pspusbstor.h>
#include <psppower.h>
#include <umd9660_driver.h>
#include <isofs_driver.h>
#include <virtualpbpmgr.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <oe_malloc.h>
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PSP_MODULE_INFO("VshControl", 0x1007, 1, 3);
const int module_sdk_version = 0x03060010;

#define EBOOT_BIN	"disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define BOOT_BIN	"disc0:/PSP_GAME/SYSDIR/BOOT.BIN"
#define PROGRAM	"ms0:/PSP/GAME/BOOT/EBOOT.PBP"

SEConfig config;
int (* vshmenu_ctrl)(SceCtrlData *pad_data, int count);
static SceUID satelite = -1, usbdev = -1;
static SceKernelLMOption lmoption;
u8 videoiso_mounted, set, return_iso;
int firsttick, ticklast;
SceUID gamedfd = -1, game150dfd = -1, game5xxdfd = -1, isodfd = -1, over5xx = 0, overiso = 0, paramsfo = -1;
int vpbpinited = 0, isoindex = 0, cachechanged = 0;
VirtualPbp vpbp;
VirtualPbp *cache = NULL;
u8 referenced[32];

void ClearCaches()
{
	sceKernelDcacheWritebackAll();
	sceKernelIcacheClearAll();
}

int LoadReboot(char *file)
{
	SceUID mod = sceKernelLoadModule("flash0:/kd/reboot150.prx", 0, NULL);

	if (mod < 0)
	{
		mod = sceKernelLoadModule("ms0:/seplugins/reboot150.prx", 0, NULL);
	}

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, 0, NULL, NULL, NULL);
}

void KXploitString(char *str)
{
	if (str)
	{
		char *perc = strchr(str, '%');

		if (perc)
		{
			strcpy(perc, perc+1);
		}
	}
}

void Fix150Path(const char *file)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file)
	{
		strcpy(str, (char *)file);

		char *p = strstr(str, "__150");
		
		if (p)
		{
			strcpy((char *)file+13, "150/");
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);		
		}
	}
}

void Fix5XXPath(const char *file)
{
	char str[256];

	if (strstr(file, "ms0:/PSP/GAME/") == file)
	{
		strcpy(str, (char *)file);

		char *p = strstr(str, "__5XX");
		
		if (p)
		{
			strcpy((char *)file+13, "5XX/");
			strncpy((char *)file+17, str+14, p-(str+14));
			strcpy((char *)file+17+(p-(str+14)), p+5);		
		}
	}
}

int GetIsoIndex(const char *file)
{
	char number[5];

	if (strstr(file, "ms0:/PSP/GAME/MMMMMISO") != file)
		return -1;

	char *p = strchr(file+17, '/');

	if (!p)
		return strtol(file+22, NULL, 10);
	
	memset(number, 0, 5);
	strncpy(number, file+22, p-(file+22));

	return strtol(number, NULL, 10);	
}

void ReturnToDisc()
{
	sctrlSEUmountUmd();
	sceKernelCallSubIntrHandler(0x04,0x1a,0,0);
	videoiso_mounted = 0;
}

#if 0
char update_url[] = "http://updates.dark-alex.org/updatelist.txt";

int up_url_offsets[16] = 
{ 
	0x0000ABD0, 0x0000AC14, 0x0000AC58, 0x0000AC9C,
	0x0000ACE4, 0x0000AD2C, 0x0000AD74, 0x0000ADBC,
	0x0000AE04, 0x0000AE4C, 0x0000AE94, 0x0000AEDC,
	0x0000AF24, 0x0000AF6C, 0x0000AFB4, 0x0000AFFC
};
     
	for(i = 0; i < 16; i++)
	{
		strcpy((char*)(text_addr+up_url_offsets[i]), update_url);
	}

#endif

int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2)
{
	int k1 = pspSdkSetK1(0);
	int reboot150 = 0;

	SetUmdFile("");

	int index = GetIsoIndex(file);

	if (index >= 0)
	{
		if (config.executebootbin)
		{
			strcpy(file, BOOT_BIN);			
			param->args = strlen(BOOT_BIN)+1;	
		}
		else
		{		
			strcpy(file, EBOOT_BIN);			
			param->args = strlen(EBOOT_BIN)+1;			
		}

		param->argp = file;
		SetUmdFile(virtualpbp_getfilename(index));

		if (config.umdmode == MODE_MARCH33)
		{
			sctrlSESetBootConfFileIndex(1);			
		}
		else if (config.umdmode == MODE_NP9660)
		{
			sctrlSESetBootConfFileIndex(2);
		}

		pspSdkSetK1(k1);
		return sctrlKernelLoadExecVSHWithApitype(0x120, file, param);	
	}

	Fix150Path(file);
	Fix150Path(param->argp);
	Fix5XXPath(file);
	Fix5XXPath(param->argp);	

	if (strstr(file, "ms0:/PSP/GAME150/"))
	{
		reboot150 = 1;
		KXploitString(file);
		KXploitString(param->argp);		
	}

	else if (strstr(file, "ms0:/PSP/GAME/") == file && !strstr(file, "ms0:/PSP/GAME/UPDATE/"))
	{
		if (config.gamekernel150)
		{
			reboot150 = 1;
			KXploitString(file);
			KXploitString(param->argp);	
		}
	}	

	else if (strstr(file, "EBOOT.BIN"))
	{
		if (config.executebootbin)
		{
			strcpy(file, BOOT_BIN);	
			param->argp = file;
		}
	}

	param->args = strlen(param->argp) + 1; // update length

	if (reboot150 == 1)
	{
		LoadReboot(file);		
	}

	pspSdkSetK1(k1);

	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

int sceRtcGetCurrentClockLocalTimePatched(pspTime *time)
{
	int res = sceRtcGetCurrentClockLocalTime(time);
	int k1 = pspSdkSetK1(0);

	if (time->day == 2 && time->month == 4)
	{
		time->day = 33;
		time->month = 3;
	}

	pspSdkSetK1(k1);
	return res;
}

SceUID sceIoDopenPatched(const char *dirname)
{
	int res, g150 = 0, index;
	int k1 = pspSdkSetK1(0);

	Fix150Path(dirname);
	Fix5XXPath(dirname);	

	index = GetIsoIndex(dirname);
	if (index >= 0)
	{
		int res = virtualpbp_open(index);
		
		pspSdkSetK1(k1);
		return res;
	}

	if (strcmp(dirname, "ms0:/PSP/GAME") == 0)
	{
		g150 = 1;		
	}

	pspSdkSetK1(k1);
	res = sceIoDopen(dirname);
	pspSdkSetK1(0);

	if (g150)
	{
		gamedfd = res;
		game150dfd = sceIoDopen("ms0:/PSP/GAME150");
		over5xx = 0;
		overiso = 0;				
	}	

	pspSdkSetK1(k1);
	return res;
}

int CorruptIconPatch(char *name, int g150)
{
	char path[256];
	SceIoStat stat;

	if (g150)
	{
		sprintf(path, "ms0:/PSP/GAME150/%s%%/EBOOT.PBP", name);
	}

	else
	{
		sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);
	}

	memset(&stat, 0, sizeof(stat));
	
	if (sceIoGetstat(path, &stat) >= 0)
	{
		strcpy(name, "__SCE"); // hide icon
		
		return 1;
	}
	
	return 0;
}

void ApplyNamePatch(SceIoDirent *dir, char *patch)
{
	if (dir->d_name[0] != '.')
	{
		int patchname = 1;

		if (config.hidecorrupt)
		{
			if (CorruptIconPatch(dir->d_name, 1))
				patchname = 0;
		}

		if (patchname)
		{
			strcat(dir->d_name, patch);
		}
	}
}

void ApplyIsoNamePatch(SceIoDirent *dir)
{
	if (dir->d_name[0] != '.')
	{
		memset(dir->d_name, 0, 256);
		sprintf(dir->d_name, "MMMMMISO%d", isoindex++);
	}
}

int ReadCache()
{
	SceUID fd;
	int i;

	if (!cache)
	{
		cache = (VirtualPbp *)oe_malloc(32*sizeof(VirtualPbp));		
	}

	memset(cache, 0, sizeof(VirtualPbp)*32);
	memset(referenced, 0, sizeof(referenced));

	for (i = 0; i < 0x10; i++)
	{
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_RDONLY, 0);

		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoRead(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);

	return 0;
}

int SaveCache()
{
	SceUID fd;
	int i;

	if (!cache)
		return -1;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0 && !referenced[i])
		{
			cachechanged = 1;
			memset(&cache[i], 0, sizeof(VirtualPbp));			
		}
	}

	if (!cachechanged)
		return 0;

	cachechanged = 0;

	for (i = 0; i < 0x10; i++)
	{
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

		if (fd >= 0)
			break;
	}

	if (i == 0x10)
		return -1;

	sceIoWrite(fd, cache, sizeof(VirtualPbp)*32);
	sceIoClose(fd);

	return 0;
}

int IsCached(char *isofile, ScePspDateTime *mtime, VirtualPbp *res)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] != 0)
		{
			if (strcmp(cache[i].isofile, isofile) == 0)
			{
				if (memcmp(mtime, &cache[i].mtime, sizeof(ScePspDateTime)) == 0)
				{
					memcpy(res, &cache[i], sizeof(VirtualPbp));
					referenced[i] = 1;
					return 1;
				}
			}
		}
	}

	return 0;
}

int Cache(VirtualPbp *pbp)
{
	int i;

	for (i = 0; i < 32; i++)
	{
		if (cache[i].isofile[0] == 0)
		{
			referenced[i] = 1;
			memcpy(&cache[i], pbp, sizeof(VirtualPbp));
			cachechanged = 1;
			return 1;
		}
	}

	return 0;
}

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir)
{	
	int res;
	u32 k1 = pspSdkSetK1(0);

	if (vpbpinited)
	{
		res = virtualpbp_dread(fd, dir);		
		if (res >= 0)
		{
			pspSdkSetK1(k1);
			return res;
		}
	}
	
	if (fd >= 0)
	{
		if (fd == gamedfd)
		{
			if (game150dfd >= 0)
			{
				if ((res = sceIoDread(game150dfd, dir)) > 0)
				{
					ApplyNamePatch(dir, "__150");					
					pspSdkSetK1(k1);
					return res;
				}
				else
				{
					sceIoDclose(game150dfd);
					game150dfd = -1;					
				}
			}

			if (game150dfd < 0 && game5xxdfd < 0 && isodfd < 0 && !over5xx)
			{
				game5xxdfd = sceIoDopen("ms0:/PSP/GAME5XX");
				if (game5xxdfd < 0)
				{
					over5xx = 1;
				}
			}
			
			if (game5xxdfd >= 0)
			{
				if ((res = sceIoDread(game5xxdfd, dir)) > 0)
				{
					ApplyNamePatch(dir, "__5XX");					
					pspSdkSetK1(k1);
					return res;
				}
				else
				{
					sceIoDclose(game5xxdfd);
					game5xxdfd = -1;
					over5xx = 1;
				}
			}

			if (game150dfd < 0 && game5xxdfd < 0 && isodfd < 0 && !overiso)
			{
				isodfd = sceIoDopen("ms0:/ISO");
				
				if (isodfd >= 0)
				{
					if (!vpbpinited)
					{
						virtualpbp_init();
						vpbpinited = 1;							
					}
					else
					{
						virtualpbp_reset();						
					}

					ReadCache();
					isoindex = 0;
				}
				else
				{
					overiso = 1;
				}
			}

			if (isodfd >= 0)
			{
NEXT:
				if ((res = sceIoDread(isodfd, dir)) > 0)
				{
					char fullpath[128];
					int  res2 = -1;
					int  docache;

					if (!FIO_S_ISDIR(dir->d_stat.st_mode))
					{
						strcpy(fullpath, "ms0:/ISO/");
						strcat(fullpath, dir->d_name);

						if (IsCached(fullpath, &dir->d_stat.st_mtime, &vpbp))
						{
							res2 = virtualpbp_fastadd(&vpbp);
							docache = 0;
						}
						else
						{
							res2 = virtualpbp_add(fullpath, &dir->d_stat.st_mtime, &vpbp);
							docache = 1;							
						}
						
						if (res2 >= 0)
						{
							ApplyIsoNamePatch(dir);

							// Fake the entry from file to directory
							dir->d_stat.st_mode = 0x11FF;
							dir->d_stat.st_attr = 0x0010;
							dir->d_stat.st_size = 0;	
							
							// Change the modifcation time to creation time
							memcpy(&dir->d_stat.st_mtime, &dir->d_stat.st_ctime, sizeof(ScePspDateTime));

							if (docache)
							{
								Cache(&vpbp);									
							}
							
							if (videoiso_mounted)
								ReturnToDisc();
						}
					}
					else
					{
						goto NEXT;
					}
					
					pspSdkSetK1(k1);
					return res;
				}

				else
				{
					sceIoDclose(isodfd);
					isodfd = -1;
					overiso = 1;
				}
			}			
		}
	}

	res = sceIoDread(fd, dir);

	if (res > 0)
	{
		if (config.hidecorrupt)
			CorruptIconPatch(dir->d_name, 0);
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoDclosePatched(SceUID fd)
{
	u32 k1 = pspSdkSetK1(0);
	int res;

	if (vpbpinited)
	{	
		res = virtualpbp_close(fd);	
		if (res >= 0)
		{
			pspSdkSetK1(k1);
			return res;
		}
	}

	if (fd == gamedfd)
	{
		gamedfd = -1;
		over5xx = 0;
		overiso = 0;		
		SaveCache();		
	}

	pspSdkSetK1(k1);
	return sceIoDclose(fd);
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode)
{
	u32 k1 = pspSdkSetK1(0);	
	int index;
		
	Fix150Path(file);
	Fix5XXPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		if (videoiso_mounted)
			ReturnToDisc();
		
		int res = virtualpbp_open(index);
		
		pspSdkSetK1(k1);
		return res;
	}

	if (strstr(file, "disc0:/PSP_GAME/PARAM.SFO"))
	{
		pspSdkSetK1(k1);
		paramsfo = sceIoOpen(file, flags, mode);
		return paramsfo;
	}

	pspSdkSetK1(k1);

	return sceIoOpen(file, flags, mode);
}

int sceIoClosePatched(SceUID fd)
{
	u32 k1 = pspSdkSetK1(0);	
	int res = -1;

	if (vpbpinited)
	{	
		res = virtualpbp_close(fd);		
	}

	if (fd == paramsfo)
	{
		paramsfo = -1;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoClose(fd);

	return res;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size)
{
	u32 k1 = pspSdkSetK1(0);	
	int res = -1;
	
	if (vpbpinited)
	{
		res = virtualpbp_read(fd, data, size);		
	}

	if (fd == paramsfo)
	{
		int i;

		pspSdkSetK1(k1);		
		res = sceIoRead(fd, data, size);
		pspSdkSetK1(0);

		if (res > 4)
		{
			for (i = 0; i < res-4; i++)
			{
				if (memcmp(data+i, "5.", 2) == 0)
				{
					if (strlen(data+i) == 4)
					{
						memcpy(data+i, "5.00", 4);
						break;
					}
				}
			}
		}

		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoRead(fd, data, size);

	return res;
}

SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence)
{
	u32 k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek(fd, offset, whence);

	return res;
}

int sceIoLseek32Patched(SceUID fd, int offset, int whence)
{
	u32 k1 = pspSdkSetK1(0);
	int res = -1;

	if (vpbpinited)
	{
		res = virtualpbp_lseek(fd, offset, whence);		
	}

	pspSdkSetK1(k1);

	if (res < 0)
		return sceIoLseek32(fd, offset, whence);

	return res;
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix5XXPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_getstat(index, stat);
		
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoGetstat(file, stat);
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix5XXPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_chstat(index, stat, bits);
		
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoChstat(file, stat, bits);
}

int sceIoRemovePatched(const char *file)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(file);
	Fix5XXPath(file);

	index = GetIsoIndex(file);
	if (index >= 0)
	{
		int res = virtualpbp_remove(index);
		
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoRemove(file);
}

int sceIoRmdirPatched(const char *path)
{
	u32 k1 = pspSdkSetK1(0);
	int index;

	Fix150Path(path);
	Fix5XXPath(path);

	index = GetIsoIndex(path);
	if (index >= 0)
	{
		int res = virtualpbp_rmdir(index);
		
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);

	return sceIoRmdir(path);
}

int sceIoMkdirPatched(const char *dir, SceMode mode)
{
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0)
	{
		sceIoMkdir("ms0:/PSP/GAME150", mode);
		sceIoMkdir("ms0:/PSP/GAME5XX", mode);
		sceIoMkdir("ms0:/ISO", mode);
		sceIoMkdir("ms0:/ISO/VIDEO", mode);
		sceIoMkdir("ms0:/seplugins", mode);
	}

	pspSdkSetK1(k1);
	return sceIoMkdir(dir, mode);
}

void IoPatches()
{
	SceModule2 *mod;
	u32 text_addr;

	mod = sceKernelFindModuleByName("sceIOFileManager");
	text_addr = mod->text_addr;

	// Patcth IoFileMgrForUser syscalls
	PatchSyscall(text_addr+0x144C, sceIoDopenPatched);
	PatchSyscall(text_addr+0x15CC, sceIoDreadPatched);
	PatchSyscall(text_addr+0x167C, sceIoDclosePatched);
	PatchSyscall(text_addr+0x3CD0, sceIoOpenPatched);
	PatchSyscall(text_addr+0x3C90, sceIoClosePatched);
	PatchSyscall(text_addr+0x3DE8, sceIoReadPatched);
	PatchSyscall(text_addr+0x3E58, sceIoLseekPatched);
	PatchSyscall(text_addr+0x3E90, sceIoLseek32Patched);
	PatchSyscall(text_addr+0x3F84, sceIoGetstatPatched);
	PatchSyscall(text_addr+0x3FA4, sceIoChstatPatched);
	PatchSyscall(text_addr+0x171C, sceIoRemovePatched);
	PatchSyscall(text_addr+0x3F44, sceIoRmdirPatched);
	PatchSyscall(text_addr+0x3F28, sceIoMkdirPatched);
}

int sceCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count)
{
	int res = sceCtrlReadBufferPositive(pad_data, count);
	int k1 = pspSdkSetK1(0);

	if (config.vshcpuspeed != 0)
	{
		if (!set)
		{
			if (config.vshcpuspeed != 222 && scePowerGetCpuClockFrequencyInt() == 222)
			{
				int tickcurr = sceKernelGetSystemTimeLow();
				if(tickcurr - ticklast >= (10 * 1000 * 1000))
				{
					SetSpeed(config.vshcpuspeed, config.vshbusspeed);
					ticklast = tickcurr;
				}
			}
		}
		else
		{
			int tickcurr = sceKernelGetSystemTimeLow();
			if ((tickcurr - firsttick) >= (10*1000*1000))
			{
				set = 1;
				SetSpeed(config.vshcpuspeed, config.vshbusspeed);
				ticklast = tickcurr;
			}
		}
	}
	
	if (!sceKernelFindModuleByName("VshCtrlSatelite"))
	{
		if (!sceKernelFindModuleByName("htmlviewer_plugin_module") &&
			!sceKernelFindModuleByName("sceVshOSK_Module") &&
			!sceKernelFindModuleByName("camera_plugin_module"))
		{
			if (pad_data->Buttons & PSP_CTRL_SELECT)
			{
				sceKernelSetDdrMemoryProtection((void *)0x08400000, 4*1024*1024, 0xF); 

				lmoption.size = sizeof(lmoption);
				lmoption.mpidtext = 5;
				lmoption.mpiddata = 5;
				lmoption.access = 1;
				satelite = sceKernelLoadModule("flash0:/vsh/module/satelite.prx", 0, &lmoption);

				if (satelite >= 0)
				{
					char *argp;
					SceSize args;

					if (videoiso_mounted)
					{
						argp = GetUmdFile();
						args = strlen(argp)+1;
					}
					else
					{
						argp = NULL;
						args = 0;
					}
					
					sceKernelStartModule(satelite, args, argp, NULL, NULL);

					pad_data->Buttons &= ~(PSP_CTRL_SELECT);
				}				
			}
		}
	}
	else
	{
		if (vshmenu_ctrl)
		{
			vshmenu_ctrl(pad_data, count);
		}
		else if (satelite >= 0)
		{
			if (sceKernelStopModule(satelite, 0, NULL, NULL, NULL) >= 0)
			{
				sceKernelUnloadModule(satelite);
			}
		}
	}

	pspSdkSetK1(k1);
	return res;
}

int vctrlVSHRegisterVshMenu(int (* ctrl)(SceCtrlData *, int))
{
	int k1 = pspSdkSetK1(0);

	vshmenu_ctrl = (void *)(0x80000000 | (u32)ctrl);

	pspSdkSetK1(k1);
	return 0;
}

int vctrlVSHExitVSHMenu(SEConfig *conf, char *videoiso, int disctype)
{
	int k1 = pspSdkSetK1(0);
	int oldspeed = config.vshcpuspeed;
	
	vshmenu_ctrl = NULL;
	memcpy(&config, conf, sizeof(SEConfig));
	SetConfig(&config);

	if (set)
	{
		if (config.vshcpuspeed != oldspeed)
		{		
			if (config.vshcpuspeed)			
				SetSpeed(config.vshcpuspeed, config.vshbusspeed);
			else
				SetSpeed(222, 111);
		}
	}

	if (!videoiso)
	{
		if (videoiso_mounted)
			ReturnToDisc();
	}
	else
	{
		sctrlSESetDiscType(disctype);

		if (!videoiso_mounted)
		{
			sctrlSESetDiscOut(1);	
			videoiso_mounted = 1;
		}
		else
		{
			if (strcmp(GetUmdFile(), videoiso) == 0)
			{
				pspSdkSetK1(k1);
				return 0;
			}

			sctrlSEUmountUmd();
		}

		sctrlSEMountUmdFromFile(videoiso, 0, 0);
	}

	pspSdkSetK1(k1);
	return 0;
}

int sceIoDevctlPatched(const char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	int res = sceIoDevctl(dev, cmd, indata, inlen, outdata, outlen);
	
	if (cmd == 0x01E18030 && videoiso_mounted)
	{
		res = 1;
	}

	return res;
}

int (*getVersion)(void *, int, int *);
int GetVersionHook(void *buffer, int bufferlen, int *len)
{
	int ret = getVersion(buffer, bufferlen, len);

	int k1 = pspSdkSetK1(0);

	if (strstr(buffer, "release:5.00"))
	{
		int fd = sceIoOpen("flash0:/vsh/etc/version.txt", 1, 0);
		if (fd < 0)
			return fd;

		*len = sceIoRead(fd, buffer, bufferlen);
		sceIoClose(fd);
		
		ret = 0;
	}
	
	pspSdkSetK1(k1);
	return ret;
}

int sceUsbStartPatched(const char* driverName, int size, void *args)
{
	int k1 = pspSdkSetK1(0);

	if (!strcmp(driverName, PSP_USBSTOR_DRIVERNAME) && config.usbdevice >= 1 && config.usbdevice <= 5)
	{
		if (videoiso_mounted && config.usbdevice == 5)
		{
			return_iso = 1;
			ReturnToDisc();
		}		
		
		usbdev = sceKernelLoadModule("flash0:/kd/usbdevice.prx", 0, NULL);
		if (usbdev >= 0)
		{
			if (sceKernelStartModule(usbdev, 0, NULL, NULL, NULL) >= 0)
			{
				int res = pspUsbDeviceSetDevice(config.usbdevice-1, 0, 0);
				if (res < 0)
				{
					pspUsbDeviceFinishDevice();
				}
			}
		}
	}

	pspSdkSetK1(k1);
	return sceUsbStart(driverName, size, args);
}

int sceUsbStopPatched(const char* driverName, int size, void *args)
{
	int res = sceUsbStop(driverName, size, args);
	int k1 = pspSdkSetK1(0);
		
	if (!strcmp(driverName, PSP_USBSTOR_DRIVERNAME) && usbdev >= 0 && config.usbdevice >= 1 && config.usbdevice <= 5)
	{
		pspUsbDeviceFinishDevice();

		if (sceKernelStopModule(usbdev, 0, NULL, NULL, NULL) >= 0)
		{
			sceKernelUnloadModule(usbdev);
			usbdev = -1;
		}
		
		if (return_iso && config.usbdevice == 5)
		{
			sctrlSESetDiscOut(1);	
			sctrlSEMountUmdFromFile(GetUmdFile(), 0, 0);
			videoiso_mounted = 1;
			return_iso = 0;
		}
	}

	pspSdkSetK1(k1);
	return res;
}

void PatchVshMain(u32 text_addr)
{
	// Allow old sfo's.
	_sw(NOP, text_addr+0xEE30);
	_sw(NOP, text_addr+0xEE38);
	_sw(0x10000023, text_addr+0xF0D8);
 
	IoPatches();

	SceModule2 *mod = sceKernelFindModuleByName("sceVshBridge_Driver");

	if (!config.novshmenu)
	{
		MAKE_CALL(mod->text_addr+0x264, sceCtrlReadBufferPositivePatched);
		PatchSyscall(FindProc("sceController_Service", "sceCtrl", 0x1F803938), sceCtrlReadBufferPositivePatched);
	}
	
	// For umd video iso
	MAKE_CALL(mod->text_addr+0x724, sceIoDevctlPatched);

	// PATCH UPDATE HERE
	
	if (config.useversiontxt)
	{
		getVersion = FindProc("sceMesgLed", "sceResmgr", 0x9DC14891);
		PatchSyscall(getVersion, GetVersionHook);
	}

	PatchSyscall(FindProc("sceUSB_Driver", "sceUsb", 0xAE5DE6AF), sceUsbStartPatched);
	PatchSyscall(FindProc("sceUSB_Driver", "sceUsb", 0xC2464FA0), sceUsbStopPatched);

	ClearCaches();	
}

wchar_t verinfo[] = L"5.00 M33  ";
void PatchSysconfPlugin(u32 text_addr)
{	
	int version = sctrlSEGetVersion() & 0xF;

	if (version)
	{
		((char *)verinfo)[16] = '-';
		((char *)verinfo)[18] = version+'1';
	}

	u32 version_text = text_addr+0x23DE0;
	
	// lui v0, addrhigh
	_sw(0x3c020000 | (version_text >> 16), text_addr+0x15EE0);
	// ori v0, v0, addrlow
	_sw(0x34420000 | (version_text & 0xFFFF), text_addr+0x15EE4);
	
	memcpy((void *) version_text, verinfo, sizeof(verinfo));

	ClearCaches();
}

void PatchMsVideoMainPlugin(u32 text_addr)
{
	// Patch resolution limit to 130560 pixels (480x272)
	_sh(0xfe00, text_addr+0x2F3E4);
	_sh(0xfe00, text_addr+0x2F46C);
	_sh(0xfe00, text_addr+0x31B88);
	_sh(0xfe00, text_addr+0x31BFC);
	_sh(0xfe00, text_addr+0x31D08);
	_sh(0xfe00, text_addr+0x377A4);
	_sh(0xfe00, text_addr+0x5EB20);
	
	// Patch bitrate limit 768+2 (increase to 16384+2)
	_sh(0x4003, text_addr+0x31B24);
	_sh(0x4003, text_addr+0x31BB0);

	// Patch bitrate limit 4000 (increase to 16384+2)
	_sh(0x4003, text_addr+0x373BC);
	
	ClearCaches();
}

void PatchGamePlugin(u32 text_addr)
{
	// New Patch (3.71+)	
	_sw(0x03e00008, text_addr+0x11764);
	_sw(0x00001021, text_addr+0x11768);	

	if (config.hidepics)
	{
		// Hide pic0.png+pic1.png
		// mov v0, v1
		_sw(0x00601021, text_addr+0xF7A8);
		_sw(0x00601021, text_addr+0xF7B4);
	}
	
	ClearCaches();			
}

#if 0
void PatchUpdatePlugin(u32 text_addr)
{
	int se_version, high, low;

	se_version = sctrlSEGetVersion();
	high = (se_version >> 16);
	low = (se_version&0xFFFF);
	
	// lui a1, high
	_sw(0x3C050000 | high, text_addr+0x1F44); 
	// ori a0, a1, low
	_sw(0x34A40000 | low, text_addr+0x210C); 

	ClearCaches();
}
#endif

STMOD_HANDLER previous;
int OnModuleStart(SceModule2 *mod)
{
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;

	if (strcmp(modname, "vsh_module") == 0)
	{
		PatchVshMain(text_addr);
	}
	
	else if (strcmp(modname, "sysconf_plugin_module") == 0)
	{
		PatchSysconfPlugin(text_addr);
	}
			
	else if (strcmp(modname, "msvideo_main_plugin_module") == 0)
	{
		PatchMsVideoMainPlugin(text_addr);
	}

	else if (strcmp(modname, "game_plugin_module") == 0)
	{
		PatchGamePlugin(text_addr);					
	}

#if 0
	else if (strcmp(modname, "update_plugin_module") == 0)
	{
		if (!config.notusedaxupd)
			PatchUpdatePlugin(text_addr);
	}
#endif

	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp)
{
	SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
	
	MAKE_CALL(mod->text_addr+0x17FC, LoadExecVSHCommonPatched);
	MAKE_CALL(mod->text_addr+0x1824, LoadExecVSHCommonPatched);
	MAKE_CALL(mod->text_addr+0x1974, LoadExecVSHCommonPatched);
	MAKE_CALL(mod->text_addr+0x199C, LoadExecVSHCommonPatched);

	PatchSyscall(FindProc("sceRTC_Service", "sceRtc", 0xE7C27D1B), sceRtcGetCurrentClockLocalTimePatched);

	sctrlSEGetConfig(&config);	

	if (config.vshcpuspeed != 0)
		firsttick = sceKernelGetSystemTimeLow();
	
	ClearCaches();

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
