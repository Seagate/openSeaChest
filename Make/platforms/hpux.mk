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
# \file platforms/hpux.mk
# \brief HP-UX platform-specific configuration

#===============================================================================
# Platform Defines
#===============================================================================

PLATFORM_DEFINES := \
    -D_POSIX_C_SOURCE=200112L \
    -D__EXTENSIONS__ \
    -DDISABLE_NVME_PASSTHROUGH \
    -DDISABLE_TCG_SUPPORT

# HP-UX: No NVMe support currently
ENABLE_NVME := 0

#===============================================================================
# Platform-Specific Sources
#===============================================================================

# Source lists are now defined in subproject sources.mk files
# See: subprojects/opensea-common/sources.mk
#      subprojects/opensea-transport/sources.mk

# Add to vpath for source discovery
vpath %.c $(TRANSPORT_DIR)/src
vpath %.c $(COMMON_DIR)/src/posix

#===============================================================================
# Platform Libraries
#===============================================================================

# HP-UX system libraries (if any specific ones are needed)
PLATFORM_LIBS :=

# Math library (standard)
MATH_LIB := -lm

# No pthread needed (or may need -lpthread if threading is used)

#===============================================================================
# Platform-Specific CFLAGS
#===============================================================================

COMMON_CFLAGS += $(PLATFORM_DEFINES)

#===============================================================================
# Platform-Specific LDFLAGS
#===============================================================================

# No special linker flags for HP-UX currently

#===============================================================================
# HP-UX Tools Detection
#===============================================================================

# openSeaChest utilities to build
ENABLE_OPENSEACHEST_NVME := 0

# Note: HP-UX support is limited. Testing recommended before production use.
