#include <pspsdk.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspsuspend.h>
#include <psppower.h>
#include <pspreg.h>
#include <psprtc.h>
#include <psputils.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include <pspdecrypt.h>
#include <pspipl_update.h>
#include <vlf.h>

#include "dcman.h"
#include "main.h"
#include "install.h"

extern u8 *big_buffer;
extern u8 *sm_buffer1, *sm_buffer2;
extern int status, progress_text, progress_bar;

const char *common[] =
{
	"data/cert/CA_LIST.cer",
	"dic/apotp.dic",
	"dic/atokp.dic",
	"dic/aux0.dic",
	"dic/aux1.dic",
	"dic/aux2.dic",
	"dic/aux3.dic",
	"font/gb3s1518.bwfon",
	"font/imagefont.bin",
	"font/jpn0.pgf",
	"font/kr0.pgf",
	"font/ltn0.pgf",
	"font/ltn1.pgf",
	"font/ltn2.pgf",
	"font/ltn3.pgf",
	"font/ltn4.pgf",
	"font/ltn5.pgf",
	"font/ltn6.pgf",
	"font/ltn7.pgf",
	"font/ltn8.pgf",
	"font/ltn9.pgf",
	"font/ltn10.pgf",
	"font/ltn11.pgf",
	"font/ltn12.pgf",
	"font/ltn13.pgf",
	"font/ltn14.pgf",
	"font/ltn15.pgf",
	"kd/amctrl.prx",
	"kd/ata.prx",
	"kd/audio.prx",
	"kd/audiocodec_260.prx",
	"kd/avcodec.prx",
	"kd/cert_loader.prx",
	"kd/chkreg.prx",
	"kd/chnnlsv.prx",
	"kd/clockgen.prx",
	"kd/codec.prx",
	"kd/ctrl.prx",
	"kd/dmacman.prx",
	"kd/exceptionman.prx",
	"kd/fatms.prx",
	"kd/g729.prx",
	"kd/ge.prx",
	"kd/http_storage.prx",
	"kd/idmanager.prx",
	"kd/idstorage.prx",
	"kd/ifhandle.prx",
	"kd/impose.prx",
	"kd/init.prx",
	"kd/interruptman.prx",
	"kd/iofilemgr.prx",
	"kd/iofilemgr_dnas.prx",
	"kd/irda.prx",
	"kd/isofs.prx",
	"kd/led.prx",
	"kd/lfatfs.prx",
	"kd/lflash_fatfmt.prx",
	"kd/libaac.prx",
	"kd/libasfparser.prx",
	"kd/libatrac3plus.prx",
	"kd/libaudiocodec2.prx",
	"kd/libdnas.prx",
	"kd/libdnas_core.prx",
	"kd/libhttp.prx",
	"kd/libmp3.prx",
	"kd/libparse_http.prx",
	"kd/libparse_uri.prx",
	"kd/libssl.prx",
	"kd/libupdown.prx",
	"kd/loadcore.prx",
	"kd/lowio.prx",
	"kd/mcctrl.prx",
	"kd/me_wrapper.prx",
	"kd/mediaman.prx",
	"kd/mediasync.prx",
	"kd/memab.prx",
	"kd/mgr.prx",
	"kd/mgvideo.prx",
	"kd/mlnbridge.prx",
	"kd/modulemgr.prx",
	"kd/mpeg.prx",
	"kd/mpeg_vsh.prx",
	"kd/mpegbase_260.prx",
	"kd/msaudio.prx",
	"kd/msstor.prx",
	"kd/np.prx",
	"kd/np9660.prx",
	"kd/np_auth.prx",
	"kd/np_commerce2.prx",
	"kd/np_commerce2_regcam.prx",
	"kd/np_commerce2_store.prx",
	"kd/np_core.prx",
	"kd/np_inst.prx",
	"kd/np_matching2.prx",
	"kd/np_service.prx",
	"kd/npdrm.prx",
	"kd/openpsid.prx",
	"kd/pops.prx",
	"kd/popsman.prx",
	"kd/power.prx",
	"kd/psheet.prx",
	"kd/pspnet.prx",
	"kd/pspnet_adhoc.prx",
	"kd/pspnet_adhoc_auth.prx",
	"kd/pspnet_adhoc_discover.prx",
	"kd/pspnet_adhoc_download.prx",
	"kd/pspnet_adhoc_matching.prx",
	"kd/pspnet_adhoc_transfer_int.prx",
	"kd/pspnet_adhocctl.prx",
	"kd/pspnet_apctl.prx",
	"kd/pspnet_inet.prx",
	"kd/pspnet_resolver.prx",
	"kd/pspnet_upnp.prx",
	"kd/pspnet_wispr.prx",
	"kd/registry.prx",
	"kd/resource/big5_table.dat",
	"kd/resource/cp949_table.dat",
	"kd/resource/gbk_table.dat",
	"kd/resource/impose.rsc",
	"kd/rtc.prx",
	"kd/sc_sascore.prx",
	"kd/semawm.prx",
	"kd/sircs.prx",
	"kd/syscon.prx",
	"kd/sysmem.prx",
	"kd/systimer.prx",
	"kd/threadman.prx",
	"kd/umd9660.prx",
	"kd/umdman.prx",
	"kd/usb.prx",
	"kd/usbacc.prx",
	"kd/usbcam.prx",
	"kd/usbgps.prx",
	"kd/usbmic.prx",
	"kd/usbpspcm.prx",
	"kd/usbstor.prx",
	"kd/usbstorboot.prx",
	"kd/usbstormgr.prx",
	"kd/usbstorms.prx",
	"kd/usersystemlib.prx",
	"kd/utility.prx",
	"kd/vaudio.prx",
	"kd/videocodec_260.prx",
	"kd/vshbridge.prx",
	"kd/wlan.prx",
	"vsh/etc/cptbl.dat",
	"vsh/etc/version.txt",
	"vsh/module/adhoc_transfer.prx",
	"vsh/module/auth_plugin.prx",
	"vsh/module/camera_plugin.prx",
	"vsh/module/common_gui.prx",
	"vsh/module/common_util.prx",
	"vsh/module/content_browser.prx",
	"vsh/module/dd_helper.prx",
	"vsh/module/dd_helper_utility.prx",
	"vsh/module/dialogmain.prx",
	"vsh/module/dnas_plugin.prx",
	"vsh/module/file_parser_base.prx",
	"vsh/module/game_install_plugin.prx",
	"vsh/module/game_plugin.prx",
	"vsh/module/htmlviewer_plugin.prx",
	"vsh/module/htmlviewer_ui.prx",
	"vsh/module/htmlviewer_utility.prx",
	"vsh/module/hvauth_r.prx",
	"vsh/module/impose_plugin.prx",
	"vsh/module/launcher_plugin.prx",
	"vsh/module/lftv_main_plugin.prx",
	"vsh/module/lftv_middleware.prx",
	"vsh/module/lftv_plugin.prx",
	"vsh/module/libfont_arib.prx",
	"vsh/module/libfont_hv.prx",
	"vsh/module/libpspvmc.prx",
	"vsh/module/libslim.prx",
	"vsh/module/libwww.prx",
	"vsh/module/mcore.prx",
	"vsh/module/mlnusb.prx",
	"vsh/module/mm_flash.prx",
	"vsh/module/msgdialog_plugin.prx",
	"vsh/module/msvideo_main_plugin.prx",
	"vsh/module/msvideo_plugin.prx",
	"vsh/module/music_browser.prx",
	"vsh/module/music_main_plugin.prx",
	"vsh/module/music_parser.prx",
	"vsh/module/music_player.prx",
	"vsh/module/netconf_plugin.prx",
	"vsh/module/netconf_plugin_auto_bfl.prx",
	"vsh/module/netconf_plugin_auto_nec.prx",
	"vsh/module/netfront.prx",
	"vsh/module/netplay_client_plugin.prx",
	"vsh/module/netplay_server_plus_utility.prx",
	"vsh/module/netplay_server_utility.prx",
	"vsh/module/npadmin_plugin.prx",
	"vsh/module/npinstaller_plugin.prx",
	"vsh/module/npsignin_plugin.prx",
	"vsh/module/npsignup_plugin.prx",
	"vsh/module/opening_plugin.prx",
	"vsh/module/osk_plugin.prx",
	"vsh/module/osk_plugin_500.prx",
	"vsh/module/paf.prx",
	"vsh/module/pafmini.prx",
	"vsh/module/photo_browser.prx",
	"vsh/module/photo_main_plugin.prx",
	"vsh/module/photo_player.prx",
	"vsh/module/premo_plugin.prx",
	"vsh/module/ps3scan_plugin.prx",
	"vsh/module/psn_plugin.prx",
	"vsh/module/radioshack_plugin.prx",
	"vsh/module/rss_browser.prx",
	"vsh/module/rss_common.prx",
	"vsh/module/rss_downloader.prx",
	"vsh/module/rss_main_plugin.prx",
	"vsh/module/rss_reader.prx",
	"vsh/module/rss_subscriber.prx",
	"vsh/module/savedata_auto_dialog.prx",
	"vsh/module/savedata_plugin.prx",
	"vsh/module/savedata_utility.prx",
	"vsh/module/screenshot_plugin.prx",
	"vsh/module/store_browser_plugin.prx",
	"vsh/module/store_checkout_plugin.prx",
	"vsh/module/sysconf_plugin.prx",
	"vsh/module/update_plugin.prx",
	"vsh/module/video_main_plugin.prx",
	"vsh/module/video_plugin.prx",
	"vsh/module/visualizer_plugin.prx",
	"vsh/module/vshmain.prx",
	"vsh/resource/adhoc_transfer.rco",
	"vsh/resource/auth_plugin.rco",
	"vsh/resource/camera_plugin.rco",
	"vsh/resource/common_page.rco",
	"vsh/resource/content_browser_plugin.rco",
	"vsh/resource/dd_helper.rco",
	"vsh/resource/dnas_plugin.rco",
	"vsh/resource/game_install_plugin.rco",
	"vsh/resource/game_plugin.rco",
	"vsh/resource/gameboot.pmf",
	"vsh/resource/htmlviewer.res",
	"vsh/resource/htmlviewer_plugin.rco",
	"vsh/resource/impose_plugin.rco",
	"vsh/resource/lftv_main_plugin.rco",
	"vsh/resource/lftv_rmc_univer3in1.rco",
	"vsh/resource/lftv_rmc_univer3in1_jp.rco",
	"vsh/resource/lftv_rmc_univerpanel.rco",
	"vsh/resource/lftv_rmc_univerpanel_jp.rco",
	"vsh/resource/lftv_rmc_univertuner.rco",
	"vsh/resource/lftv_rmc_univertuner_jp.rco",
	"vsh/resource/lftv_tuner_jp_jp.rco",
	"vsh/resource/lftv_tuner_us_en.rco",
	"vsh/resource/msgdialog_plugin.rco",
	"vsh/resource/msvideo_main_plugin.rco",
	"vsh/resource/music_browser_plugin.rco",
	"vsh/resource/music_player_plugin.rco",
	"vsh/resource/netconf_dialog.rco",
	"vsh/resource/netplay_plugin.rco",
	"vsh/resource/npadmin_plugin.rco",
	"vsh/resource/npinstaller_plugin.rco",
	"vsh/resource/npsignin_plugin.rco",
	"vsh/resource/npsignup_plugin.rco",
	"vsh/resource/opening_plugin.rco",
	"vsh/resource/osk_plugin_500.rco",
	"vsh/resource/osk_utility.rco",
	"vsh/resource/photo_browser_plugin.rco",
	"vsh/resource/photo_player_plugin.rco",
	"vsh/resource/premo_plugin.rco",
	"vsh/resource/ps3scan_plugin.rco",
	"vsh/resource/psn_plugin.rco",
	"vsh/resource/radioshack_plugin.rco",
	"vsh/resource/rss_browser_plugin.rco",
	"vsh/resource/rss_downloader_plugin.rco",
	"vsh/resource/rss_subscriber.rco",
	"vsh/resource/savedata_plugin.rco",
	"vsh/resource/savedata_utility.rco",
	"vsh/resource/screenshot_plugin.rco",
	"vsh/resource/store_browser_plugin.rco",
	"vsh/resource/store_checkout_plugin.rco",
	"vsh/resource/sysconf_plugin.rco",
	"vsh/resource/sysconf_plugin_about.rco",
	"vsh/resource/system_plugin.rco",
	"vsh/resource/system_plugin_bg.rco",
	"vsh/resource/system_plugin_fg.rco",
	"vsh/resource/topmenu_icon.rco",
	"vsh/resource/topmenu_plugin.rco",
	"vsh/resource/update_plugin.rco",
	"vsh/resource/video_main_plugin.rco",
	"vsh/resource/video_plugin_videotoolbar.rco",
	"vsh/resource/visualizer_plugin.rco"
};

