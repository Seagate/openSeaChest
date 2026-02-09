# SPDX-License-Identifier: MPL-2.0
#
# Do NOT modify or remove this copyright and license
#
# Copyright (c) 2026-2026 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
#
# This software is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ******************************************************************************************
#
# \file tools.mk
# \brief openSeaChest utility builds (17 tools)

#===============================================================================
# Common Tool Sources
#===============================================================================

# Shared across all tools
TOOL_COMMON_SOURCES := \
    EULA.c \
    openseachest_util_options.c

#===============================================================================
# Tool List (Platform-Conditional)
#===============================================================================

# All tools except openSeaChest_NVMe (Windows-only)
TOOLS := \
    openSeaChest_Basics \
    openSeaChest_Configure \
    openSeaChest_Defect \
    openSeaChest_Erase \
    openSeaChest_Firmware \
    openSeaChest_Format \
    openSeaChest_GenericTests \
    openSeaChest_Info \
    openSeaChest_Logs \
    openSeaChest_PassthroughTest \
    openSeaChest_PowerControl \
    openSeaChest_Raw \
    openSeaChest_Reservations \
    openSeaChest_Security \
    openSeaChest_SMART \
    openSeaChest_ZBD

# Add openSeaChest_NVMe on Windows only
ifdef ENABLE_OPENSEACHEST_NVME
    TOOLS += openSeaChest_NVMe
endif

# Tool executables with platform extension
TOOL_EXES := $(addsuffix $(EXE_EXT),$(addprefix $(BIN_DIR)/,$(TOOLS)))

# Support FAT and other 2-second resolution filesystems
# This prevents unnecessary rebuilds on old NFS or FAT filesystems
.LOW_RESOLUTION_TIME: $(TOOL_EXES)

#===============================================================================
# Tool-Specific Source Files
#===============================================================================

# Each tool has its own main source file
BASICS_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Basics.c
CONFIGURE_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Configure.c
DEFECT_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Defect.c
ERASE_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Erase.c
FIRMWARE_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Firmware.c
FORMAT_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Format.c
GENERICTESTS_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_GenericTests.c
INFO_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Info.c
LOGS_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Logs.c
NVME_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_NVMe.c
PASSTHROUGHTEST_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_PassthroughTest.c
POWERCONTROL_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_PowerControl.c
RAW_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Raw.c
RESERVATIONS_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Reservations.c
SECURITY_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_Security.c
SMART_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_SMART.c
ZBD_SOURCES := $(TOOL_COMMON_SOURCES) openSeaChest_ZBD.c

#===============================================================================
# Tool Include Directories and Flags
#===============================================================================

TOOLS_INCLUDES := \
    -I$(PROJECT_ROOT)/include \
    -I$(PROJECT_SRC_DIR) \
    -I$(OPERATIONS_DIR)/include \
    -I$(TRANSPORT_DIR)/include \
    -I$(COMMON_DIR)/include

TOOLS_CFLAGS += $(TOOLS_INCLUDES)

# Add JSON includes if enabled
ifeq ($(BUILD_JSON),1)
    TOOLS_CFLAGS += -I$(JSONFORMAT_DIR)/include -I$(JSONC_DIR)
endif

#===============================================================================
# Tool Libraries to Link
#===============================================================================

TOOLS_LIBS := -L$(LIB_DIR) -lopensea-operations -lopensea-transport -lopensea-common

# Add platform libraries
TOOLS_LIBS += $(PLATFORM_LIBS)

# Add JSON library if enabled
ifeq ($(BUILD_JSON),1)
    TOOLS_LIBS += -lopensea-jsonformat -ljson-c
endif

# Add PIE flags for executables (ASLR security)
TOOLS_CFLAGS += $(PIE_CFLAGS)
TOOLS_LDFLAGS := $(LDFLAGS) $(PIE_LDFLAGS)

#===============================================================================
# vpath for Tool Sources
#===============================================================================

vpath %.c $(UTILS_DIR)
vpath %.c $(PROJECT_SRC_DIR)

#===============================================================================
# Build Rules for Each Tool
#===============================================================================

.PHONY: tools $(TOOLS)

# Build all tools (depends on libraries)
tools: libraries $(TOOL_EXES)

# Individual tool targets
$(TOOLS): %: $(BIN_DIR)/%$(EXE_EXT)

# Pattern rule for building tools
# All tools depend on opensea-operations (which transitively brings in transport and common)
$(BIN_DIR)/openSeaChest_Basics$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(BASICS_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Configure$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(CONFIGURE_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Defect$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(DEFECT_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Erase$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(ERASE_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Firmware$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(FIRMWARE_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Format$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(FORMAT_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_GenericTests$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(GENERICTESTS_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Info$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(INFO_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Logs$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(LOGS_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_NVMe$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(NVME_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_PassthroughTest$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(PASSTHROUGHTEST_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_PowerControl$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(POWERCONTROL_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Raw$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(RAW_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Reservations$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(RESERVATIONS_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_Security$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(SECURITY_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_SMART$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(SMART_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

$(BIN_DIR)/openSeaChest_ZBD$(EXE_EXT): $(addprefix $(OBJ_DIR_TOOLS)/,$(ZBD_SOURCES:.c=.o)) $(LIBOPERATIONS) $(LIBTRANSPORT) $(LIBCOMMON) | $(BIN_DIR)
	$(LINK_MSG)
	$(Q)$(CC) $(TOOLS_LDFLAGS) -o $@ $^ $(TOOLS_LIBS)

#===============================================================================
# Export Variables
#===============================================================================

export TOOLS
export TOOL_EXES
export TOOLS_CFLAGS
export TOOLS_LIBS
export TOOLS_LDFLAGS
