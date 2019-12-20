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
CFLAGS ?= -Wall -c -std=gnu99
CFLAGS_I686 ?= -Wall -c -m32
LFLAGS ?= \
	-Wall \
	../../opensea-operations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../opensea-transport/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../opensea-common/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a
TCGLFLAGS ?= \
	-Wall \
	../../SeaCTCGOperations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCGOperations.a \
	../../SeaCTCG/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libSeaCTCG.a \
	../../opensea-operations/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-operations.a \
	../../opensea-transport/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-transport.a \
	../../opensea-common/Make/gcc/$(LIB_FILE_OUTPUT_DIR)/libopensea-common.a
INC_DIR?= \
	-I../../opensea-common/include \
	-I../../opensea-transport/include \
	-I../../opensea-transport/include/vendor \
	-I../../include \
	-I../../opensea-operations/include

UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
	LFLAGS += -lm -lrt
	TCGLFLAGS += -lm -lrt
endif
ifeq ($(UNAME),SunOS)
	LFLAGS += 
	TCGLFLAGS += 
endif
ifeq ($(UNAME),FreeBSD)
	LFLAGS += -lm -lcam
	TCGLFLAGS += -lm -lcam
endif

MACHINE := $(shell uname -m)

FILE_OUTPUT_DIR=openseachest_exes

ifeq ($(MACHINE), x86_64)

endif

#Files for the final binary

NVMEOUTFILE = openSeaChest_NVMe
NVMESOURCES = $(UTIL_SRC_DIR)/openSeaChest_NVMe.c ../../src/EULA.c ../../src/openseachest_util_options.c
NVMEOBJS = $(NVMESOURCES:.c=.o)

ERASEOUTFILE = openSeaChest_Erase
ERASESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Erase.c ../../src/EULA.c ../../src/openseachest_util_options.c
ERASEOBJS = $(ERASESOURCES:.c=.o)

SMARTOUTFILE = openSeaChest_SMART
SMARTSOURCES = $(UTIL_SRC_DIR)/openSeaChest_SMART.c ../../src/EULA.c ../../src/openseachest_util_options.c
SMARTOBJS = $(SMARTSOURCES:.c=.o)

POWERCONTROLOUTFILE = openSeaChest_PowerControl
POWERCONTROLSOURCES = $(UTIL_SRC_DIR)/openSeaChest_PowerControl.c ../../src/EULA.c ../../src/openseachest_util_options.c
POWERCONTROLOBJS = $(POWERCONTROLSOURCES:.c=.o)

GENERICTESTSOUTFILE = openSeaChest_GenericTests
GENERICTESTSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_GenericTests.c ../../src/EULA.c ../../src/openseachest_util_options.c
GENERICTESTSOBJS = $(GENERICTESTSSOURCES:.c=.o)

BASICSOUTFILE = openSeaChest_Basics
BASICSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Basics.c ../../src/EULA.c ../../src/openseachest_util_options.c
BASICSOBJS = $(BASICSSOURCES:.c=.o)

CONFIGUREOUTFILE = openSeaChest_Configure
CONFIGURESOURCES = $(UTIL_SRC_DIR)/openSeaChest_Configure.c ../../src/EULA.c ../../src/openseachest_util_options.c
CONFIGUREOBJS = $(CONFIGURESOURCES:.c=.o)

INFOOUTFILE = openSeaChest_Info
INFOSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Info.c ../../src/EULA.c ../../src/openseachest_util_options.c
INFOOBJS = $(INFOSOURCES:.c=.o)

ZBDOUTFILE = openSeaChest_ZBD
ZBDSOURCES = $(UTIL_SRC_DIR)/openSeaChest_ZBD.c ../../src/EULA.c ../../src/openseachest_util_options.c
ZBDOBJS = $(ZBDSOURCES:.c=.o)

FORMATOUTFILE = openSeaChest_FormatUnit
FORMATSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Format.c ../../src/EULA.c ../../src/openseachest_util_options.c
FORMATOBJS = $(FORMATSOURCES:.c=.o)

LOGSOUTFILE = openSeaChest_Logs
LOGSSOURCES = $(UTIL_SRC_DIR)/openSeaChest_Logs.c ../../src/EULA.c ../../src/openseachest_util_options.c
LOGSOBJS = $(LOGSSOURCES:.c=.o)

