TARGET = satelite
OBJS = main.o blit.o font.o menu.o videoiso.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_USER_LIBS = 1
USE_PSPSDK_LIBC=1
PSP_FW_VERSION = 500

LIBDIR = ../lib
LIBS = -lpspsystemctrl_user -lpspvshctrl -lpsppaf -lpsppower
LDFLAGS = -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
