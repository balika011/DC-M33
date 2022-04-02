release: all

TARGET = resurrection
OBJS = crt0.o main.o install.o nandoperations.o flash_format.o idstools.o DcManager.o pspDecrypt.o pspIplUpdate.o idsRegeneration.o 

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division 
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS) -c

LIBDIR = ../lib
LDFLAGS = -nostdlib -nodefaultlibs
LIBS = -lpsppower -lpsprtc -lvlfgui -lvlfgu -lvlfutils -lvlflibc -lpspreg -lpspsystemctrl_user -lpspkubridge -lpspwlan

PSP_FW_VERSION = 661

BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include ./build.mak