const char *slim[] =
{
	"font/arib.pgf",
	"kd/resource/me_t2img.img",
	"vsh/module/oneseg_core.prx",
	"vsh/module/oneseg_hal_toolbox.prx",
	"vsh/module/oneseg_launcher_plugin.prx",
	"vsh/module/oneseg_plugin.prx",
	"vsh/module/oneseg_sal.prx",
	"vsh/module/oneseg_sdk.prx",
	"vsh/module/oneseg_sdkcore.prx",
	"vsh/module/skype_main_plugin.prx",
	"vsh/module/skype_plugin.prx",
	"vsh/module/skype_skyhost.prx",
	"vsh/module/skype_ve.prx",
	"vsh/resource/13-27.bmp",
	"vsh/resource/custom_theme.dat",
	"vsh/resource/oneseg_plugin.rco",
	"vsh/resource/skype_main_plugin.rco"
};

const char *spec_01g[] =
{
	"kd/display_01g.prx",
	"kd/hpremote_01g.prx",
	"kd/loadexec_01g.prx",
	"kd/memlmd_01g.prx",
	"kd/mesg_led_01g.prx",
	"kd/pspbtcnf.bin",
	"kd/resource/me_blimg.img",
	"kd/resource/me_sdimg.img",
	"kd/resource/meimg.img",
	"kd/wlanfirm_01g.prx",
	"vsh/etc/index_01g.dat",
	"vsh/resource/01-12.bmp"
};

