#
# Do NOT modify or remove this copyright and license
#
# Copyright (c) 2012 - 2019 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
#
# This software is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ******************************************************************************************

# Hand Written Makefile (Edit with caution) -Muhammad
#

UTIL_SRC_DIR=../../utils/C/openSeaChest
CC ?= gcc
LIB_FILE_OUTPUT_DIR=lib
STRIP ?= strip
STRIPOPTS ?=
#start with only -Wall and -Wextra. More warnings added below based on version of clang/gcc
CFLAGS ?= -Wall -Wextra -c

CFLAGS_I686 ?= -Wall -Wextra -c -m32
LFLAGS ?= \
	-Wall \
	../../subprojects/opensea-operations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../subprojects/opensea-transport/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../subprojects/opensea-common/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a \
	../../subprojects/wingetopt/lib/libwingetopt.a
TCGLFLAGS ?= \
	-Wall \
	../../subprojects/SeaCTCGOperations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCGOperations.a \
	../../subprojects/SeaCTCG/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCG.a \
	../../subprojects/opensea-operations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../subprojects/opensea-transport/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../subprojects/opensea-common/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a \
	../../subprojects/wingetopt/lib/libwingetopt.a
INC_DIR ?= \
	-I../../subprojects/opensea-common/include \
	-I../../subprojects/opensea-transport/include \
	-I../../subprojects/opensea-transport/include/vendor \
	-I../../include \
	-I../../subprojects/opensea-operations/include \
	-I../../subprojects/wingetopt/src

UNAME := $(shell uname -s)

#If C_INCLUDE_PATH is not set, set it to /usr/include/
C_INCLUDE_PATH := /usr/include/

#NOTE -Wsign-conversion can be useful but warns way too much by default. Only enable it if debugging specific problems
COMPILER_VERSION := $(shell $(CC) --version)
ifneq (,$(findstring clang,$(COMPILER_VERSION)))
	#setup clang specific warnings/flags (centos 7's old version supports -Wno-unknown-warning-option so enabling all of them)
	CFLAGS += -Wno-unknown-warning-option -Wcast-align=strict -Wvla -Wfloat-equal -Wnull-dereference -Wunused-const-variable \
	-Wduplicated-cond -Wjump-misses-init -Wstringop-overflow -Wlogical-op -Wshift-overflow=2 -Wdouble-promotion -Wformat-security \
	-Wold-style-definition -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes
else 
	ifneq (,$(findstring GCC,$(COMPILER_VERSION)))
		#setup gcc specific warnings/flags
		GCC_VERSION_STRING = $(shell $(CC) -dumpversion)
		GCC_VER = $(subst ., ,$(GCC_VERSION_STRING))
		GCC_MAJOR = $(word 1,$(GCC_VER))
		GCC_MINOR = $(word 2,$(GCC_VER))
		GCC_SUBMINOR = $(word 3,$(GCC_VER))
		ifeq ($(GCC_MINOR),)
			GCC_MINOR = 0
		endif 
		ifeq ($(GCC_SUBMINOR),)
			GCC_SUBMINOR = 0
		endif
		#version 8.5 and higher
		ifeq ($(shell test $(GCC_MAJOR) -gt 7; echo $$?),0)
			ifeq ($(shell test $(GCC_MINOR) -gt 4; echo $$?),0)
				CFLAGS += -Wcast-align=strict
			else 
				CFLAGS += -Wcast-align
			endif
		else
			CFLAGS += -Wcast-align
		endif
		#version 7.5 and higher
		ifeq ($(shell test $(GCC_MAJOR) -gt 6; echo $$?),0)
			ifeq ($(shell test $(GCC_MINOR) -gt 4; echo $$?),0)
				CFLAGS += -Wshadow=compatible-local -Wstringop-overflow
			else 
				CFLAGS +=
			endif
		else
			CFLAGS +=
		endif
		#version 6.5 and higher
		ifeq ($(shell test $(GCC_MAJOR) -gt 5; echo $$?),0)
			ifeq ($(shell test $(GCC_MINOR) -gt 4; echo $$?),0)
				CFLAGS += -Wnull-dereference -Wunused-const-variable -Wduplicated-cond -Wshift-overflow=2
			else 
				CFLAGS +=
			endif
		else
			CFLAGS +=
		endif
		#version 5.5 and higher
		ifeq ($(shell test $(GCC_MAJOR) -gt 4; echo $$?),0)
			ifeq ($(shell test $(GCC_MINOR) -gt 4; echo $$?),0)
				CFLAGS += -Wlogical-not-parentheses
			endif
		else
			#GCC less that v 5.x.x needs to set gnu99 standard
			#as of 5.x.x, gnu11 is default
			CFLAGS += -std=gnu99
		endif
		
		CFLAGS += -Wvla -Wfloat-equal -Wjump-misses-init -Wlogical-op -Wdouble-promotion -Wformat-security \
			-Wold-style-definition -Wstrict-prototypes -Wmissing-declarations -Wmissing-prototypes
	endif
