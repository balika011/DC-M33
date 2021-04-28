TARGET = ipl_stage1_payload
OBJS = crt0.o main.o printf.o uart.o syscon.o sysreg.o payload.o kirk.o

INCDIR = 
CFLAGS = -Os -G0 -Wall -fno-pic $(BFLAGS) -fdata-sections -ffunction-sections -Wl,--gc-sections
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti 
ASFLAGS = $(CFLAGS)

LIBS = -lc
LIBDIR =
LDFLAGS = -nostartfiles  -T linkfile.l -fdata-sections -ffunction-sections -Wl,--gc-sections

PSPSDK=$(shell psp-config --pspsdk-path)
#include $(PSPSDK)/lib/build.mak
include build.mak