const char *spec_02g[] =
{
	"kd/display_02g.prx",
	"kd/hpremote_02g.prx",
	"kd/loadexec_02g.prx",
	"kd/memlmd_02g.prx",
	"kd/mesg_led_02g.prx",
	"kd/pspbtcnf_02g.bin",
	"kd/umdcache_02g.prx",
	"kd/usb1seg_02g.prx",
	"kd/usbdmb_02g.prx",
	"kd/wlanfirm_02g.prx",
	"vsh/etc/index_02g.dat",
	"vsh/resource/01-12.bmp"
};

const char *spec_03g[] =
{
	"kd/display_03g.prx",
	"kd/hpremote_03g.prx",
	"kd/loadexec_03g.prx",
	"kd/memlmd_03g.prx",
	"kd/mesg_led_03g.prx",
	"kd/pspbtcnf_03g.bin",
	"kd/umdcache_03g.prx",
	"kd/usb1seg_03g.prx",
	"kd/usbdmb_03g.prx",
	"kd/wlanfirm_03g.prx",
	"vsh/etc/index_03g.dat",
	"vsh/resource/01-12_03g.bmp"
};

const char *common_m33[] =
{
	"kd/vshctrl.prx",
	"kd/usbdevice.prx",
	"vsh/module/satelite.prx",
	"vsh/module/recovery.prx",
	"kd/popcorn.prx",
	"kd/march33.prx",
	"kd/idcanager.prx",
	"kd/galaxy.prx"
};

