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
# \file compiler-detection.mk
# \brief Detect compiler type and version for version-specific flags

#===============================================================================
# Compiler Detection
#===============================================================================

# Detect if using GCC or Clang
COMPILER_ID := $(shell $(CC) --version 2>/dev/null | head -n1)
COMPILER_ID_LOWER := $(shell echo '$(COMPILER_ID)' | tr 'A-Z' 'a-z')

# GCC detection (case-insensitive)
IS_GCC := 0
ifneq ($(findstring gcc,$(COMPILER_ID_LOWER)),)
    IS_GCC := 1
endif
ifneq ($(findstring g++,$(COMPILER_ID_LOWER)),)
    IS_GCC := 1
endif

# Clang detection (case-insensitive)
IS_CLANG := 0
ifneq ($(findstring clang,$(COMPILER_ID_LOWER)),)
    IS_CLANG := 1
    # Clang pretends to be GCC, disambiguate
    IS_GCC := 0
endif

#===============================================================================
# GCC Version Detection
#===============================================================================

ifeq ($(IS_GCC),1)
    # Get GCC version (format: major.minor.patch)
    GCC_VERSION_FULL := $(shell $(CC) -dumpversion 2>/dev/null)

    # Parse major version (strip non-numeric suffixes like "-win32" from MinGW)
    GCC_MAJOR := $(shell echo $(GCC_VERSION_FULL) | cut -d. -f1 | sed 's/[^0-9].*//')

    # Parse minor version (default to 0 if not present)
    GCC_MINOR := $(shell echo $(GCC_VERSION_FULL) | cut -d. -f2 | sed 's/[^0-9].*//' 2>/dev/null || echo 0)
    ifeq ($(GCC_MINOR),)
        GCC_MINOR := 0
    endif

    # Parse patch version (default to 0 if not present)
    GCC_PATCH := $(shell echo $(GCC_VERSION_FULL) | cut -d. -f3 | sed 's/[^0-9].*//' 2>/dev/null || echo 0)
    ifeq ($(GCC_PATCH),)
        GCC_PATCH := 0
    endif

    # Combined version for comparisons (e.g., 40900 for 4.9.0)
    GCC_VERSION_NUM := $(shell printf "%d%02d%02d" $(GCC_MAJOR) $(GCC_MINOR) $(GCC_PATCH) 2>/dev/null || echo 0)

    # Version checks (for conditional flag support)
    GCC_AT_LEAST_4_7 := $(shell [ $(GCC_MAJOR) -gt 4 -o \( $(GCC_MAJOR) -eq 4 -a $(GCC_MINOR) -ge 7 \) ] && echo 1 || echo 0)
    GCC_AT_LEAST_4_8 := $(shell [ $(GCC_MAJOR) -gt 4 -o \( $(GCC_MAJOR) -eq 4 -a $(GCC_MINOR) -ge 8 \) ] && echo 1 || echo 0)
    GCC_AT_LEAST_4_9 := $(shell [ $(GCC_MAJOR) -gt 4 -o \( $(GCC_MAJOR) -eq 4 -a $(GCC_MINOR) -ge 9 \) ] && echo 1 || echo 0)
    GCC_AT_LEAST_5 := $(shell [ $(GCC_MAJOR) -ge 5 ] && echo 1 || echo 0)
    GCC_AT_LEAST_6 := $(shell [ $(GCC_MAJOR) -ge 6 ] && echo 1 || echo 0)
    GCC_AT_LEAST_7 := $(shell [ $(GCC_MAJOR) -ge 7 ] && echo 1 || echo 0)
    GCC_AT_LEAST_8 := $(shell [ $(GCC_MAJOR) -ge 8 ] && echo 1 || echo 0)
    GCC_AT_LEAST_9 := $(shell [ $(GCC_MAJOR) -ge 9 ] && echo 1 || echo 0)
    GCC_AT_LEAST_10 := $(shell [ $(GCC_MAJOR) -ge 10 ] && echo 1 || echo 0)
    GCC_AT_LEAST_11 := $(shell [ $(GCC_MAJOR) -ge 11 ] && echo 1 || echo 0)
    GCC_AT_LEAST_12 := $(shell [ $(GCC_MAJOR) -ge 12 ] && echo 1 || echo 0)
    GCC_AT_LEAST_13 := $(shell [ $(GCC_MAJOR) -ge 13 ] && echo 1 || echo 0)
    GCC_AT_LEAST_14 := $(shell [ $(GCC_MAJOR) -ge 14 ] && echo 1 || echo 0)

    COMPILER_NAME := GCC $(GCC_VERSION_FULL)