endif

ifeq ($(UNAME),Linux)
	LFLAGS += -lm -lrt -Wl,--gc-sections 
	TCGLFLAGS += -lm -lrt -Wl,--gc-sections 
	STRIPOPTS= -s
	CFLAGS += -ffunction-sections -fdata-sections
endif
ifeq ($(UNAME),SunOS)
	LFLAGS += -lm
	TCGLFLAGS += -lm
	STRIPOPTS=
endif
ifeq ($(UNAME),FreeBSD)
	LFLAGS += -lm -lcam -Wl,--gc-sections 
	TCGLFLAGS += -lm -lcam -Wl,--gc-sections 
	STRIPOPTS= -s
	CFLAGS += -ffunction-sections -fdata-sections
endif
ifeq ($(UNAME),AIX)
	LFLAGS += -lm -lodm -lcfg
	TCGLFLAGS += -lm -lodm -lcfg
	STRIPOPTS=
	CFLAGS +=
endif


MACHINE := $(shell uname -m)

FILE_OUTPUT_DIR=openseachest_exes

ifeq ($(MACHINE), x86_64)

endif

#Files for the final binary
EULA_SRC = ../../src/EULA.c
UTIL_OPTS_SRC = ../../src/openseachest_util_options.c

NVMEOUTFILE = openSeaChest_NVMe
NVMESOURCES = $(UTIL_SRC_DIR)/openSeaChest_NVMe.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
NVMEOBJS = $(NVMESOURCES:.c=.o)

ERASEOUTFILE = openSeaChest_Erase
ERASESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Erase.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
ERASEOBJS = $(ERASESOURCES:.c=.o)

SMARTOUTFILE = openSeaChest_SMART
SMARTSOURCES = $(UTIL_SRC_DIR)/openSeaChest_SMART.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
SMARTOBJS = $(SMARTSOURCES:.c=.o)

POWERCONTROLOUTFILE = openSeaChest_PowerControl
POWERCONTROLSOURCES = $(UTIL_SRC_DIR)/openSeaChest_PowerControl.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
POWERCONTROLOBJS = $(POWERCONTROLSOURCES:.c=.o)

GENERICTESTSOUTFILE = openSeaChest_GenericTests
GENERICTESTSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_GenericTests.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
GENERICTESTSOBJS = $(GENERICTESTSSOURCES:.c=.o)

BASICSOUTFILE = openSeaChest_Basics
BASICSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Basics.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
BASICSOBJS = $(BASICSSOURCES:.c=.o)

SECURITYOUTFILE = openSeaChest_Security
SECURITYSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Security.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
SECURITYOBJS = $(SECURITYSOURCES:.c=.o)

CONFIGUREOUTFILE = openSeaChest_Configure
CONFIGURESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Configure.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
CONFIGUREOBJS = $(CONFIGURESOURCES:.c=.o)

INFOOUTFILE = openSeaChest_Info
INFOSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Info.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
INFOOBJS = $(INFOSOURCES:.c=.o)

ZBDOUTFILE = openSeaChest_ZBD
ZBDSOURCES = $(UTIL_SRC_DIR)/openSeaChest_ZBD.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
ZBDOBJS = $(ZBDSOURCES:.c=.o)

FORMATOUTFILE = openSeaChest_FormatUnit
FORMATSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Format.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
FORMATOBJS = $(FORMATSOURCES:.c=.o)

LOGSOUTFILE = openSeaChest_Logs
LOGSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Logs.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
LOGSOBJS = $(LOGSSOURCES:.c=.o)

PASSTHROUGHTESTOUTFILE = openSeaChest_PassthroughTest
PASSTHROUGHTESTSOURCES = $(UTIL_SRC_DIR)/openSeaChest_PassthroughTest.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
PASSTHROUGHTESTOBJS = $(PASSTHROUGHTESTSOURCES:.c=.o)

RESERVATIONSOUTFILE = openSeaChest_Reservations
RESERVATIONSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Reservations.c $(EULA_SRC) $(UTIL_OPTS_SRC) 
RESERVATIONSOBJS = $(RESERVATIONSSOURCES:.c=.o)

FIRMWAREOUTFILE = openSeaChest_Firmware
FIRMWARESOURCES = \
	$(UTIL_SRC_DIR)/openSeaChest_Firmware.c \
	$(EULA_SRC) \
	$(UTIL_OPTS_SRC) \
	
FIRMWAREOBJS = $(FIRMWARESOURCES:.c=.o)