const char *spec_01g_m33[] =
{
	"kd/pspbtcnf_umd.bin",
	"kd/pspbtcnf_oe.bin",
	"kd/pspbtcnf_m33.bin",
	"kd/pspbtcnf_recovery.bin",
	"kd/systemctrl.prx"
};

const char *spec_02g_m33[] =
{
	"kd/pspbtcnf_02g_umd.bin",
	"kd/pspbtcnf_02g_oe.bin",
	"kd/pspbtcnf_02g_m33.bin",
	"kd/pspbtcnf_02g_recovery.bin",
	"kd/systemctrl_02g.prx"
};

const char *spec_03g_m33[] =
{
	"kd/pspbtcnf_03g_umd.bin",
	"kd/pspbtcnf_03g_oe.bin",
	"kd/pspbtcnf_03g_m33.bin",
	"kd/pspbtcnf_03g_recovery.bin",
	"kd/systemctrl_03g.prx"
};

int LoadUpdaterModules(int ofw)
{
	SceUID mod = sceKernelLoadModule("flash0:/kd/emc_sm_updater.prx", 0, NULL);
	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return 1;

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			return 2;
	}

	mod = sceKernelLoadModule("flash0:/kd/lfatfs_updater.prx", 0, NULL);
	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return 3;

	dcPatchModuleString("sceLFatFs_Updater_Driver", "flash", "flach");

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			return 4;
	}

	mod = sceKernelLoadModule("flash0:/kd/lflash_fatfmt_updater.prx", 0, NULL);
	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return 5;

	sceKernelDelayThread(10000);

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			return 6;
	}

	mod = sceKernelLoadModule("flash0:/kd/ipl_update.prx", 0, NULL);
	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return 7;

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			return 8;
	}

	mod = sceKernelLoadModule("flash0:/kd/pspdecrypt.prx", 0, NULL);
	if (mod < 0 && mod != SCE_KERNEL_ERROR_EXCLUSIVE_LOAD)
		return 9;

	if (mod >= 0)
	{
		mod = sceKernelStartModule(mod, 0, NULL, NULL, NULL);
		if (mod < 0)
			return 10;
	}

	return 0;
}

