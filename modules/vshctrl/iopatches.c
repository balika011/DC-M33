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





SceUID gamedfd = -1, game150dfd = -1, game5xxdfd = -1, isodfd = -1, over5xx = 0, overiso = 0, paramsfo = -1;
int vpbpinited = 0, isoindex = 0, cachechanged = 0;
VirtualPbp vpbp;
VirtualPbp *cache = NULL;
u8 referenced[32];

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