USING_MUSL=0
#add any defines needed for tool release.
#PROJECT_DEFINES += -DDISABLE_TCG_SUPPORT
#Linux: If using a different libc, you will likely get a compile warning.
#       You can add a -DUSING_LIBNAME_LIBC here to specify it.
#       Currently, -DUSING_MUSL_LIBC is supported to show the MIT license for it.
#       Calling make as "make release USING_MUSL=1 will enable musl libc license quickly and easily
PROJECT_DEFINES += -DDISABLE_TCG_SUPPORT -DUSING_MUSL_LIBC=$(USING_MUSL)


.PHONY: clean_all all

all: clean_all debug

debug: CXXFLAGS += -g
debug: CFLAGS += -g
debug: PROJECT_DEFINES += -D_DEBUG
debug: ERASEOUTFILE := $(ERASEOUTFILE)_dbg
debug: SMARTOUTFILE := $(SMARTOUTFILE)_dbg
debug: POWERCONTROLOUTFILE := $(POWERCONTROLOUTFILE)_dbg
debug: GENERICTESTSOUTFILE := $(GENERICTESTSOUTFILE)_dbg
debug: BASICSOUTFILE := $(BASICSOUTFILE)_dbg
debug: SECURITYOUTFILE := $(SECURITYOUTFILE)_dbg
debug: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_dbg
debug: INFOOUTFILE := $(INFOOUTFILE)_dbg
debug: ZBDOUTFILE := $(ZBDOUTFILE)_dbg
debug: FORMATOUTFILE := $(FORMATOUTFILE)_dbg
debug: NVMEOUTFILE := $(NVMEOUTFILE)_dbg
debug: LOGSOUTFILE := $(LOGSOUTFILE)_dbg
debug: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_dbg
debug: RESERVATIONSOUTFILE := $(RESERVATIONSOUTFILE)_dbg
debug: FIRMWAREOUTFILE := $(FIRMWAREOUTFILE)_dbg
debug: mkoutputdir wingetopt opensea-libs $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE) $(RESERVATIONSOUTFILE)

static-debug: CXXFLAGS += -g
static-debug: CFLAGS += -g
static-debug: LFLAGS += -static
static-debug: TCGLFLAGS += -static
static-debug: PROJECT_DEFINES += -D_DEBUG
static-debug: ERASEOUTFILE := $(ERASEOUTFILE)_static_dbg
static-debug: SMARTOUTFILE := $(SMARTOUTFILE)_static_dbg
static-debug: POWERCONTROLOUTFILE := $(POWERCONTROLOUTFILE)_static_dbg
static-debug: GENERICTESTSOUTFILE := $(GENERICTESTSOUTFILE)_static_dbg
static-debug: BASICSOUTFILE := $(BASICSOUTFILE)_static_dbg
static-debug: SECURITYOUTFILE := $(SECURITYOUTFILE)_static_dbg
static-debug: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_static_dbg
static-debug: INFOOUTFILE := $(INFOOUTFILE)_static_dbg
static-debug: ZBDOUTFILE := $(ZBDOUTFILE)_static_dbg
static-debug: FORMATOUTFILE := $(FORMATOUTFILE)_static_dbg
static-debug: NVMEOUTFILE := $(NVMEOUTFILE)_static_dbg
static-debug: LOGSOUTFILE := $(LOGSOUTFILE)_static_dbg
static-debug: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_static_dbg
static-debug: RESERVATIONSOUTFILE := $(RESERVATIONSOUTFILE)_static_dbg
static-debug: FIRMWAREOUTFILE := $(FIRMWAREOUTFILE)_static_dbg
static-debug: mkoutputdir wingetopt opensea-libs $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE) $(RESERVATIONSOUTFILE)

release: CXXFLAGS += -O3
release: CFLAGS += -O3
release: mkoutputdir wingetopt opensea-libs $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE) $(RESERVATIONSOUTFILE)

static-release: LFLAGS += -static
static-release: TCGLFLAGS += -static
static-release: ERASEOUTFILE := $(ERASEOUTFILE)_static
static-release: SMARTOUTFILE := $(SMARTOUTFILE)_static
static-release: POWERCONTROLOUTFILE := $(POWERCONTROLOUTFILE)_static
static-release: GENERICTESTSOUTFILE := $(GENERICTESTSOUTFILE)_static
static-release: BASICSOUTFILE := $(BASICSOUTFILE)_static
static-release: SECURITYOUTFILE := $(SECURITYOUTFILE)_static
static-release: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_static
static-release: INFOOUTFILE := $(INFOOUTFILE)_static
static-release: ZBDOUTFILE := $(ZBDOUTFILE)_static
static-release: FORMATOUTFILE := $(FORMATOUTFILE)_static
static-release: NVMEOUTFILE := $(NVMEOUTFILE)_static
static-release: LOGSOUTFILE := $(LOGSOUTFILE)_static
static-release: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_static
static-release: RESERVATIONSOUTFILE := $(RESERVATIONSOUTFILE)_static
static-release: FIRMWAREOUTFILE := $(FIRMWAREOUTFILE)_static
static-release: CXXFLAGS += -O3
static-release: CFLAGS += -O3
static-release: mkoutputdir wingetopt opensea-libs $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE) $(RESERVATIONSOUTFILE)

