#include <pspsdk.h>
#include <pspkernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_internal.h>


#include <stdio.h>
#include <string.h>
#include <oe_malloc.h>

#include <main.h>

static int plugindone = 0;
extern SEConfig config;
SceUID evflag;
int ms_status;

int LoadStartModule(char *module)
{
	SceUID mod = sceKernelLoadModule(module, 0, NULL);	

	if (mod < 0)
		return mod;

	return sceKernelStartModule(mod, strlen(module)+1, module, NULL, NULL);
}

void trim(char *str)
{
	int len = strlen(str);
	int i;

	for (i = len-1; i >= 0; i--)
	{
		if (str[i] == 0x20)
		{
			str[i] = 0;
		}
		else
		{
			break;
		}
	}
}

int ReadLine(char *buf, int size, char *str, int *enabled)
{
	char ch = 0;
	int n = 0;
	int i = 0;
	char *s = str;

	while (1)
	{	
		if (i >= size)
			break;

		ch = buf[i];

		if(ch < 0x20 && ch != '\t')
		{
			if (n != 0)
			{
				i++;
				break;
			}
		}
		else
		{
			*str++ = ch;
			n++;
		}

		i++;
	}

	trim(s);

	*enabled = 0;

	if(i > 0)
	{
		char *p = strpbrk(s, " \t");
		if(p)
		{
			char *q = p + 1;
			while(*q < 0) q++;
			if(strcmp(q, "1") == 0)
				*enabled = 1;
			*p = 0;
		}   
	}

	return i;
}

int Ms_Callback(u32 a0, int status)
{
	ms_status = status;
	sceKernelSetEventFlag(evflag, 1);

	return 0;
}

int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option)
{
	SceModule2 *mod = sceKernelFindModuleByUID(modid);
	SceUID fpl = -1;
	u32 *vshmain_args = NULL;
	char *plug_buf = NULL;
	char *p;
	int res;

	if (mod != NULL)
	{		
		//Kprintf(mod->modname);
		if (strcmp(mod->modname, "vsh_module") == 0)
		{
			if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH)
			{
				if (argsize == 0)
				{
					if (config.skiplogo)
					{
						vshmain_args = (u32 *)oe_malloc(0x0400);
						
						memset(vshmain_args, 0, 0x0400);
						vshmain_args[0/4] = 0x0400;
						vshmain_args[4/4] = 0x20;
						vshmain_args[0x40/4] = 1;
						vshmain_args[0x280/4] = 1;
						vshmain_args[0x284/4] = 0x40003;

						argsize = 0x0400;
						argp = vshmain_args;						
					}
				}				
			}			
		}
		
		else if (!plugindone)
		{
			char *waitmodule;
						
			if (sceKernelFindModuleByName("sceNp9660_driver"))
			{
				ms_status = 1;
			}
			
			
			if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_POPS)
			{
				waitmodule = "sceIoFilemgrDNAS";
			}
			else
			{
				waitmodule = "sceMediaSync";
			}

			if (sceKernelFindModuleByName(waitmodule))
			{
				SceUID fd;
	
				plugindone = 1;
				int initmode = sceKernelInitKeyConfig();	
				
				if (initmode == PSP_INIT_KEYCONFIG_VSH && !sceKernelFindModuleByName("scePspNpDrm_Driver"))
				{
					goto START_MODULE;
				}
				
				if (!ms_status)
				{				
					int out;
					if (sceIoDevctl("mscmhc0:", 0x02025806, NULL, 0, &out, sizeof(out)) >= 0)
					{
						if (out == 1)
						{				
							evflag = sceKernelCreateEventFlag("", 0, 0, NULL);
							SceUID cb = sceKernelCreateCallback("", (void *)Ms_Callback, NULL);
						
							if (sceIoDevctl("fatms0:", 0x02415821, &cb, 4, NULL, 0) >= 0)
							{
								sceKernelWaitEventFlagCB(evflag, 1, 0x11, NULL, NULL);
								sceIoDevctl("fatms0:", 0x02415822, &cb, 4, NULL, 0);
							}

							sceKernelDeleteCallback(cb);
							sceKernelDeleteEventFlag(evflag);
						}
					}
				}

				if (!ms_status)
				{
					goto START_MODULE;
				}

				fpl = sceKernelCreateFpl("", PSP_MEMORY_PARTITION_KERNEL, 0, 1024, 1, NULL);
				if (fpl < 0)
					goto START_MODULE;

				sceKernelAllocateFpl(fpl, (void *)&plug_buf, NULL);

				fd = sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_RDONLY, 0);
				if (fd >= 0)
				{
					int size = sceIoRead(fd, plug_buf, 1024);
					p = plug_buf;
					
					for (int i = 0; i < 5; i++)
					{
						char plugin[64];
						memset(plugin, 0, sizeof(plugin));
						
						int enabled;
						res = ReadLine(p, size, plugin, &enabled);
						if (res > 0)
						{
							if (initmode == PSP_INIT_KEYCONFIG_VSH)
							{
								if (enabled)
								{
									LoadStartModule(plugin);								
								}
							}

							size -= res;
							p += res;
						}
						else
						{
							break;
						}
					}

					sceIoClose(fd);
				}

				if (initmode == PSP_INIT_KEYCONFIG_VSH)
					goto START_MODULE;

				fd= sceIoOpen("ms0:/seplugins/game.txt", PSP_O_RDONLY, 0);
				if (fd >= 0)
				{
					int size = sceIoRead(fd, plug_buf, 1024);
					p = plug_buf;
					
					for (int i = 0; i < 5; i++)
					{
						char plugin[64];
						memset(plugin, 0, sizeof(plugin));

						int enabled;
						res = ReadLine(p, size, plugin, &enabled);

						if (res > 0)
						{
							if (initmode == PSP_INIT_KEYCONFIG_GAME)
							{
								if (enabled)
								{
									LoadStartModule(plugin);									
								}
							}

							size -= res;
							p += res;
						}
						else
						{
							break;
						}
					}

					sceIoClose(fd);
				}

				if (initmode == PSP_INIT_KEYCONFIG_GAME)
					goto START_MODULE;

				if (initmode == PSP_INIT_KEYCONFIG_POPS)
				{
					fd = sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_RDONLY, 0);
					if (fd >= 0)
					{
						int size = sceIoRead(fd, plug_buf, 1024);
						p = plug_buf;
						
						for (int i = 0; i < 5; i++)
						{
							char plugin[64];
							memset(plugin, 0, sizeof(plugin));
							
							int enabled;
							res = ReadLine(p, size, plugin, enabled);
							if (res > 0)
							{							
								if (enabled)
								{
									LoadStartModule(plugin);								
								}

								size -= res;
								p += res;
							}
							else
							{
								break;
							}						
						}
					
						sceIoClose(fd);	
					}
				}
			}
		}
	}

START_MODULE:

	res = sceKernelStartModule(modid, argsize, argp, status, option);
	
	if (vshmain_args)
	{
		oe_free(vshmain_args);		
	}
	
	if (plug_buf)
	{
		sceKernelFreeFpl(fpl, plug_buf);
		sceKernelDeleteFpl(fpl);
	}

	return res;
}

int PatchInit(int (* module_bootstart)(SceSize, BootInfo *), BootInfo *bootinfo)
{
	u32 text_addr = ((u32)module_bootstart) - 0x1A4C; // OK

	MAKE_JUMP(text_addr + 0x1C3C, sceKernelStartModulePatched); // OK
	ClearCaches();
	
	return module_bootstart(4, bootinfo);
	
}

