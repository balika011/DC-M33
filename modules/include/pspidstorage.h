#ifndef PSPIDSTORAGE_H
#define PSPIDSTORAGE_H

#include <pspkerneltypes.h>

#ifdef __cplusplus
extern "C" {
#endif

int sceIdStorageUnformat(void);

int sceIdStorageFormat(void);

int sceIdStorageCreateLeaf(u16 leafid);

int sceIdStorage_driver_99ACCB71(u16 *leaves, int n);
#define sceIdStorageCreateAtomicLeaves sceIdStorage_driver_99ACCB71

int sceIdStorageReadLeaf(u16 leafid, void *buf);

int sceIdStorageWriteLeaf(u16 leafid, void *buf);

int sceIdStorageFlush(void);

#ifdef __cplusplus
}
#endif

#endif
