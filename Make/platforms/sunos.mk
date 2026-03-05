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
# \file platforms/sunos.mk
# \brief Solaris/Illumos platform-specific configuration

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D_POSIX_C_SOURCE=200112L \
    -D__EXTENSIONS__ \
    -DDISABLE_NVME_PASSTHROUGH \
    -DDISABLE_TCG_SUPPORT

# Solaris/Illumos: No NVMe support currently
ENABLE_NVME := 0

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src/solaris
vpath %.c $(TRANSPORT_DIR)/src

#===============================================================================
# Platform Libraries
#===============================================================================

# Standard math library
PLATFORM_LIBS := -lm

# Solaris may need socket library for network operations
PLATFORM_LIBS += -lsocket -lnsl

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)
TRANSPORT_CFLAGS += $(PLATFORM_DEFINES)
OPERATIONS_CFLAGS += $(PLATFORM_DEFINES)
TOOLS_CFLAGS += $(PLATFORM_DEFINES)

#===============================================================================
# Solaris Linker Flags (different syntax than Linux)
#===============================================================================

# Solaris linker uses different syntax for some flags
# Already handled in security-hardening.mk with platform check

#===============================================================================
# Export Variables
#===============================================================================

export ENABLE_NVME
export PLATFORM_DEFINES
export PLATFORM_LIBS
