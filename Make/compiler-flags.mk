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
# \file compiler-flags.mk
# \brief Warning flags from meson.build with compatibility testing
# \note Flags extracted from meson.build April 4, 2024
# \note Reference: https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++

#===============================================================================
# Compiler Flag Support Testing
#===============================================================================

# Test if compiler supports a flag (mirrors Meson's get_supported_arguments)
# Returns "yes" if supported, empty if not
# Usage: $(call cc_supports,-Wflag-name)
cc_supports = $(shell echo 'int main(void) { return 0; }' | \
               $(CC) $(1) -Werror -x c - -o /dev/null 2>/dev/null && echo yes)

# Cache file for tested flags
COMPILER_FLAGS_CACHE := $(BUILD_DIR)/.compiler-flags-cache

# Create build directory if needed
$(shell mkdir -p $(BUILD_DIR) 2>/dev/null)

#===============================================================================
# Base Warning Flags (GCC 4.7+ and Clang 3.5+ known-good, no testing needed)
#===============================================================================

BASE_WARNING_FLAGS := \
    -Wall \
    -Wextra \
    -Wformat=2 \
    -Werror=format-security \
    -Wnull-dereference \
    -Wunused-const-variable \
    -Wunused-parameter \
    -Wunused-value \
    -Wfloat-equal \
    -Wvla \
    -Wdouble-promotion \
    -Wold-style-definition \
    -Wstrict-prototypes \
    -Wmissing-declarations \
    -Wmissing-prototypes \
    -Wchar-subscripts \
    -Wundef \
    -Wformat \
    -Wimplicit-fallthrough \
    -Werror=implicit \
    -Werror=int-conversion \
    -Werror=implicit-int \
    -Wparentheses \
    -Wcast-qual \
    -Wuninitialized \
    -Wvarargs \
    -Wwrite-strings \
    -Wcomment \
    -Wsequence-point \
    -Wreturn-type

#===============================================================================
# Newer Warning Flags (require version checks or testing)
#===============================================================================

# GCC-specific newer flags (test for support)
NEWER_WARNING_CANDIDATES_GCC := \
    -Wshadow=compatible-local \
    -Wduplicated-cond \
    -Wjump-misses-init \
    -Wstringop-overflow \
    -Wlogical-op \
    -Wshift-overflow=2 \
    -Wrestrict \
    -Wstringop-truncation \
    -Wtrampolines \
    -Wmismatched-dealloc \
    -Wfree-nonheap-object

# Clang-specific newer flags (test for support)
NEWER_WARNING_CANDIDATES_CLANG := \
    -Wincompatible-pointer-types-discards-qualifiers \
    -Woverlength-strings \
    -Wnewline-eof \
    -Wextra-semi \
    -Werror=sometimes-uninitialized \
    -Wunevaluated-expression \
    -Wunsequenced \
    -Wunreachable-code \
    -Wpointer-bool-conversion \
    -Wnullability \
    -Wnullability-completeness \
    -Wnullability-completeness-on-arrays

# Common newer flags (both GCC and Clang, test for support)
NEWER_WARNING_CANDIDATES_COMMON := \
    -Werror=incompatible-pointer-types \
    -Werror=trigraphs \
    -Walloc-zero \
    -Walloc-size \
    -Wcalloc-transposed-args \
    -Werror=alloca \
    -Wstring-compare

# Test newer flags based on compiler
ifeq ($(IS_GCC),1)
    NEWER_WARNING_CANDIDATES := $(NEWER_WARNING_CANDIDATES_GCC) $(NEWER_WARNING_CANDIDATES_COMMON)
else ifeq ($(IS_CLANG),1)
    NEWER_WARNING_CANDIDATES := $(NEWER_WARNING_CANDIDATES_CLANG) $(NEWER_WARNING_CANDIDATES_COMMON)
else
    NEWER_WARNING_CANDIDATES := $(NEWER_WARNING_CANDIDATES_COMMON)
endif

# Test each flag and collect supported ones
NEWER_WARNINGS := $(foreach flag,$(NEWER_WARNING_CANDIDATES),$(if $(call cc_supports,$(flag)),$(flag)))

#===============================================================================
# Conversion Warnings (commonly supported)
#===============================================================================

CONVERSION_WARNING_FLAGS := \
    -Wint-conversion \
    -Wenum-conversion \
    -Wfloat-conversion \
    -Wint-to-pointer-cast

# GCC 10+ specific: -Wsign-conversion (very noisy, use while debugging)
# NOTE: Intentionally not enabled by default per meson.build comments
# Uncomment if desired:
# ifeq ($(GCC_AT_LEAST_10),1)
#     CONVERSION_WARNING_FLAGS += -Wsign-conversion
# endif

#===============================================================================
# Common Defines and Flags
#===============================================================================

# GLibC++ assertions for C++ (if building any C++ code)
COMMON_DEFINES := -D_GLIBCXX_ASSERTIONS

# Strict flex arrays (GCC 13+, Clang 16+)
STRICT_FLEX_ARRAYS := $(call cc_supports,-fstrict-flex-arrays=3)
ifneq ($(STRICT_FLEX_ARRAYS),)
    COMMON_FLAGS += -fstrict-flex-arrays=3
