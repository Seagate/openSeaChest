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
# \file config.mk
# \brief Platform detection, build configuration, and directory structure

# GNU Make 3.81 or later required
MAKE_VERSION_REQUIRED := 3.81
MAKE_VERSION_CURRENT := $(MAKE_VERSION)

ifeq ($(filter $(MAKE_VERSION_REQUIRED),$(firstword $(sort $(MAKE_VERSION_CURRENT) $(MAKE_VERSION_REQUIRED)))),)
    $(error GNU Make $(MAKE_VERSION_REQUIRED) or later required. Current: $(MAKE_VERSION_CURRENT))
endif

# Project root (parent of Make/ directory)
MAKE_ROOT := $(dir $(lastword $(MAKEFILE_LIST)))
PROJECT_ROOT := $(abspath $(MAKE_ROOT)/..)

# Fail on undefined variables
MAKEFLAGS += --warn-undefined-variables

# Use one shell for entire recipe (security + performance)
.ONESHELL:

# Delete target files if recipe fails (prevents incomplete/corrupted files)
.DELETE_ON_ERROR:

# Safe shell with error handling
# Prefer bash for pipefail support (catches errors in pipes), fall back to POSIX sh
# NOTE: bash is recommended for best error detection. Install with:
#   - Linux/BSD: pkg install bash (or apt/yum/dnf install bash)
#   - AIX: installp -a -d . bash (from AIX Toolbox)
#   - HP-UX: swinstall bash (from HP-UX porting centre)
#   - Solaris: pkg install bash
SHELL := $(shell command -v bash 2>/dev/null || echo sh)
.SHELLFLAGS := $(if $(findstring bash,$(SHELL)),-eu -o pipefail -c,-eu -c)

#===============================================================================
# Platform Detection
#===============================================================================

# Detect operating system
UNAME_S := $(shell uname -s 2>/dev/null || echo unknown)
UNAME_M := $(shell uname -m 2>/dev/null || echo unknown)

# Detect host platform (where we're building, not what we're building for)
ifeq ($(findstring Linux,$(UNAME_S)),Linux)
    HOST_PLATFORM := linux
else ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    HOST_PLATFORM := windows
else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    HOST_PLATFORM := windows
else ifeq ($(findstring CYGWIN,$(UNAME_S)),CYGWIN)
    HOST_PLATFORM := windows
else ifeq ($(findstring FreeBSD,$(UNAME_S)),FreeBSD)
    HOST_PLATFORM := freebsd
else
    HOST_PLATFORM := $(UNAME_S)
endif

# Check for VMware ESXi build environment first (special case)
# Note: Use basic grep for Solaris compatibility (no -E support)
HAS_VMWARE_DDK := $(shell rpm -qa 2>/dev/null | grep 'vmware-esx-.*ddk-devtools' || true)

# Check if using MinGW cross-compiler (e.g., x86_64-w64-mingw32-gcc on Linux)
CC_NAME := $(shell $(CC) --version 2>/dev/null | head -n1)
IS_MINGW_CC := $(findstring mingw,$(CC_NAME))

# Detect if cross-compiling (MinGW on non-Windows host)
CROSS_COMPILING := 0
ifneq ($(IS_MINGW_CC),)
    ifneq ($(HOST_PLATFORM),windows)
        CROSS_COMPILING := 1
    endif
endif

# Platform detection (simplified to avoid nested function issues)
ifdef HAS_VMWARE_DDK
    PLATFORM_DETECTED := vmware
else ifneq ($(IS_MINGW_CC),)
    # Auto-detect MinGW cross-compilation (e.g., x86_64-w64-mingw32-gcc on Linux)
    PLATFORM_DETECTED := windows
else ifeq ($(findstring Linux,$(UNAME_S)),Linux)
    PLATFORM_DETECTED := linux
else ifeq ($(findstring Darwin,$(UNAME_S)),Darwin)
    PLATFORM_DETECTED := macos
else ifeq ($(findstring FreeBSD,$(UNAME_S)),FreeBSD)
    PLATFORM_DETECTED := freebsd
else ifeq ($(findstring OpenBSD,$(UNAME_S)),OpenBSD)
    PLATFORM_DETECTED := openbsd
else ifeq ($(findstring NetBSD,$(UNAME_S)),NetBSD)
    PLATFORM_DETECTED := netbsd
else ifeq ($(findstring DragonFly,$(UNAME_S)),DragonFly)
    PLATFORM_DETECTED := dragonflybsd
else ifeq ($(findstring SunOS,$(UNAME_S)),SunOS)
    PLATFORM_DETECTED := sunos
else ifeq ($(findstring AIX,$(UNAME_S)),AIX)
    PLATFORM_DETECTED := aix