else
    GCC_VERSION_FULL := 0.0.0
    GCC_MAJOR := 0
    GCC_MINOR := 0
    GCC_PATCH := 0
    GCC_VERSION_NUM := 0
    GCC_AT_LEAST_4_7 := 0
    GCC_AT_LEAST_4_8 := 0
    GCC_AT_LEAST_4_9 := 0
    GCC_AT_LEAST_5 := 0
    GCC_AT_LEAST_6 := 0
    GCC_AT_LEAST_7 := 0
    GCC_AT_LEAST_8 := 0
    GCC_AT_LEAST_9 := 0
    GCC_AT_LEAST_10 := 0
    GCC_AT_LEAST_11 := 0
    GCC_AT_LEAST_12 := 0
    GCC_AT_LEAST_13 := 0
    GCC_AT_LEAST_14 := 0
endif

#===============================================================================
# Clang Version Detection
#===============================================================================

ifeq ($(IS_CLANG),1)
    # Get Clang version (format: "clang version major.minor.patch")
    CLANG_VERSION_OUTPUT := $(shell $(CC) --version 2>/dev/null | head -n1)
    CLANG_VERSION_FULL := $(shell echo "$(CLANG_VERSION_OUTPUT)" | sed -n 's/.*version \([0-9.]*\).*/\1/p')

    # Parse major version
    CLANG_MAJOR := $(shell echo $(CLANG_VERSION_FULL) | cut -d. -f1)

    # Parse minor version (default to 0 if not present)
    CLANG_MINOR := $(shell echo $(CLANG_VERSION_FULL) | cut -d. -f2 2>/dev/null || echo 0)

    # Parse patch version (default to 0 if not present)
    CLANG_PATCH := $(shell echo $(CLANG_VERSION_FULL) | cut -d. -f3 2>/dev/null || echo 0)

    # Combined version for comparisons
    CLANG_VERSION_NUM := $(shell printf "%d%02d%02d" $(CLANG_MAJOR) $(CLANG_MINOR) $(CLANG_PATCH) 2>/dev/null || echo 0)

    # Version checks (for conditional flag support)
    CLANG_AT_LEAST_3_0 := $(shell [ $(CLANG_MAJOR) -ge 3 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_3_5 := $(shell [ $(CLANG_MAJOR) -gt 3 -o \( $(CLANG_MAJOR) -eq 3 -a $(CLANG_MINOR) -ge 5 \) ] && echo 1 || echo 0)
    CLANG_AT_LEAST_5 := $(shell [ $(CLANG_MAJOR) -ge 5 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_7 := $(shell [ $(CLANG_MAJOR) -ge 7 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_10 := $(shell [ $(CLANG_MAJOR) -ge 10 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_12 := $(shell [ $(CLANG_MAJOR) -ge 12 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_14 := $(shell [ $(CLANG_MAJOR) -ge 14 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_16 := $(shell [ $(CLANG_MAJOR) -ge 16 ] && echo 1 || echo 0)
    CLANG_AT_LEAST_18 := $(shell [ $(CLANG_MAJOR) -ge 18 ] && echo 1 || echo 0)

    COMPILER_NAME := Clang $(CLANG_VERSION_FULL)
else
    CLANG_VERSION_FULL := 0.0.0
    CLANG_MAJOR := 0
    CLANG_MINOR := 0
    CLANG_PATCH := 0
    CLANG_VERSION_NUM := 0
    CLANG_AT_LEAST_3_0 := 0
    CLANG_AT_LEAST_3_5 := 0
    CLANG_AT_LEAST_5 := 0
    CLANG_AT_LEAST_7 := 0
    CLANG_AT_LEAST_10 := 0
    CLANG_AT_LEAST_12 := 0
    CLANG_AT_LEAST_14 := 0
    CLANG_AT_LEAST_16 := 0
    CLANG_AT_LEAST_18 := 0
endif

#===============================================================================
# Compiler Unknown Warning
#===============================================================================

ifeq ($(IS_GCC)$(IS_CLANG),00)
    COMPILER_NAME := Unknown ($(COMPILER_ID))
    $(warning Unknown compiler detected: $(COMPILER_ID))
    $(warning Build may fail. Supported compilers: GCC, Clang)
endif

#===============================================================================
# Export Variables
#===============================================================================

export IS_GCC
export IS_CLANG
export GCC_VERSION_FULL
export GCC_MAJOR
export GCC_MINOR
export GCC_PATCH
export GCC_VERSION_NUM
export CLANG_VERSION_FULL
export CLANG_MAJOR
export CLANG_MINOR
export CLANG_PATCH
export CLANG_VERSION_NUM
export COMPILER_NAME
