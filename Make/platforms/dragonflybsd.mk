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
# \file platforms/dragonflybsd.mk
# \brief DragonFlyBSD platform-specific configuration (uses CAM like FreeBSD)

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D_BSD_SOURCE \
    -DENABLE_CAM \
    -DDISABLE_TCG_SUPPORT

# DragonFlyBSD has NVMe support via CAM (same as FreeBSD)
ENABLE_NVME := 1

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery (reuse FreeBSD CAM helper)
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
