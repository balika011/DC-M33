# PSP Software Development Kit - http://www.pspdev.org
# -----------------------------------------------------------------------
# Licensed under the BSD license, see LICENSE in PSPSDK root for details.
#
# build.mak - Base makefile for projects using PSPSDK.
#
# Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
# Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
# Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
#
# $Id: build.mak 355 2005-06-27 07:38:48Z mrbrown $

# Note: The PSPSDK make variable must be defined before this file is included.
ifeq ($(PSPSDK),)
$(error $$(PSPSDK) is undefined.  Use "PSPSDK := $$(shell psp-config --pspsdk-path)" in your Makefile)
endif

CC       = psp-gcc
CXX      = psp-g++
AS       = psp-gcc
#LD       = psp-gcc
LD       = psp-ld
AR       = psp-ar
RANLIB   = psp-ranlib
STRIP    = psp-strip

# Add in PSPSDK includes and libraries.
INCDIR   := $(INCDIR) . $(PSPSDK)/include
LIBDIR   := $(LIBDIR) . $(PSPSDK)/lib

CFLAGS   := $(addprefix -I,$(INCDIR)) $(CFLAGS)
CXXFLAGS := $(CFLAGS) $(CXXFLAGS)
ASFLAGS  := $(CFLAGS) $(ASFLAGS)

LDFLAGS  := $(addprefix -L,$(LIBDIR)) $(LDFLAGS)

# Library selection.  By default we link with PSPSDK's libc.  Allow the
# user to link with Newlib's libc if USE_NEWLIB_LIBC is set to 1.
#PSPSDK_LIBC_LIB = -lpsplibc
#ifeq ($(USE_NEWLIB_LIBC),1)
#PSPSDK_LIBC_LIB = -lc -lpspglue
#endif

# Link with following default libraries.  Other libraries should be specified in the $(LIBS) variable.
# TODO: This library list needs to be generated at configure time.
#PSPSDK_LIBS = -lpspdebug
#LIBS     := $(LIBS) $(PSPSDK_LIBS) $(PSPSDK_LIBC_LIB) -lpspkernel
#
ifneq ($(TARGET_LIB),)
FINAL_TARGET = $(TARGET_LIB)
else
FINAL_TARGET = $(TARGET).bin
endif

all: $(EXTRA_TARGETS) $(FINAL_TARGET)

$(TARGET).bin: $(TARGET).elf
	$(STRIP) -s -O binary $(TARGET).elf -o $(TARGET).bin
	-rm -f $(TARGET).elf

$(TARGET).elf: $(OBJS)
	-rm -f $(TARGET).elf
	$(LINK.c) $^ $(LIBS) -o $(TARGET).elf

$(TARGET_LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)

clean: $(EXTRA_CLEAN)
	-rm -f $(FINAL_TARGET) *.o $(PSP_EBOOT_SFO) $(PSP_EBOOT)
