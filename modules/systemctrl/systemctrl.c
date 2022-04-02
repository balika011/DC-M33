#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <psploadcore.h>
#include "main.h"
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "sysmodpatches.h"
#include "umd9660_driver.h"
#include "isofs_driver.h"

int sctrlKernelSetUserLevel(int level)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();
	
	SceModule2 *mod = sceKernelFindModuleByName("sceThreadManager");
	u32 *thstruct;

	thstruct = (u32 *)_lw(mod->text_addr + 0x19F40); // OK

	thstruct[0x14/4] = (level ^ 8) << 28;

	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelSetDevkitVersion(int version)
{
	int k1 = pspSdkSetK1(0);
	int prev = sceKernelDevkitVersion();

	int high = version >> 16;
	int low = version & 0xFFFF;

	_sh(high, 0x8801191C);
	_sh(low, 0x88011924);

	ClearCaches();

	pspSdkSetK1(k1);
	return prev;
}

int sctrlHENIsSE()
{
	return 1;
}

int sctrlHENIsDevhook()
{
	return 0;
}

int sctrlHENGetVersion()
{
	return 0x00001000;
}

int sctrlSEGetVersion()
{
	return 0x00020000;
}

int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHDisc(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHDiscUpdater(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadExecVSHMs1(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = sceKernelLoadExecVSHMs2(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = sceKernelLoadExecVSHMs3(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);	
	int res = sceKernelLoadExecVSHMs4(file, param);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
	int (* LoadExecVSH)(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param, int unk2) = (void *)(mod->text_addr + 0x2384); // OK, TODO: 5G: 0x25D8
	int res = LoadExecVSH(apitype, file, param, 0x10000);
	pspSdkSetK1(k1);

	return res;
}

int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param)
{
	int k1 = pspSdkSetK1(0);
	int res = sceKernelExitVSHVSH(param);
	pspSdkSetK1(k1);

	return res;
}

static void PatchIsofsDriver2()
{
	SceModule2 *mod = sceKernelFindModuleByName("sceIsofs_driver");
	if (mod)
	{
		_sw(0x03e00008, mod->text_addr + 0x4330); // OK
		_sw(0x34020000, mod->text_addr + 0x4334); // OK
		ClearCaches();
	}
}

int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs)
{
	int k1 = pspSdkSetK1(0);

	SetUmdFile(file);

	if (!noumd && !isofs)
	{
		DoAnyUmd();
	}
	else
	{
		int res;
		if ((res = sceIoDelDrv("umd")) < 0)
			return res;

		if ((res = sceIoAddDrv(getumd9660_driver())) < 0)
			return res;
	}

	if (noumd)
	{
		DoNoUmdPatches();
	}

	if (isofs)
	{
		sceIoDelDrv("isofs");
		sceIoAddDrv(getisofs_driver());
		PatchIsofsDriver2();

		sceIoAssign("disc0:", "umd0:", "isofs0:", IOASSIGN_RDONLY, NULL, 0);
	}

	pspSdkSetK1(k1);

	return 0;
}

SceIoDeviceTable *sctrlHENFindDriver(char *drvname)
{
	int k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");
	u32 text_addr = mod->text_addr;

	u32 *(* lookup_device_list)(char *) = (void *)(text_addr + 0x2A4C); // OK
	u32 *u = lookup_device_list(drvname);
	if (!u)
	{
		pspSdkSetK1(k1);
		return NULL;
	}

	pspSdkSetK1(k1);

	return (SceIoDeviceTable *) u[1];
}

SceIoDeviceEntry *sctrlHENFindEntry(char *alias)
{
	int k1 = pspSdkSetK1(0);
	SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");
	u32 text_addr = mod->text_addr;

	u32 *(* parsealias)(char *) = (void *)(text_addr + 0x35D0); // OK
	u32 *u = parsealias(alias);
	if (!u)
	{
		pspSdkSetK1(k1);
		return NULL;
	}

	pspSdkSetK1(k1);

	return (SceIoDeviceEntry *) u[1];
}

