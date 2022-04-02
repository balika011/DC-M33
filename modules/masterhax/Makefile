TARGET = masterhax
OBJS = main.o imports.o exploit.o

#INCDIR = ../include
CFLAGS = -Os -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c
 
#LIBDIR = ../lib
LIBS = -lpsppower

PSP_FW_VERSION = 271

EXTRA_TARGETS = EBOOT.PBP

BUILD_PRX = 1

PSP_EBOOT_ICON = icon0.png
PSP_EBOOT_TITLE = MasterHax

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
