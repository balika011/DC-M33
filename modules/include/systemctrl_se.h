#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

/**
 * These functions are only available in SE-C and later, 
 * and they are not in HEN 
*/

enum 
{
	FAKE_REGION_DISABLED,
	FAKE_REGION_JAPAN,
	FAKE_REGION_AMERICA,
	FAKE_REGION_EUROPE,
	FAKE_REGION_KOREA, 
	FAKE_REGION_UK, 
	FAKE_REGION_MEXICO,
	FAKE_REGION_AUSTRALIA,
	FAKE_REGION_EAST, 
	FAKE_REGION_TAIWAN, /* do not use, may cause brick on restore default settings */
	FAKE_REGION_RUSSIA,
	FAKE_REGION_CHINA, /* do not use, may cause brick on restore default settings */
	FAKE_REGION_DEBUG1,
	FAKE_REGION_DEBUG2,
};

enum SEUmdModes
{
	MODE_UMD = 0,
	MODE_OE_LEGACY = 1,
	MODE_MARCH33 = 2,
	MODE_NP9660 = 3,
};

enum SESpeedUpMsModes
{
	SPEEDUPMS_NEVER = 0,
	SPEEDUPMS_VSH = 1,
	SPEEDUPMS_GAME = 2,
	SPEEDUPMS_POPS = 4,
	SPEEDUPMS_ALWAYS = 7,
};

typedef struct
{
	int magic; /* 0x47434553 */
	int hidecorrupt;
	int skiplogo;
	int umdactivatedplaincheck;
	int gamekernel150;
	int executebootbin;
	int startupprog;
	int umdmode;
	int useisofsonumdinserted;
	int vshcpuspeed;
	int vshbusspeed;
	int umdisocpuspeed;
	int umdisobusspeed;
	int fakeregion;
	int freeumdregion;
	int hardresetHB;
	int usbdevice;
	int novshmenu;
	int usbcharge;
	int notusedaxupd;
	int hidepics;
	int xmbplugins;
	int gameplugins;
	int popsplugins;
	int useversiontxt;
	int speedupms;
	int field_68;
	int field_6C;
	int bootconffileindex;
} SEConfig;


/**
 * Gets the SE/OE version
 *
 * @returns the SE version
 *
 * 3.03 OE-A: 0x00000500
*/
int sctrlSEGetVersion();

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @returns 0 on success
*/
int sctrlSEGetConfig(SEConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(SEConfig *config, int size);

/**
 * Sets the SE configuration
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(SEConfig *config);

/**
 * Sets the SE configuration
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(SEConfig *config, int size);

/**
 * Initiates the emulation of a disc from an ISO9660/CSO file.
 *
 * @param file - The path of the 
 * @param noumd - Wether use noumd or not
 * @param isofs - Wether use the custom SE isofs driver or not
 * 
 * @returns 0 on success
 *
 * @Note - When setting noumd to 1, isofs should also be set to 1,
 * otherwise the umd would be still required.
 *
 * @Note 2 - The function doesn't check if the file is valid or even if it exists
 * and it may return success on those cases
 *
 * @Note 3 - This function is not available in SE for devhook
 * @Example:
 *
 * SEConfig config;
 *
 * sctrlSEGetConfig(&config);
 *
 * if (config.usenoumd)
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * }
 * else
 * {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
 * }
*/
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);

/**
 * Umounts an iso.
 *
 * @returns 0 on success
*/
int sctrlSEUmountUmd(void);

/**
 * Forces the umd disc out state
 *
 * @param out - non-zero for disc out, 0 otherwise
 *
*/
void sctrlSESetDiscOut(int out);

/**
 * Sets the disctype.
 *
 * @param type - the disctype (0x10=game, 0x20=video, 0x40=audio)
*/
void sctrlSESetDiscType(int type);

/**
 * Sets the current umd file (kernel only)
*/
char *sctrlSEGetUmdFile();

/**
 * Sets the current umd file (kernel only)
 *
 * @param file - The umd file
*/
void sctrlSESetUmdFile(char *file);

/** 
 * Sets the boot config file for next reboot (kernel only)
 *
 * @param index - The index identifying the file (0 -> normal bootconf, 1 -> march33 driver bootconf, 2 -> np9660 bootcnf)
*/
void sctrlSESetBootConfFileIndex(int index);

#endif
