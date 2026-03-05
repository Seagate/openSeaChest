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
# \file security-hardening.mk
# \brief OpenSSF security hardening flags from meson.build
# \note Reference: https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++

# Define comma variable for use in subst/patsubst (Make doesn't allow literal commas in function calls)
comma := ,

# Only perform security hardening flag checking if we're not cleaning
ifneq ($(filter clean distclean mostlyclean,$(MAKECMDGOALS)),)
    # Skipping security checks during clean targets
else
$(info Testing security hardening flags...)
endif

#===============================================================================
# Stack Protection
#===============================================================================

# Stack protector (not on Windows due to GCC bug, Solaris needs special handling)
ifeq ($(PLATFORM),windows)
    ifeq ($(IS_GCC),1)
        # GCC on Windows has a bug with -fstack-clash-protection
        # https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90458
        # Fixed in GCC 11/12 but still fails in 12.2, so skip it
        HARDENING_FLAGS +=
    else
        # Clang on Windows is OK
        HARDENING_FLAGS += -fstack-clash-protection
    endif
else ifeq ($(PLATFORM),sunos)
    # Solaris: only enable stack-protector on Illumos, not vanilla Solaris
    ifeq ($(KERNEL),illumos)
        HARDENING_FLAGS += -fstack-protector-strong -fstack-clash-protection
    endif
else
    # All other platforms: full stack protection
    HARDENING_FLAGS += -fstack-protector-strong -fstack-clash-protection
endif

#===============================================================================
# Architecture-Specific Hardening
#===============================================================================

# x86_64: Control-flow protection
ifeq ($(ARCH),x86_64)
    ifneq ($(filter clean distclean mostlyclean,$(MAKECMDGOALS)),)
        ARCH_HARDENING :=
    else
        ARCH_HARDENING := $(call cc_supports,-fcf-protection=full)
    endif
    ifneq ($(ARCH_HARDENING),)
        HARDENING_FLAGS += -fcf-protection=full
    endif
endif

# ARM64/AArch64: Branch protection
ifeq ($(ARCH),aarch64)
    ifneq ($(filter clean distclean mostlyclean,$(MAKECMDGOALS)),)
        ARCH_HARDENING :=
    else
        ARCH_HARDENING := $(call cc_supports,-mbranch-protection=standard)
    endif
    ifneq ($(ARCH_HARDENING),)
        HARDENING_FLAGS += -mbranch-protection=standard
    endif
endif

# PowerPC64: Suppress noisy ABI change warnings (since GCC 5)
ifeq ($(findstring ppc64,$(ARCH)),ppc64)
    HARDENING_FLAGS += -Wno-psabi
endif

#===============================================================================
# FORTIFY_SOURCE (Runtime Buffer Overflow Detection)
#===============================================================================

# Test if _FORTIFY_SOURCE=3 is supported without warnings
# Rocky Linux 8 and some distros already set _FORTIFY_SOURCE=2 by default
# which causes redefinition warnings if we try to set it
FORTIFY_TEST_CODE := \
    '\#include <stdio.h>\n\
    int main(void) { return 0; }'

FORTIFY_LEVEL_3 := $(shell echo -e $(FORTIFY_TEST_CODE) | \
                    $(CC) -D_FORTIFY_SOURCE=3 -Werror -x c - -o /dev/null 2>/dev/null && echo yes)

FORTIFY_LEVEL_2 := $(shell echo -e $(FORTIFY_TEST_CODE) | \
                    $(CC) -D_FORTIFY_SOURCE=2 -Werror -x c - -o /dev/null 2>/dev/null && echo yes)

# Use highest supported FORTIFY_SOURCE level
ifneq ($(FORTIFY_LEVEL_3),)
    HARDENING_FLAGS += -D_FORTIFY_SOURCE=3
else ifneq ($(FORTIFY_LEVEL_2),)
    HARDENING_FLAGS += -D_FORTIFY_SOURCE=2
else
    # Fallback to =1 (always supported)
    HARDENING_FLAGS += -D_FORTIFY_SOURCE=1
endif

#===============================================================================
# Linker Hardening (Security Flags)
#===============================================================================

# Test each linker flag individually - not all linkers support all flags
# OmniOS/Solaris linkers don't support many GNU ld flags
LINKER_HARDENING :=

