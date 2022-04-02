TARGET = flashemu
OBJS = main.o flashemu.o exports.o rebootex.o rebootex_02g.o rebootex_03g.o cptbl.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall -fno-pic
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1
PSP_FW_VERSION=500

LIBDIR = ../lib
LDFLAGS = -nostartfiles
LIBS = -lpspsystemctrl_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
