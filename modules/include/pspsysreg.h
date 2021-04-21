#ifndef PSPSYSREG_H
#define PSPSYSREG_H

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int sceSysregGetTachyonVersion(void);
u64 sceSysregGetFuseId(void);
int sceSysregGetFuseConfig(void);


#ifdef __cplusplus
}
#endif

#endif