else ifeq ($(findstring HP-UX,$(UNAME_S)),HP-UX)
    PLATFORM_DETECTED := hpux
else ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    PLATFORM_DETECTED := windows
else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    PLATFORM_DETECTED := windows
else ifeq ($(findstring CYGWIN,$(UNAME_S)),CYGWIN)
    PLATFORM_DETECTED := windows
else
    PLATFORM_DETECTED := unknown
endif

# Set PLATFORM (user can override with PLATFORM=xxx)
PLATFORM ?= $(PLATFORM_DETECTED)

# Validate platform
SUPPORTED_PLATFORMS := linux windows freebsd openbsd netbsd dragonflybsd sunos aix hpux vmware macos
ifeq ($(filter $(PLATFORM),$(SUPPORTED_PLATFORMS)),)
    $(error Unsupported PLATFORM=$(PLATFORM). Supported: $(SUPPORTED_PLATFORMS))
endif

# Detect kernel for Solaris/Illumos distinction
ifeq ($(PLATFORM),sunos)
    KERNEL := $(shell uname -v 2>/dev/null | grep -qi illumos && echo illumos || echo solaris)
else
    KERNEL := $(PLATFORM)
endif

#===============================================================================
# Architecture Detection
#===============================================================================

# Architecture mapping (simplified)
ifeq ($(findstring x86_64,$(UNAME_M)),x86_64)
    ARCH_DETECTED := x86_64
else ifeq ($(findstring amd64,$(UNAME_M)),amd64)
    ARCH_DETECTED := x86_64
else ifeq ($(findstring i686,$(UNAME_M)),i686)
    ARCH_DETECTED := i686
else ifeq ($(findstring i386,$(UNAME_M)),i386)
    ARCH_DETECTED := i686
else ifeq ($(findstring aarch64,$(UNAME_M)),aarch64)
    ARCH_DETECTED := aarch64
else ifeq ($(findstring arm64,$(UNAME_M)),arm64)
    ARCH_DETECTED := aarch64
else ifeq ($(findstring armv7,$(UNAME_M)),armv7)
    ARCH_DETECTED := armv7l
else ifeq ($(findstring ppc64le,$(UNAME_M)),ppc64le)
    ARCH_DETECTED := powerpc64le
else ifeq ($(findstring ppc64,$(UNAME_M)),ppc64)
    ARCH_DETECTED := powerpc64
else ifeq ($(findstring riscv64,$(UNAME_M)),riscv64)
    ARCH_DETECTED := riscv64
else
    ARCH_DETECTED := $(UNAME_M)
endif

ARCH ?= $(ARCH_DETECTED)

#===============================================================================
# Compiler Selection
#===============================================================================

# Auto-detect compiler
# On Solaris/Illumos, prefer gcc/clang if available, otherwise fall back to cc
# This avoids issues where 'cc' may not be in PATH on some Solaris installations
ifeq ($(PLATFORM_DETECTED),sunos)
    # Check for gcc first, then clang, then cc
    HAS_GCC := $(shell command -v gcc 2>/dev/null)
    HAS_CLANG := $(shell command -v clang 2>/dev/null)
    ifneq ($(HAS_GCC),)
        CC ?= gcc
        CXX ?= g++
    else ifneq ($(HAS_CLANG),)
        CC ?= clang
        CXX ?= clang++
    else
        CC ?= cc
        CXX ?= c++
    endif
else
    # All other platforms: use cc by default (respects system defaults)
    CC ?= cc
    CXX ?= c++
endif

# Export CC/CXX immediately so compiler-detection.mk uses the correct compiler
export CC
export CXX

#===============================================================================
# Build Configuration
#===============================================================================

# Build type: debug, release, relwithdebinfo, minsize
# Matches Meson's --buildtype options for consistency
BUILD_TYPE ?= release
VALID_BUILD_TYPES := debug release relwithdebinfo minsize

ifeq ($(filter $(BUILD_TYPE),$(VALID_BUILD_TYPES)),)
    $(error Invalid BUILD_TYPE=$(BUILD_TYPE). Valid: $(VALID_BUILD_TYPES))
endif

# Optimization and debug flags based on build type
# Use ?= so downstream packagers can override (e.g., OPTIMIZATION_FLAGS="-O2" make)
ifeq ($(BUILD_TYPE),debug)
    # No optimization, maximum debug information
    OPTIMIZATION_FLAGS ?= -O0 -g3
    BUILD_DEFINES ?= -DDEBUG -D_DEBUG
else ifeq ($(BUILD_TYPE),release)
    # Full optimization, no debug info
    OPTIMIZATION_FLAGS ?= -O3
    BUILD_DEFINES ?= -DNDEBUG -DRELEASE
else ifeq ($(BUILD_TYPE),relwithdebinfo)
    # Optimized with debug symbols (for production debugging/profiling)
    OPTIMIZATION_FLAGS ?= -O2 -g
    BUILD_DEFINES ?= -DNDEBUG
