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
# \file platforms/linux.mk
# \brief Linux platform-specific configuration

#===============================================================================
# NVMe Support Detection
#===============================================================================

# Check for NVMe kernel headers (try multiple locations)
NVME_IOCTL_H := /usr/include/linux/nvme_ioctl.h
NVME_H := /usr/include/linux/nvme.h
UAPI_NVME_H := /usr/include/linux/uapi/nvme.h

HAS_NVME_IOCTL := $(shell test -f $(NVME_IOCTL_H) && echo yes)
HAS_NVME := $(shell test -f $(NVME_H) && echo yes)
HAS_UAPI_NVME := $(shell test -f $(UAPI_NVME_H) && echo yes)

# Enable NVMe if any header found
ifneq ($(HAS_NVME_IOCTL)$(HAS_NVME)$(HAS_UAPI_NVME),)
    ENABLE_NVME := 1
else
    ENABLE_NVME := 0
endif

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D_GNU_SOURCE \
    -DENABLE_CISS \
    -DENABLE_CSMI \
    -DDISABLE_TCG_SUPPORT

# Add NVMe define if supported
ifeq ($(ENABLE_NVME),1)
    PLATFORM_DEFINES += -DENABLE_NVME
else
    PLATFORM_DEFINES += -DDISABLE_NVME_PASSTHROUGH
endif

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src

#===============================================================================
# Platform Libraries
#===============================================================================

PLATFORM_LIBS := -lm -lrt

# pthread support (for threading in operations)
PLATFORM_LIBS += -lpthread

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)
TRANSPORT_CFLAGS += $(PLATFORM_DEFINES)
OPERATIONS_CFLAGS += $(PLATFORM_DEFINES)
TOOLS_CFLAGS += $(PLATFORM_DEFINES)

#===============================================================================
# Linker Flags
#===============================================================================

# Already set in security-hardening.mk:
# -Wl,--gc-sections
# -Wl,-z,relro,-z,now,-z,noexecstack,-z,nodlopen

# No additional platform-specific linker flags needed

#===============================================================================
# Export Variables
#===============================================================================

export ENABLE_NVME
export PLATFORM_DEFINES
export PLATFORM_LIBS