void SetInstallProgress(int value, int max, int force, int ofw)
{
	u32 prog;
	
	prog = (((ofw ? 95 : 93) * value) / max) + 4;

	SetProgress(prog, force);
}

char upd_error_msg[256];

int OnInstallError(void *param)
{
	int ofw = *(int *)param;
	
	vlfGuiMessageDialog(upd_error_msg, VLF_MD_TYPE_ERROR | VLF_MD_BUTTONS_NONE);

	vlfGuiRemoveText(status);
	vlfGuiRemoveText(progress_text);
	vlfGuiRemoveProgressBar(progress_bar);

	progress_bar = -1;
	progress_text = -1;
	status = -1;

	dcSetCancelMode(0);
	MainMenu(ofw);

	return VLF_EV_RET_REMOVE_HANDLERS;
}

void InstallError(int ofw, char *fmt, ...)
{
	va_list list;
	
	va_start(list, fmt);
	vsprintf(upd_error_msg, fmt, list);
	va_end(list);

	vlfGuiAddEventHandler(0, -1, OnInstallError, &ofw);
	sceKernelExitDeleteThread(0);
}

int OnInstallComplete(void *param)
{
	int ofw = *(int *)param;

	vlfGuiRemoveProgressBar(progress_bar);
	vlfGuiRemoveText(progress_text);

	dcSetCancelMode(0);

	if (ofw && kuKernelGetModel() == 1)
	{
		SetStatus("Install is complete.\n"
			      "A shutdown is required. A normal battery is\n"
				  "required to boot this firmware on this PSP.");
		 AddShutdownRebootBD(1);
	}
	else
	{
		SetStatus("Install is complete.\nA shutdown or a reboot is required.");
		AddShutdownRebootBD(0);
	}

	progress_bar = -1;
	progress_text = -1;	

	return VLF_EV_RET_REMOVE_HANDLERS;
}