else ifeq ($(BUILD_TYPE),minsize)
    # Optimize for binary size
    OPTIMIZATION_FLAGS ?= -Os
    BUILD_DEFINES ?= -DNDEBUG
endif

# Legacy support for static builds (deprecated, use IS_STATIC variable instead)
IS_STATIC ?= 0

# Legacy IS_DEBUG flag (derived from BUILD_TYPE for backward compatibility)
IS_DEBUG := $(if $(filter debug,$(BUILD_TYPE)),1,0)

# Feature flags (user-overridable)
BUILD_JSON ?= 0
BUILD_SHARED_LIBS ?= 0
USING_MUSL ?= 0

# Validate BUILD_JSON
ifeq ($(BUILD_JSON),1)
    HAS_CMAKE := $(shell command -v cmake 2>/dev/null)
    ifeq ($(HAS_CMAKE),)
        $(error BUILD_JSON=1 requires CMake but cmake was not found in PATH)
    endif
endif

#===============================================================================
# Reproducible Builds Support
# Reference: https://reproducible-builds.org/docs/source-date-epoch/
#===============================================================================

# SOURCE_DATE_EPOCH: Unix timestamp for reproducible builds
# If set, use it for BUILD_TIMESTAMP; otherwise use current date/time
# This supports reproducible builds while maintaining compatibility
ifdef SOURCE_DATE_EPOCH
    # Use SOURCE_DATE_EPOCH if set (for reproducible builds)
    # Try GNU date format first (-d flag), fall back to BSD date format (-r flag)
    BUILD_TIMESTAMP := $(shell date -u -d "@$(SOURCE_DATE_EPOCH)" 2>/dev/null || date -u -r "$(SOURCE_DATE_EPOCH)" 2>/dev/null || date -u)
else
    # Use current date/time if SOURCE_DATE_EPOCH not set
    BUILD_TIMESTAMP := $(shell date -u)
endif

# Export for use in build flags if needed
export BUILD_TIMESTAMP

#===============================================================================
# Build Directories
#===============================================================================

# Build directory structure: build/platform-arch-buildtype/
BUILD_DIR_NAME := $(PLATFORM)-$(ARCH)-$(BUILD_TYPE)
BUILD_DIR := $(PROJECT_ROOT)/build/$(BUILD_DIR_NAME)

# Subdirectories
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := $(BUILD_DIR)/lib
BIN_DIR := $(BUILD_DIR)/bin

# Object file subdirectories (one per library/module)
OBJ_DIR_COMMON := $(OBJ_DIR)/common
OBJ_DIR_TRANSPORT := $(OBJ_DIR)/transport
OBJ_DIR_OPERATIONS := $(OBJ_DIR)/operations
OBJ_DIR_JSONFORMAT := $(OBJ_DIR)/jsonformat
OBJ_DIR_TOOLS := $(OBJ_DIR)/tools

# All object directories (for order-only prerequisites)
OBJ_DIRS := $(OBJ_DIR_COMMON) $(OBJ_DIR_TRANSPORT) $(OBJ_DIR_OPERATIONS) \
            $(OBJ_DIR_JSONFORMAT) $(OBJ_DIR_TOOLS)

# Create directories target
$(OBJ_DIR) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIRS):
	@mkdir -p $@

#===============================================================================
# Build System Dependencies
#===============================================================================

# Automatically rebuild all targets if build configuration changes
# This ensures clean rebuilds when compiler flags, security settings, etc. change
.EXTRA_PREREQS := $(MAKE_ROOT)config.mk $(MAKE_ROOT)compiler-flags.mk $(MAKE_ROOT)security-hardening.mk

#===============================================================================
# Toolchain
#===============================================================================

# Compiler and tools
# Auto-detect system default compiler (cc points to gcc or clang depending on system)
# FreeBSD/OpenBSD use clang by default, most Linux use gcc
# Solaris doesn't have 'cc' by default, use gcc
# Only override if CC is unset (not if it's already set to 'cc')
ifeq ($(PLATFORM),sunos)
    CC ?= gcc
    CXX ?= g++
else
    CC ?= cc
    CXX ?= c++
endif
AR ?= ar
STRIP ?= strip
INSTALL ?= install

# Ranlib (some platforms need it)
RANLIB ?= ranlib

#===============================================================================
# Installation Directories
#===============================================================================

# Installation prefix (user-overridable)
PREFIX ?= /usr/local
DESTDIR ?=

# Standard installation directories
BINDIR := $(PREFIX)/bin
LIBDIR := $(PREFIX)/lib
INCLUDEDIR := $(PREFIX)/include
MANDIR := $(PREFIX)/share/man
MAN8DIR := $(MANDIR)/man8