ifneq ($(PLATFORM),windows)
    # RELRO (Relocation Read-Only): prevents GOT overwrite attacks
    RELRO_TEST := $(shell echo 'int main(void) { return 0; }' | \
                  $(CC) -Wl,-z,relro -x c - -o /dev/null 2>/dev/null && echo yes)
    ifneq ($(RELRO_TEST),)
        LINKER_HARDENING += -Wl,-z,relro
    endif

    # BIND_NOW: resolve all symbols at program startup (prevents lazy binding attacks)
    NOW_TEST := $(shell echo 'int main(void) { return 0; }' | \
                $(CC) -Wl,-z,now -x c - -o /dev/null 2>/dev/null && echo yes)
    ifneq ($(NOW_TEST),)
        LINKER_HARDENING += -Wl,-z,now
    endif

    # NX Stack (No-Execute): prevent code execution on stack
    NOEXEC_TEST := $(shell echo 'int main(void) { return 0; }' | \
                   $(CC) -Wl,-z,noexecstack -x c - -o /dev/null 2>/dev/null && echo yes)
    ifneq ($(NOEXEC_TEST),)
        LINKER_HARDENING += -Wl,-z,noexecstack
    endif

    # Disable dlopen() on the executable (library hardening)
    NODLOPEN_TEST := $(shell echo 'int main(void) { return 0; }' | \
                     $(CC) -Wl,-z,nodlopen -x c - -o /dev/null 2>/dev/null && echo yes)
    ifneq ($(NODLOPEN_TEST),)
        LINKER_HARDENING += -Wl,-z,nodlopen
    endif
endif

# Garbage collection of unused sections (security + size reduction)
GC_SECTIONS_TEST := $(shell echo 'int main(void) { return 0; }' | \
                    $(CC) -Wl,--gc-sections -x c - -o /dev/null 2>/dev/null && echo yes)
ifneq ($(GC_SECTIONS_TEST),)
    LINKER_HARDENING += -Wl,--gc-sections
endif

# Dead code elimination (compiler flag, linker uses --gc-sections above)
HARDENING_FLAGS += -ffunction-sections -fdata-sections

#===============================================================================
# Position Independent Code (PIC/PIE)
#===============================================================================

# Test for -fPIC support (needed for static libraries on some platforms like BSD)
# This prevents relocation errors when linking static libraries into executables
ifneq ($(filter clean distclean mostlyclean,$(MAKECMDGOALS)),)
    PIC_SUPPORT :=
else
    PIC_SUPPORT := $(call cc_supports,-fPIC)
endif
ifneq ($(PIC_SUPPORT),)
    # Add -fPIC globally for static library compatibility
    HARDENING_FLAGS += -fPIC
endif

# PIE (Position Independent Executable) for ASLR
# Only apply for executables, not libraries (libraries are PIC by default)
# Test both compiler flag (-fPIE) and linker flag (-pie) support
ifneq ($(filter clean distclean mostlyclean,$(MAKECMDGOALS)),)
    PIE_CFLAGS_SUPPORT :=
    PIE_LDFLAGS_SUPPORT :=
else
    PIE_CFLAGS_SUPPORT := $(call cc_supports,-fPIE)
    PIE_LDFLAGS_SUPPORT := $(shell echo 'int main(void) { return 0; }' | \
                            $(CC) -fPIE -pie -x c - -o /dev/null 2>/dev/null && echo yes)
endif

ifneq ($(PIE_CFLAGS_SUPPORT),)
    PIE_CFLAGS := -fPIE
else
    PIE_CFLAGS :=
endif

ifneq ($(PIE_LDFLAGS_SUPPORT),)
    PIE_LDFLAGS := -pie
else
    PIE_LDFLAGS :=
endif

#===============================================================================
# Combined CFLAGS and LDFLAGS
#===============================================================================

# Add hardening to compiler flags
CFLAGS += $(HARDENING_FLAGS)
CXXFLAGS += $(HARDENING_FLAGS)

# Add hardening to linker flags
LDFLAGS += $(LINKER_HARDENING)

#===============================================================================
# Export Variables
#===============================================================================

export HARDENING_FLAGS
export LINKER_HARDENING
export PIE_CFLAGS
export PIE_LDFLAGS
export CFLAGS
export CXXFLAGS
export LDFLAGS
