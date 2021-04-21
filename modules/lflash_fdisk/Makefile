release: all

TARGET = lflash_fdisk
OBJS = main.o exports.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LDFLAGS = -nostdlib -nodefaultlibs
LIBDIR = ../lib
LIBS = -lpspdisplay_driver -lpspsysreg_driver -lpspsyscon_driver -lpspnand_driver -lpspsystemctrl_kernel -lpsplflash_fatfmt

PSPSDK=$(shell psp-config --pspsdk-path)
include ./build.mak