endif

# Safe null pointer handling
COMMON_FLAGS += -fno-delete-null-pointer-checks

# Prevent aggressive optimizations that violate C standard
COMMON_FLAGS += -fno-strict-overflow -fno-strict-aliasing

# Zero-initialize automatic variables
ZERO_INIT_AUTO_VAR := $(call cc_supports,-ftrivial-auto-var-init=zero)
ifneq ($(ZERO_INIT_AUTO_VAR),)
    COMMON_FLAGS += -ftrivial-auto-var-init=zero
endif

# Zero-initialize padding bits (GCC 14+, Clang 18+)
ZERO_INIT_PADDING := $(call cc_supports,-fzero-init-padding-bits=all)
ifneq ($(ZERO_INIT_PADDING),)
    COMMON_FLAGS += -fzero-init-padding-bits=all
endif

# Hidden visibility (works like Windows DLL import/export)
COMMON_FLAGS += -fvisibility=hidden

# Disable C23 extensions warning (we check availability before use)
NO_C23_EXT := $(call cc_supports,-Wno-c23-extensions)
ifneq ($(NO_C23_EXT),)
    COMMON_FLAGS += -Wno-c23-extensions
endif

#===============================================================================
# Combined Warning Flags
#===============================================================================

WARNING_FLAGS := $(BASE_WARNING_FLAGS) $(NEWER_WARNINGS) $(CONVERSION_WARNING_FLAGS)

# Add common flags
WARNING_FLAGS += $(COMMON_DEFINES) $(COMMON_FLAGS)

#===============================================================================
# Optimization-Level Flags (Build-Type Specific)
#===============================================================================

# Use optimization flags from config.mk (based on BUILD_TYPE)
# - debug: -O0 -g3
# - release: -O3
# - relwithdebinfo: -O2 -g
# - minsize: -Os
OPT_FLAGS := $(OPTIMIZATION_FLAGS)

# Add frame pointer for debugging (even in optimized builds)
ifeq ($(BUILD_TYPE),debug)
    OPT_FLAGS += -fno-omit-frame-pointer
else ifeq ($(BUILD_TYPE),relwithdebinfo)
    # Keep frame pointer for profiling
    OPT_FLAGS += -fno-omit-frame-pointer
endif

# Use build defines from config.mk (based on BUILD_TYPE)
DEBUG_FLAGS := $(BUILD_DEFINES)

# Link-time optimization for release and minsize builds
ifeq ($(filter $(BUILD_TYPE),release minsize),$(BUILD_TYPE))
    LTO := $(call cc_supports,-flto)
    ifneq ($(LTO),)
        OPT_FLAGS += -flto
        # Need to pass LTO flag to linker too
        LDFLAGS += -flto
    endif
endif

#===============================================================================
# Platform-Specific Flag Adjustments
#===============================================================================

# MinGW/Windows specific
ifeq ($(PLATFORM),windows)
    # Use MinGW ANSI stdio for proper printf format checking
    WARNING_FLAGS += -D__USE_MINGW_ANSI_STDIO=1

    # Suppress Windows CRT secure warnings (we use safe_ functions from opensea-common)
    WARNING_FLAGS += -D_CRT_SECURE_NO_WARNINGS
endif

# MUSL libc specific
ifeq ($(USING_MUSL),1)
    # MUSL doesn't have certain GLibC features
    WARNING_FLAGS := $(filter-out -D_GLIBCXX_ASSERTIONS,$(WARNING_FLAGS))
endif

#===============================================================================
# Combined CFLAGS and CXXFLAGS
#===============================================================================

# Initialize if not already set
CFLAGS ?=
CXXFLAGS ?=

# Combine all flags
CFLAGS := $(WARNING_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(CFLAGS)
CXXFLAGS := $(WARNING_FLAGS) $(OPT_FLAGS) $(DEBUG_FLAGS) $(CXXFLAGS)

#===============================================================================
# Save Tested Flags to Cache (for debugging)
#===============================================================================

.PHONY: save-flag-cache
save-flag-cache:
	@echo "# Compiler flag test cache" > $(COMPILER_FLAGS_CACHE)
	@echo "# Generated: $(shell date)" >> $(COMPILER_FLAGS_CACHE)
	@echo "# Compiler: $(COMPILER_NAME)" >> $(COMPILER_FLAGS_CACHE)
	@echo "" >> $(COMPILER_FLAGS_CACHE)
	@echo "Supported newer flags:" >> $(COMPILER_FLAGS_CACHE)
	@$(foreach flag,$(NEWER_WARNINGS),echo "  $(flag)" >> $(COMPILER_FLAGS_CACHE);)
	@echo "" >> $(COMPILER_FLAGS_CACHE)
	@echo "Unsupported flags:" >> $(COMPILER_FLAGS_CACHE)
	@$(foreach flag,$(NEWER_WARNING_CANDIDATES),\
	  $(if $(filter $(flag),$(NEWER_WARNINGS)),,echo "  $(flag)" >> $(COMPILER_FLAGS_CACHE);))

#===============================================================================
# Export Variables
#===============================================================================

export WARNING_FLAGS
export OPT_FLAGS
export DEBUG_FLAGS
export CFLAGS
export CXXFLAGS
