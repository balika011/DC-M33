	.set noreorder

#include "pspimport.s"

// Build List
// LoadCoreForKernel_0000.o LoadCoreForKernel_0001.o

#ifdef F_LoadCoreForKernel_0000
	IMPORT_START	"LoadCoreForKernel",0x00010011
#endif
#ifdef F_LoadCoreForKernel_0001
	IMPORT_FUNC	"LoadCoreForKernel",0xF6B1BF0F,sceKernelFindModuleByName
#endif