PASSTHROUGHTESTOUTFILE = openSeaChest_PassthroughTest
PASSTHROUGHTESTSOURCES = $(UTIL_SRC_DIR)/openSeaChest_PassthroughTest.c ../../src/EULA.c ../../src/openseachest_util_options.c
PASSTHROUGHTESTOBJS = $(PASSTHROUGHTESTSOURCES:.c=.o)

FIRMWAREOUTFILE = openSeaChest_Firmware

#add any defines needed for tool release.
#PROJECT_DEFINES += -DDISABLE_NVME_PASSTHROUGH -DDISABLE_TCG_SUPPORT
PROJECT_DEFINES += -DDISABLE_TCG_SUPPORT
ifeq ($(UNAME),FreeBSD)
PROJECT_DEFINES += -DDISABLE_NVME_PASSTHROUGH
endif

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
debug: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_dbg
debug: INFOOUTFILE := $(INFOOUTFILE)_dbg
debug: ZBDOUTFILE := $(ZBDOUTFILE)_dbg
debug: FORMATOUTFILE := $(FORMATOUTFILE)_dbg
debug: NVMEOUTFILE := $(NVMEOUTFILE)_dbg
debug: LOGSOUTFILE := $(LOGSOUTFILE)_dbg
debug: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_dbg
debug: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE)

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
static-debug: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_static_dbg
static-debug: INFOOUTFILE := $(INFOOUTFILE)_static_dbg
static-debug: ZBDOUTFILE := $(ZBDOUTFILE)_static_dbg
static-debug: FORMATOUTFILE := $(FORMATOUTFILE)_static_dbg
static-debug: NVMEOUTFILE := $(NVMEOUTFILE)_static_dbg
static-debug: LOGSOUTFILE := $(LOGSOUTFILE)_static_dbg
static-debug: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_static_dbg
static-debug: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE)

release: CXXFLAGS += -O3
release: CFLAGS += -O3
release: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(NVMEOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE)

static-release: LFLAGS += -static
static-release: TCGLFLAGS += -static
static-release: ERASEOUTFILE := $(ERASEOUTFILE)_static
static-release: SMARTOUTFILE := $(SMARTOUTFILE)_static
static-release: POWERCONTROLOUTFILE := $(POWERCONTROLOUTFILE)_static
static-release: GENERICTESTSOUTFILE := $(GENERICTESTSOUTFILE)_static
static-release: BASICSOUTFILE := $(BASICSOUTFILE)_static
static-release: CONFIGUREOUTFILE := $(CONFIGUREOUTFILE)_static
static-release: INFOOUTFILE := $(INFOOUTFILE)_static
static-release: ZBDOUTFILE := $(ZBDOUTFILE)_static
static-release: FORMATOUTFILE := $(FORMATOUTFILE)_static
static-release: NVMEOUTFILE := $(NVMEOUTFILE)_static
static-release: LOGSOUTFILE := $(LOGSOUTFILE)_static
static-release: PASSTHROUGHTESTOUTFILE := $(PASSTHROUGHTESTOUTFILE)_static
static-release: CXXFLAGS += -O3
static-release: CFLAGS += -O3
static-release: mkoutputdir $(ERASEOUTFILE) $(SMARTOUTFILE) $(POWERCONTROLOUTFILE) $(GENERICTESTSOUTFILE) $(BASICSOUTFILE) $(SECURITYOUTFILE) $(NVMEOUTFILE) $(CONFIGUREOUTFILE) $(FIRMWAREOUTFILE) $(INFOOUTFILE) $(ZBDOUTFILE) $(FORMATOUTFILE) $(LOGSOUTFILE) $(PASSTHROUGHTESTOUTFILE)

export CFLAGS
export CXXFLAGS
export PROJECT_DEFINES

opensea-libs:
	$(MAKE) -C ../../opensea-common/Make/gcc
	$(MAKE) -C ../../opensea-transport/Make/gcc
	$(MAKE) -C ../../opensea-operations/Make/gcc

$(SMARTOUTFILE): $(SMARTOBJS) opensea-libs mkoutputdir
	$(CC) $(SMARTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)

