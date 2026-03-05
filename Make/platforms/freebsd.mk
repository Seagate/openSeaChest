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
# \file platforms/freebsd.mk
# \brief FreeBSD platform-specific configuration (uses CAM for SCSI/ATA/NVMe)

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D_BSD_SOURCE \
    -DENABLE_CAM \
    -DDISABLE_TCG_SUPPORT

# FreeBSD has NVMe support via CAM if the nvme.h header is available
# Check for /usr/include/dev/nvme/nvme.h
FREEBSD_NVME_H := /usr/include/dev/nvme/nvme.h
ifneq ($(shell test -f $(FREEBSD_NVME_H) && printf "yes"),yes)
    PLATFORM_DEFINES += -DDISABLE_NVME_PASSTHROUGH
    ENABLE_NVME := 0
else
    ENABLE_NVME := 1
endif

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src/freebsd
vpath %.c $(TRANSPORT_DIR)/src

#===============================================================================
# Platform Libraries
#===============================================================================

# CAM library (required for device access)
PLATFORM_LIBS := -lcam

# Standard math library
PLATFORM_LIBS += -lm

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)
TRANSPORT_CFLAGS += $(PLATFORM_DEFINES)
OPERATIONS_CFLAGS += $(PLATFORM_DEFINES)
TOOLS_CFLAGS += $(PLATFORM_DEFINES)

#===============================================================================
# Export Variables
#===============================================================================

export ENABLE_NVME
export PLATFORM_DEFINES
export PLATFORM_LIBS
