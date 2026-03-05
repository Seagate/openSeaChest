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
# \file platforms/vmware.mk
# \brief VMware ESXi platform-specific configuration

#===============================================================================
# Include Shared VMware SDK Detection
#===============================================================================

include $(MAKE_ROOT)/vmware/vmware-sdk.inc

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    $(VMW_EXTRA_DEFS) \
    -D_GNU_SOURCE \
    -DDISABLE_TCG_SUPPORT

# VMware: NVMe passthrough is supported
ENABLE_NVME := 1

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src/linux
vpath %.c $(TRANSPORT_DIR)/src

#===============================================================================
# Platform Libraries
#===============================================================================

# Math library
PLATFORM_LIBS := -lm

# VMware management library (set in vmware-sdk.inc based on DEV_NEED_VMKMGMT)

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)
TRANSPORT_CFLAGS += $(PLATFORM_DEFINES)
OPERATIONS_CFLAGS += $(PLATFORM_DEFINES)
TOOLS_CFLAGS += $(PLATFORM_DEFINES)

# VMware SDK provides its own CFLAGS via userworld-tool.inc
# These will be merged with our flags

#===============================================================================
# DEV_APP Configuration (per-target)
#===============================================================================

# DEV_APP is set by library.mk (empty) and tools.mk (1)
# This tells VMware's build system whether we're building a library or executable

#===============================================================================
# Export Variables
#===============================================================================

export ENABLE_NVME
export PLATFORM_DEFINES
export PLATFORM_LIBS