$(GENERICTESTSOUTFILE) : $(GENERICTESTSOBJS) opensea-libs mkoutputdir
	$(CC) $(GENERICTESTSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)

$(POWERCONTROLOUTFILE) : $(POWERCONTROLOBJS) opensea-libs mkoutputdir
	$(CC) $(POWERCONTROLOBJS) -o $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE) $(LFLAGS)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE)

$(OUTFILE): $(OBJS) opensea-libs mkoutputdir
	$(CC) $(OBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(OUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(OUTFILE)

$(BASICSOUTFILE): $(BASICSOBJS) opensea-libs mkoutputdir
	$(CC) $(BASICSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(BASICSOUTFILE)

$(CONFIGUREOUTFILE): $(CONFIGUREOBJS) opensea-libs mkoutputdir
	$(CC) $(CONFIGUREOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(CONFIGUREOUTFILE)

$(FIRMWAREOUTFILE):
	$(MAKE) -f Makefile.openSeaChest_firmware $(MAKECMDGOALS)

$(INFOOUTFILE): $(INFOOBJS) opensea-libs mkoutputdir
	$(CC) $(INFOOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)

$(ZBDOUTFILE): $(ZBDOBJS) opensea-libs mkoutputdir
	$(CC) $(ZBDOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)

$(FORMATOUTFILE): $(FORMATOBJS) opensea-libs mkoutputdir
	$(CC) $(FORMATOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)

$(NVMEOUTFILE): $(NVMEOBJS) opensea-libs mkoutputdir
	$(CC) $(NVMEOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)

$(LOGSOUTFILE): $(LOGSOBJS) opensea-libs mkoutputdir
	$(CC) $(LOGSOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(LOGSOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(LOGSOUTFILE)

$(PASSTHROUGHTESTOUTFILE): $(PASSTHROUGHTESTOBJS) opensea-libs mkoutputdir
	$(CC) $(PASSTHROUGHTESTOBJS) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(PASSTHROUGHTESTOUTFILE)
#	$(STRIP) -s $(FILE_OUTPUT_DIR)/$(PASSTHROUGHTESTOUTFILE)

$(ERASEOUTFILE): $(ERASEOBJS) opensea-libs mkoutputdir
    ifneq (,$(findstring DISABLE_TCG_SUPPORT,$(PROJECT_DEFINES)))
		$(CC) $(ERASEOBJS) $(PROJECT_DEFINES) $(LFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
#	    $(STRIP) -s $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
    else
		$(CC) $(ERASEOBJS) $(TCGLFLAGS) -o $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
#	    $(STRIP) -s $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)
    endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC_DIR) $(PROJECT_DEFINES) $< -o $@

clean:
	rm -f *.o *.a $(FILE_OUTPUT_DIR)/$(ERASEOUTFILE)* $(FILE_OUTPUT_DIR)/$(SMARTOUTFILE)* $(FILE_OUTPUT_DIR)/$(POWERCONTROLOUTFILE)* $(FILE_OUTPUT_DIR)/$(GENERICTESTSOUTFILE)* $(FILE_OUTPUT_DIR)/stripped* $(FILE_OUTPUT_DIR)/$(FIRMWAREOUTFILE)* $(FILE_OUTPUT_DIR)/$(INFOOUTFILE)* $(FILE_OUTPUT_DIR)/$(ZBDOUTFILE)* $(FILE_OUTPUT_DIR)/$(FORMATOUTFILE)* $(FILE_OUTPUT_DIR)/$(NVMEOUTFILE)*
	rm -f ../../src/*.o
	rm -f $(UTIL_SRC_DIR)/*.o
	rm -rf $(FILE_OUTPUT_DIR)
	$(MAKE) -f Makefile.openSeaChest_firmware clean

clean_all: clean
	rm -f *.o *.a
	$(MAKE) -C ../../opensea-common/Make/gcc clean
	$(MAKE) -C ../../opensea-transport/Make/gcc clean
	$(MAKE) -C ../../opensea-operations/Make/gcc clean
	$(MAKE) -f Makefile.openSeaChest_firmware clean_all

mkoutputdir:
	mkdir -p $(FILE_OUTPUT_DIR)
