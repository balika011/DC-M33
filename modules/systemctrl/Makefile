TARGET = systemctrl
OBJS = main.o sysmodpatches.o isofs_driver.o umd9660_driver.o isoread.o csoread.o malloc.o conf.o kubridge.o systemctrl.o init_patch.o rebootex.o rebootex_02g.o rebootex_03g.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1
PSP_FW_VERSION=500

LIBDIR = ../lib
LDFLAGS = -nostartfiles
LIBS = -lpspsysmem_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
