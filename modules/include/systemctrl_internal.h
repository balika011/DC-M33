#ifndef __SYSTEMCTRL_INTERNAL_H__
#define __SYSTEMCTRL_INTERNAL_H__

#include <systemctrl.h>
#include <bootinfo.h>

typedef int (* KDEC_HANDLER)(u32 *buf, int size, int *retSize, int m);
typedef int (* MDEC_HANDLER)(u32 *tag, u8 *keys, u32 code, u32 *buf, int size, int *retSize, int m, void *unk0, int unk1, int unk2, int unk3, int unk4);
typedef int (* LLE_HANDLER)(SceLibraryStubTable *import); 

KDEC_HANDLER sctrlHENRegisterKDecryptHandler(KDEC_HANDLER handler);
MDEC_HANDLER sctrlHENRegisterMDecryptHandler(MDEC_HANDLER handler);
void sctrlHENRegisterLLEHandler(LLE_HANDLER handler);

#endif