export CFLAGS
export CXXFLAGS
export PROJECT_DEFINES

#wingetopt has a GNUMakefile that should be automatically found.
#it has CMake and meson support too, which are preferred
wingetopt:
	$(MAKE) -C ../../subprojects/wingetopt

opensea-libs:
	$(MAKE) $(MAKEFLAG) -C ../../subprojects/opensea-common/Make/gcc
	$(MAKE) $(MAKEFLAG) -C ../../subprojects/opensea-transport/Make/gcc
	$(MAKE) $(MAKEFLAG) -C ../../subprojects/opensea-operations/Make/gcc

$(SMARTOUTFILE): $(SMARTOBJS)
	$(CC) $(SMARTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)

$(GENERICTESTSOUTFILE) : $(GENERICTESTSOBJS)
	$(CC) $(GENERICTESTSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)

$(POWERCONTROLOUTFILE) : $(POWERCONTROLOBJS)
	$(CC) $(POWERCONTROLOBJS) -o $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE) $(LFLAGS)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE)

$(OUTFILE): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(OUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(OUTFILE)

$(BASICSOUTFILE): $(BASICSOBJS)
	$(CC) $(BASICSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)

$(SECURITYOUTFILE): $(SECURITYOBJS) 
    ifneq (,$(findstring DISABLE_TCG_SUPPORT,$(PROJECT_DEFINES)))
		$(CC) $(SECURITYOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(SECURITYOUTFILE)
#		$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(SECURITYOUTFILE)
    else
		$(CC) $(SECURITYOBJS) $(TCGLFLAGS) -o $(FILE_OUTPUT_DIR)/$(SECURITYOUTFILE)
#	    $(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(SECURITYOUTFILE)
    endif


$(CONFIGUREOUTFILE): $(CONFIGUREOBJS)
	$(CC) $(CONFIGUREOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)

$(FIRMWAREOUTFILE): $(FIRMWAREOBJS)
	$(CC) $(FIRMWAREOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(FIRMWAREOUTFILE)

$(INFOOUTFILE): $(INFOOBJS)
	$(CC) $(INFOOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)

$(ZBDOUTFILE): $(ZBDOBJS)
	$(CC) $(ZBDOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)

$(FORMATOUTFILE): $(FORMATOBJS)
	$(CC) $(FORMATOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)

$(NVMEOUTFILE): $(NVMEOBJS)
	$(CC) $(NVMEOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)

$(LOGSOUTFILE): $(LOGSOBJS)
	$(CC) $(LOGSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(LOGSOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(LOGSOUTFILE)

$(PASSTHROUGHTESTOUTFILE): $(PASSTHROUGHTESTOBJS)
	$(CC) $(PASSTHROUGHTESTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(PASSTHROUGHTESTOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(PASSTHROUGHTESTOUTFILE)

$(RESERVATIONSOUTFILE): $(RESERVATIONSOBJS)
	$(CC) $(RESERVATIONSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(RESERVATIONSOUTFILE)
#	$(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(RESERVATIONSOUTFILE)


$(ERASEOUTFILE): $(ERASEOBJS)
    ifneq (,$(findstring DISABLE_TCG_SUPPORT,$(PROJECT_DEFINES)))
		$(CC) $(ERASEOBJS) $(PROJECT_DEFINES) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
#	    $(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
    else
		$(CC) $(ERASEOBJS) $(TCGLFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
#	    $(STRIP) $(STRIPOPTS) $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
    endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIR) $(PROJECT_DEFINES) $< -o $@

clean:
	rm -f *.o *.a $(FILE_OUTPUT_DIR)/openSeaChest*
	rm -f ../../src/*.o
	rm -f $(UTIL_SRC_DIR)/*.o
	rm -rf $(FILE_OUTPUT_DIR)

clean_all: clean
	rm -f *.o *.a
	$(MAKE) -C ../../subprojects/opensea-common/Make/gcc clean
	$(MAKE) -C ../../subprojects/opensea-transport/Make/gcc clean
	$(MAKE) -C ../../subprojects/opensea-operations/Make/gcc clean

mkoutputdir:
	mkdir -p $(FILE_OUTPUT_DIR)