#===============================================================================
# Source Directories
#===============================================================================

# Subproject source directories
COMMON_DIR := $(PROJECT_ROOT)/subprojects/opensea-common
TRANSPORT_DIR := $(PROJECT_ROOT)/subprojects/opensea-transport
OPERATIONS_DIR := $(PROJECT_ROOT)/subprojects/opensea-operations
JSONFORMAT_DIR := $(PROJECT_ROOT)/subprojects/opensea-jsonformat
WINGETOPT_DIR := $(PROJECT_ROOT)/subprojects/wingetopt
JSONC_DIR := $(PROJECT_ROOT)/subprojects/json-c

# Utility source directory
UTILS_DIR := $(PROJECT_ROOT)/utils/C/openSeaChest

# Common project source files
PROJECT_SRC_DIR := $(PROJECT_ROOT)/src

#===============================================================================
# Library Versions
#===============================================================================

# opensea-common version
COMMON_MAJOR := 6
COMMON_MINOR := 0
COMMON_PATCH := 4
COMMON_VERSION := $(COMMON_MAJOR).$(COMMON_MINOR).$(COMMON_PATCH)

# opensea-transport version
TRANSPORT_MAJOR := 10
TRANSPORT_MINOR := 2
TRANSPORT_PATCH := 0
TRANSPORT_VERSION := $(TRANSPORT_MAJOR).$(TRANSPORT_MINOR).$(TRANSPORT_PATCH)

# opensea-operations version
OPERATIONS_MAJOR := 9
OPERATIONS_MINOR := 3
OPERATIONS_PATCH := 0
OPERATIONS_VERSION := $(OPERATIONS_MAJOR).$(OPERATIONS_MINOR).$(OPERATIONS_PATCH)

# opensea-jsonformat version
JSONFORMAT_MAJOR := 1
JSONFORMAT_MINOR := 0
JSONFORMAT_PATCH := 0
JSONFORMAT_VERSION := $(JSONFORMAT_MAJOR).$(JSONFORMAT_MINOR).$(JSONFORMAT_PATCH)

#===============================================================================
# Project Version
#===============================================================================

# Try to get version from git
GIT_VERSION := $(shell git describe --tags --always --dirty 2>/dev/null || echo "unknown")
PROJECT_VERSION ?= $(GIT_VERSION)

#===============================================================================
# Verbose Build Support
#===============================================================================

# V=1 for verbose output (default to quiet)
V ?= 0
ifeq ($(V),1)
    Q :=
    VERBOSE := 1
else
    Q := @
    VERBOSE := 0
endif

#===============================================================================
# Color Output (optional, for better UX)
#===============================================================================

# ANSI color codes (only if outputting to terminal)
ifeq ($(shell test -t 1 && echo yes),yes)
    COLOR_RESET := \033[0m
    COLOR_BOLD := \033[1m
    COLOR_RED := \033[31m
    COLOR_GREEN := \033[32m
    COLOR_YELLOW := \033[33m
    COLOR_BLUE := \033[34m
    COLOR_CYAN := \033[36m
    COLOR_HEADER := \033[1;36m
    COLOR_STATUS := \033[1;34m
else
    COLOR_RESET :=
    COLOR_BOLD :=
    COLOR_RED :=
    COLOR_GREEN :=
    COLOR_YELLOW :=
    COLOR_BLUE :=
    COLOR_CYAN :=
    COLOR_HEADER :=
    COLOR_STATUS :=
endif

# Print macros (for nice output)
define print_info
    @printf "$(COLOR_BLUE)INFO:$(COLOR_RESET) %s\n" "$(1)"
endef

define print_warning
    @printf "$(COLOR_YELLOW)WARNING:$(COLOR_RESET) %s\n" "$(1)"
endef

define print_error
    @printf "$(COLOR_RED)ERROR:$(COLOR_RESET) %s\n" "$(1)"
endef

define print_success
    @printf "$(COLOR_GREEN)SUCCESS:$(COLOR_RESET) %s\n" "$(1)"
endef

define print_status
    @printf "$(COLOR_STATUS)STATUS:$(COLOR_RESET) %s\n" "$(1)"
endef

#===============================================================================
# Export Variables for Recursive Make
#===============================================================================

export PLATFORM
export ARCH
export BUILD_TYPE
export OPTIMIZATION_FLAGS
export BUILD_DEFINES
export IS_DEBUG
export IS_STATIC
export BUILD_JSON
export BUILD_SHARED_LIBS
export USING_MUSL
export BUILD_DIR
export OBJ_DIR
export LIB_DIR
export BIN_DIR
export CC
export CXX
export AR
export RANLIB
export STRIP
export INSTALL
export PROJECT_ROOT
export MAKE_ROOT
export VERBOSE