int CreateDirs()
{
	int res = sceIoMkdir("flach0:/data", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/data/cert", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/dic", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/font", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/kd", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/kd/resource", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/vsh", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/vsh/etc", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/vsh/module", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach0:/vsh/resource", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	return 0;
}

int CreateFlash1Dirs()
{
	int res = sceIoMkdir("flach1:/dic", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/gps", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/net", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/net/http", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/registry", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/vsh", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	res = sceIoMkdir("flach1:/vsh/theme", 0777);
	if (res < 0 && res != 0x80010011)
		return res;

	return 0;
}

void DisablePlugins()
{
	u8 conf[15];

	memset(conf, 0, 15);
	
	WriteFile("ms0:/seplugins/conf.bin", conf, 15);
}

void CopyFileList(int ofw, const char **list, int file_count, int start_file_count, int max_file_count)
{
	for (int i = 0; i < file_count; i++)
	{
		char src[256];
		char dest[256];
		strcpy(src,  "flash0:/");
		strcpy(dest, "flach0:/");
		strcat(src, list[i]);
		strcat(dest, list[i]);
		
		SceUID fdi = sceIoOpen(src, PSP_O_RDONLY, 0);
		if (fdi < 0)
		{
			InstallError(ofw, "Error opening %s: 0x%08X", src, fdi);
		}
		
		SceUID fdo = sceIoOpen(dest, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fdo < 0)
		{
			InstallError(ofw, "Error opening %s: 0x%08X", dest, fdo);
		}
		
		char signechecked = 0;
		while (1)
		{
			int read = sceIoRead(fdi, big_buffer, BIG_BUFFER_SIZE);
			if (read < 0)
			{
				InstallError(ofw, "Error reading %s", src);
			}
			if (!read)
				break;
		
			if (!signechecked &&
				(memcmp(&list[i][strlen(list[i]) - 4], ".prx", 4) == 0 ||
				memcmp(list[i], "kd/pspbtcnf", 11) == 0))
			{
				pspSignCheck(big_buffer);
				signechecked = 1;
			}

			if (sceIoWrite(fdo, big_buffer, read) < 0)
			{
				InstallError(ofw, "Error writing %s", dest);
			}
		}
		
		sceIoClose(fdi);
		sceIoClose(fdo);

		SetInstallProgress(start_file_count + i, max_file_count, 0, ofw);
		scePowerTick(0);
	}
}

int install_thread(SceSize args, void *argp)
{
	int ofw = *(int *)argp;	
	char *argv[2];
	int res;
	int size;
	int model = kuKernelGetModel();

	dcSetCancelMode(1);
	
	switch(LoadUpdaterModules(ofw))
	{
		case 0: break;
		case 1: InstallError(ofw, "Failed to load emc_sm_updater.prx"); break;
		case 2: InstallError(ofw, "Failed to start emc_sm_updater.prx"); break;
		case 3: InstallError(ofw, "Failed to load lfatfs_updater.prx"); break;
		case 4: InstallError(ofw, "Failed to start lfatfs_updater.prx"); break;
		case 5: InstallError(ofw, "Failed to load lflash_fatfmt_updater.prx"); break;
		case 6: InstallError(ofw, "Failed to start lflash_fatfmt_updater.prx"); break;
		case 7: InstallError(ofw, "Failed to load ipl_update.prx"); break;
		case 8: InstallError(ofw, "Failed to start ipl_update.prx"); break;
		case 9: InstallError(ofw, "Failed to load pspdecrypt.prx"); break;
		case 10: InstallError(ofw, "Failed to start pspdecrypt.prx"); break;
		default: InstallError(ofw, "Error loading updater modules.");
	}

	// Unassign with error ignore (they might have been assigned in a failed attempt of install M33/OFW)
	sceIoUnassign("flach0:");
	sceIoUnassign("flach1:");
	sceIoUnassign("flach2:");
	sceIoUnassign("flach3:");

	sceKernelDelayThread(1200000);

	SetStatus("Formatting flash0... ");

	argv[0] = "fatfmt";
	argv[1] = "lflach0:0,0";

	res = dcLflashStartFatfmt(2, argv);
	if (res < 0)
	{
		InstallError(ofw, "Flash0 format failed: 0x%08X", res);
	}

	sceKernelDelayThread(1200000);
	SetProgress(1, 1);

	SetStatus("Formatting flash1... ");

	argv[1] = "lflach0:0,1";

	res = dcLflashStartFatfmt(2, argv);
	if (res < 0)
	{
		InstallError(ofw, "Flash1 format failed: 0x%08X", res);
	}

	sceKernelDelayThread(1200000);

	SetStatus("Formatting flash2... ");

	argv[1] = "lflach0:0,2";
	dcLflashStartFatfmt(2, argv);

	sceKernelDelayThread(1200000);
	SetProgress(2, 1);

	SetStatus("Assigning flashes...");

	res = sceIoAssign("flach0:", "lflach0:0,0", "flachfat0:", IOASSIGN_RDWR, NULL, 0);
	if (res < 0)
	{
		InstallError(ofw, "Flash0 assign failed: 0x%08X", res);  
	}

	res = sceIoAssign("flach1:", "lflach0:0,1", "flachfat1:", IOASSIGN_RDWR, NULL, 0);
	if (res < 0)
	{
		InstallError(ofw, "Flash1 assign failed: 0x%08X", res);  
	}

	sceIoAssign("flach2:", "lflach0:0,2", "flachfat2:", IOASSIGN_RDWR, NULL, 0);
	
	sceKernelDelayThread(1200000);
	SetProgress(3, 1);

	SetStatus("Creating directories...");

	if (CreateDirs() < 0)
	{
		InstallError(ofw, "Directories creation failed.");
	}

	sceKernelDelayThread(1200000);
	SetProgress(4, 1);

	SetStatus("Flashing files...");
	
	int file_count = sizeof(common) / sizeof(common[0]);
	if (model)
		file_count += sizeof(slim) / sizeof(slim[0]);

	const char ** spec = 0;
	int spec_count;
	switch (model)
	{
		case 0:
			spec = spec_01g;
			spec_count = sizeof(spec_01g) / sizeof(spec_01g[0]);
			break;
		case 1:
			spec = spec_02g;
			spec_count = sizeof(spec_02g) / sizeof(spec_02g[0]);
			break;
		case 2:
			spec = spec_03g;
			spec_count = sizeof(spec_03g) / sizeof(spec_03g[0]);
			break;
		default:
			InstallError(ofw, "Unsupported model.");
	}
	file_count += spec_count;
	
	CopyFileList(ofw, common, sizeof(common) / sizeof(common[0]), 0, file_count);
	if (model)
	{
		CopyFileList(ofw, slim, sizeof(slim) / sizeof(slim[0]), sizeof(common) / sizeof(common[0]), file_count);
		CopyFileList(ofw, spec, spec_count, sizeof(common) / sizeof(common[0]) + sizeof(slim) / sizeof(slim[0]), file_count);
	}
	else
	{		
		CopyFileList(ofw, spec, spec_count, sizeof(common) / sizeof(common[0]), file_count);
	}

	sceKernelDelayThread(200000);
	SetInstallProgress(1, 1, 1, ofw);

	SetStatus("Restoring registry...");
	sceKernelDelayThread(100000);

	if (CreateFlash1Dirs() < 0)
		InstallError(ofw, "Error creating flash1 directories.");

	if (WriteFile("flach1:/registry/init.dat", sm_buffer1, 0) < 0)
	{
		InstallError(ofw, "Cannot write init.dat\n");
	}

	res = ReadFile("flash2:/registry/act.dat", 0, sm_buffer1, SMALL_BUFFER_SIZE);
	if (res > 0)
	{
		WriteFile("flach2:/act.dat", sm_buffer1, res);		
	}

	SetStatus("Flashing IPL... ");
	
	const char *ipl_name = 0;
	u16 ipl_key = 0;
	
	if (ofw)
	{
		switch (model)
		{
			case 0: ipl_name = "flash0:/nandipl_01g.bin"; break;
			case 1: ipl_name = "flash0:/nandipl_02g.bin"; break;
			case 2: ipl_name = "flash0:/nandipl_03g.bin"; ipl_key = 1; break;
			default: InstallError(ofw, "Unsupported model.");
		}
	}
	else
	{
		switch (model)
		{
			case 0: ipl_name = "flash0:/nandcipl_01g.bin"; break;
			case 1: ipl_name = "flash0:/nandcipl_02g.bin"; break;
			case 2: ipl_name = "flash0:/nandcipl_03g.bin"; break;
			default: InstallError(ofw, "Unsupported model.");
		}
	}

	size = ReadFile(ipl_name, 0, big_buffer, BIG_BUFFER_SIZE);
	if (size <= 0)
	{
		InstallError(ofw, "Cannot read nandipl\n");
	}
	
	dcPatchModuleString("IoPrivileged", "IoPrivileged", "IoPrivileged");

	if (pspIplUpdateClearIpl() < 0)
		InstallError(ofw, "Error in pspIplUpdateClearIpl");
	
	if (pspIplUpdateSetIpl(big_buffer, size, ipl_key) < 0)
		InstallError(ofw, "Error in pspIplUpdateSetIpl");

	sceKernelDelayThread(900000);

	if (!ofw)
	{
		int file_count = sizeof(common_m33) / sizeof(common_m33[0]);

		const char ** spec = 0;
		int spec_count;
		switch (model)
		{
			case 0:
				spec = spec_01g_m33;
				spec_count = sizeof(spec_01g_m33) / sizeof(spec_01g_m33[0]);
				break;
			case 1:
				spec = spec_02g_m33;
				spec_count = sizeof(spec_02g_m33) / sizeof(spec_02g_m33[0]);
				break;
			case 2:
				spec = spec_03g_m33;
				spec_count = sizeof(spec_03g_m33) / sizeof(spec_03g_m33[0]);
				break;
			default:
				InstallError(ofw, "Unsupported model.");
		}
		file_count += spec_count;

		CopyFileList(ofw, common_m33, sizeof(common_m33) / sizeof(common_m33[0]), 0, file_count);
		CopyFileList(ofw, spec, spec_count, sizeof(common_m33) / sizeof(common_m33[0]), file_count);
		
		DisablePlugins();

		res = ReadFile("flash1:/config.se", 0, sm_buffer1, SMALL_BUFFER_SIZE);
		if (res > 0)
		{
			WriteFile("flach1:/config.se", sm_buffer1, res);		
		}
	}

	SetProgress(100, 1);
	vlfGuiAddEventHandler(0, 600000, OnInstallComplete, &ofw);

	return sceKernelExitDeleteThread(0);
}

int Install(int ofw)
{
	ClearProgress();
	status = vlfGuiAddText(80, 100, "Loading updater modules...");

	progress_bar = vlfGuiAddProgressBar(136);	
	progress_text = vlfGuiAddText(240, 148, "0%");
	vlfGuiSetTextAlignment(progress_text, VLF_ALIGNMENT_CENTER);

	SceUID install_thid = sceKernelCreateThread("install_thread", install_thread, 0x18, 0x10000, 0, NULL);
	if (install_thid >= 0)
	{
		sceKernelStartThread(install_thid, 4, &ofw);
	}

	return VLF_EV_RET_REMOVE_OBJECTS | VLF_EV_RET_REMOVE_HANDLERS;
}

