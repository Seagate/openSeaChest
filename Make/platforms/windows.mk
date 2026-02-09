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
# \file platforms/windows.mk
# \brief Windows/MinGW platform-specific configuration

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D__USE_MINGW_ANSI_STDIO=1 \
    -D_CRT_SECURE_NO_WARNINGS \
    -DENABLE_CISS \
    -DENABLE_CSMI \
    -DDISABLE_TCG_SUPPORT \
    -DSTATIC_OPENSEA_COMMON \
    -DSTATIC_OPENSEA_TRANSPORT \
    -DSTATIC_OPENSEA_OPERATIONS

# Windows always has NVMe support via STORAGE_PROTOCOL_COMMAND
ENABLE_NVME := 1
PLATFORM_DEFINES += -DENABLE_NVME

# Windows-specific: enable openSeaChest_NVMe utility
ENABLE_OPENSEACHEST_NVME := 1

# Windows-specific: Intel RST passthrough support
# Can be disabled by setting ENABLE_INTEL_RST=0
ifndef ENABLE_INTEL_RST
    ENABLE_INTEL_RST := 1
endif
ifeq ($(ENABLE_INTEL_RST),1)
    PLATFORM_DEFINES += -DENABLE_INTEL_RST
endif

# Windows-specific: OpenFabrics NVMe IOCTL support
# Can be disabled by setting ENABLE_OFNVME=0
ifndef ENABLE_OFNVME
    ENABLE_OFNVME := 1
endif
ifeq ($(ENABLE_OFNVME),1)
    PLATFORM_DEFINES += -DENABLE_OFNVME
endif

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src/windows
vpath %.c $(TRANSPORT_DIR)/src

#===============================================================================
# Platform Libraries
#===============================================================================

# Windows system libraries
PLATFORM_LIBS := \
    -lversion \
    -lcfgmgr32 \
    -ladvapi32 \
    -lsetupapi

# No math library needed on Windows
# No pthread needed (use Windows threads via opensea-common)

#===============================================================================
# Executable Extension
#===============================================================================

EXE_EXT := .exe

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)
TRANSPORT_CFLAGS += $(PLATFORM_DEFINES)
OPERATIONS_CFLAGS += $(PLATFORM_DEFINES)
TOOLS_CFLAGS += $(PLATFORM_DEFINES)

# MinGW-specific: ensure proper printf format checking
# (Already added in compiler-flags.mk, but emphasize here)

#===============================================================================
# Linker Flags
#===============================================================================

# Windows-specific: ensure we link static runtime (if desired)
ifeq ($(IS_STATIC),1)
    LDFLAGS += -static -static-libgcc
endif

# Windows doesn't use RELRO/NX linker flags (uses PE format, not ELF)
# Security hardening on Windows is done via:
# - DEP (Data Execution Prevention) - automatic in modern Windows
# - ASLR (Address Space Layout Randomization) - automatic in modern Windows
# - Control Flow Guard - would need /guard:cf (MSVC) or -mguard=cf (MinGW GCC 10+)

# Enable Control Flow Guard if supported (MinGW GCC 10+)
ifeq ($(IS_GCC),1)
    ifeq ($(GCC_AT_LEAST_10),1)
        CFG_SUPPORT := $(call cc_supports,-mguard=cf)
        ifneq ($(CFG_SUPPORT),)
            LDFLAGS += -mguard=cf
        endif
    endif
endif

#===============================================================================
# Export Variables
#===============================================================================

export ENABLE_NVME
export ENABLE_OPENSEACHEST_NVME
export EXE_EXT
export PLATFORM_DEFINES
export PLATFORM_LIBS
