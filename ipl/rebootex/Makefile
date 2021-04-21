TARGET = rebootex
OBJS = crt0.o main.o fat.o ms.o printf.o uart.o syscon.o sysreg.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS = -lc
LIBDIR =
LDFLAGS = -nostartfiles  -T rebootex.x -fdata-sections -ffunction-sections -Wl,--gc-sections

PSPSDK=$(shell psp-config --pspsdk-path)
#include $(PSPSDK)/lib/build.mak
include build.mak
